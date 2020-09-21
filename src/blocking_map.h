#pragma once
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include "log.h"
#include "util.h"

namespace gserver {

enum ERROR_NUM {
    OK = 0,
    NULL_PARAM = 1,  //空指针错误
    GET_TIMEOUT = 2, //调用take接口获取数据超时, 在tmo时间内未等到指定key对应的value
    DUPLICATE_KEY = 3, //调用put接口时发现对应key已经存在value

};

template<typename T>
class DataLatcher {
public:
    DataLatcher() :  _data(NULL) {}

    int take(T*& data, long tmo) {
        std::unique_lock<std::mutex> lock(_mutex);
        //wait putter put data
        std::chrono::milliseconds millon_sec(tmo);
        while (_data == NULL) {
            if (_cv.wait_for(lock, millon_sec) == std::cv_status::timeout) {
                return ERROR_NUM::GET_TIMEOUT;
            }
        }
        data = _data;
        return ERROR_NUM::OK;
    }

   int put(T* data) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (data == NULL) {
            return ERROR_NUM::NULL_PARAM;
        }
        _data = data;
        _cv.notify_all();
        return ERROR_NUM::OK;
    }

    inline bool empty() {
        return (_data == NULL);
    }

    inline T* data() {
        return _data;
    }

private:
    T* _data;
    //如果需要在bthread环境中使用，将以下两个变量替换成bthread::同步原语即可
    std::condition_variable _cv;
    std::mutex _mutex;
};

class BaseBlockingMap {
public:
    virtual void gc(long expire_time) = 0;
    virtual ~BaseBlockingMap() {};
};

class BlockingMapManager {
public:
    static BlockingMapManager* instance() {
        static BlockingMapManager manager;
        return &manager;
    }

    BlockingMapManager() : _reload_run(false) {}

    ~BlockingMapManager() {
        std::unique_lock<std::mutex> lock(_lock);
        if (_reload_run == true) {
            _reload_run = false;
            _reload_thread.join();
        }

        _objs.clear();
    }

    const std::vector<BaseBlockingMap*>& get_objs() {
        return _objs;
    }

    void regist(BaseBlockingMap* map) {
        std::unique_lock<std::mutex> lock(_lock);
        _objs.push_back(map);
    }
    
    void remove(BaseBlockingMap* need_remove) {
        std::unique_lock<std::mutex> lock(_lock);
        for (auto it = _objs.begin(); it != _objs.end(); ) {
            if (need_remove == *it) {
                it = _objs.erase(it);
            }  else {
                ++it; 
            }
        }
    }

    void reload(long expire_time, long interval = 3000) {
        std::unique_lock<std::mutex> lock(_lock);
        _reload_run = true;
        std::thread tmp(&BlockingMapManager::reload_entry, this, expire_time, interval);
        _reload_thread = std::move(tmp);
    }

    void reload_entry(long expire_time, long interval) {
        while (_reload_run) {
            for(auto obj: _objs) {
                obj->gc(expire_time);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }

private:
    std::vector<BaseBlockingMap*> _objs;
    std::mutex _lock;
    std::thread _reload_thread;
    bool _reload_run;
};



template<
    typename K,
    typename V,
    typename Hash = std::hash<K>,
    typename KeyEqual = std::equal_to<K>
>
class BlockingMap : public BaseBlockingMap {
public:
    //HashNode
    template<typename Key, typename Value>
    struct Node {
        Node() : next(NULL), timestamp(0), taken(false) {}
        Node* next;
        Key key;
        Value value;
        int64_t timestamp;
        bool taken;
    };
    typedef Node<K, DataLatcher<V> > HashNode;

    BlockingMap() : _buckets_num(0), _size(0), _buckets(NULL), _locks(NULL){}
    ~BlockingMap() {
        _buckets_num = 0;
        BlockingMapManager::instance()->remove(this);
        if (_buckets != NULL) {
            for (int i = 0; i < _buckets_num; ++i) {
                HashNode* node = _buckets[i];
                while (node) {
                    HashNode* next = node->next;
                    destroy<V>(node->value.data());
                    destroy<HashNode>(node);
                    node = next;
                }
            }
            delete [] _buckets;
            _buckets = NULL;
        }

        if (_locks != NULL) {
            delete [] _locks;
            _locks = NULL;
        }
    }

    //使用前必须先init，buckets_num建议设置为qps
    int init(size_t buckets_num) {
        _buckets_num = 1;
        while (_buckets_num < buckets_num) {
            _buckets_num <<= 1;
        }
        _buckets = new (std::nothrow) HashNode*[_buckets_num];
        //notice, call memset for initialize
        memset(_buckets, 0, _buckets_num * sizeof(HashNode*));
        _locks = new (std::nothrow) std::mutex[_buckets_num];
        //memset(_locks, 0, _buckets_num * sizeof(std::mutex));

        if (_buckets == NULL || _locks == NULL) {
            CFATAL_LOG("alloc buckets failed");
            return -1;
        }

        BlockingMapManager::instance()->regist(this);
        return 0;
    }

    int put(const K& key, const V& value) {
        //use uinque_ptr to avoid memory leak becasuse of return midway
        std::unique_ptr<V> value_ptr(create<V>(value));
        const size_t index = _index(key);
        std::unique_lock<std::mutex> lock(_locks[index]);
        HashNode* node = _seek_or_create(key, index);
        if (node == NULL) {
            CWARNING_LOG("put failed cause bad alloc");
            return ERROR_NUM::NULL_PARAM;
        }
        DataLatcher<V>& latcher = node->value;
        if (!latcher.empty()) {
            CWARNING_LOG("put failed cause duplicate key");
            return ERROR_NUM::DUPLICATE_KEY;
        }
        //already find an empty DataLacher, put V pointer and release the bucket lock
        int ret = latcher.put(value_ptr.get());
        if (ret != ERROR_NUM::OK) {
            CWARNING_LOG("put to DataLatcher failed, ret[%d]", ret);
            return ret;
        }
        //release the ownership
        value_ptr.release(); 
        //CDEBUG_LOG("put success!");
        return ERROR_NUM::OK;
    }

    int take(const K& key, V* val, long tmo) {
        const size_t index = _index(key);
        std::unique_lock<std::mutex> lock(_locks[index]);
        HashNode* node = _seek_or_create(key, index);

        if (node == NULL) {
            CWARNING_LOG("take failed cause bad alloc");
            return ERROR_NUM::NULL_PARAM;
        }
        if (node->taken) {
            CWARNING_LOG("value has been taken");
            return ERROR_NUM::DUPLICATE_KEY;
        }
        node->taken = true;
        DataLatcher<V>& latcher = node->value;
        //already find an DataLacher, release the bucket lock and take the value
        lock.unlock();
        V* tmp_v = NULL;
        int ret = latcher.take(tmp_v, tmo);
        if (ret != 0) {
            CWARNING_LOG("take data from DataLatcher failed, ret[%d]", ret);
            return ret;
        }
        //copy out and erase
        *val = *tmp_v;
        lock.lock();
        if (_erase(key, index) != 0) {
            CWARNING_LOG("erase failed, cant find key");
            return ERROR_NUM::NULL_PARAM;
        }
        //CDEBUG_LOG("take success!");
        return ERROR_NUM::OK;
    }
    
    size_t size() {
        return _size; 
    }
    size_t buckets_num() {
        return _buckets_num;
    }

    void gc(long expire_time) override { 
        int index = 0;
        HashNode deleted_head;
        HashNode* deleted_ptr = &deleted_head; //待删除节点链表头指针
        while (index < _buckets_num) {
            std::unique_lock<std::mutex> lock(_locks[index]);
            HashNode* node = _buckets[index];
            HashNode head;
            HashNode* head_ptr = &head; //当前桶内链表头指针
            while (node) {
                if (std_get_time_us() - node->timestamp > expire_time) {
                    deleted_ptr->next = node;
                    deleted_ptr = deleted_ptr->next;
                    _size--;
                } else {
                    head_ptr->next = node;
                    head_ptr = head_ptr->next;
                }
                node = node -> next;
            }

            head_ptr->next = NULL;
            _buckets[index] = head.next;
            ++index;
        }
        deleted_ptr->next = NULL;
        deleted_ptr = deleted_head.next;
        while (deleted_ptr) {
            HashNode* next = deleted_ptr->next;
            destroy<V>(deleted_ptr->value.data());
            destroy<HashNode>(deleted_ptr);
            deleted_ptr = next;
        }
    }

private:
    size_t _index(const K& key) {
        size_t index = _hash_fun(key) & (_buckets_num - 1);
        return index;
    }

    //call this method with bucket lock
    HashNode* _seek_or_create(const K& key, const size_t index) {
        HashNode* node = _buckets[index];
        HashNode* pre = NULL;
        while (node) {
            if (_euqual_fun(node->key, key)) {
                node->timestamp = std_get_time_us();
                return node;
            }
            pre = node;
            node = node->next;
        }

        node = create<HashNode>();
        if (node == NULL) {
            return NULL;
        }
        //create a HashNode with an empty DataLatcher
        node->key = key;
        node->next = NULL;
        node->timestamp = std_get_time_us();
        //CDEBUG_LOG("pre[%p] key[%d] ts[%lu]", pre, key, node->timestamp);
        if (pre) {
            pre->next = node;
        } else {
            _buckets[index] = node;
        }
        _size++;
        return node;
    }

    //call this method with bucket lock
    int _erase(const K& key, const size_t index) {
        HashNode* node = _buckets[index];
        HashNode* pre = NULL;
        while (node) {
            if (_euqual_fun(node->key, key)) {
                if (pre == NULL) {
                    _buckets[index] = node->next;
                } else {
                    pre->next = node->next;
                }
                V* v = node->value.data();
                destroy<V>(v);
                destroy<HashNode>(node);
                _size--;
                return 0;
            }
            pre = node;
            node = node->next;
        }
        return -1;
    }

    template<typename T, typename... Args>
    T* create(Args&& ...args) {
        T* p = (T*)malloc(sizeof(T));
        if (p == NULL) {
            CFATAL_LOG("alloc failed");
            return NULL;
        }
        new (p) T(std::forward<Args>(args)...);
        return p;
    }

    template<class T>
    void destroy(T* ptr) {
        if (ptr == NULL) {
            return;
        }
        delete ptr;
    }

    size_t _buckets_num;
    std::atomic_int _size;
    HashNode** _buckets;
    std::mutex* _locks;
    Hash _hash_fun;
    KeyEqual _euqual_fun;
};

} //namespace gserver
