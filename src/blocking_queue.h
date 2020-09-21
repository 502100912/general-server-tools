#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <stdio.h>
#include <queue>
#include <mutex>
#include <assert.h>
#include <condition_variable>
namespace gserver {

//thred-safe blocking queue
//support limited capacity and default to be unlimited
template <typename T, size_t MAXSIZE = 0>
class BlockingQueue {
public:
    BlockingQueue() : _capacity(MAXSIZE) {
        if (MAXSIZE > 0) {
            _is_fixed_size = true;
        }
    }
    
    //初始化BlockingQueue, 并初始化就持有init_size个T对象
    template<typename... Args>
    BlockingQueue(size_t init_size, Args&&... args) : _capacity(MAXSIZE) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (MAXSIZE > 0) {
            _is_fixed_size = true;
            if (init_size > _capacity) {
                init_size = _capacity; 
            }
        }
        for(int i = 0; i < init_size; i++) {
            _queue.emplace(std::forward<Args>(args)...);
        }
    }

    //Produce an resource, random notify a thread which waiting on resource
    //Will block at wait free_slot if queue is limited
    bool put(const T& t) {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_is_fixed_size) {
            _free_slot.wait(lock, [this] {return _queue.size() < _capacity;});
            assert(_queue.size() < _capacity);
        }
        _queue.push(t);
        _resource.notify_one();
        return true;
    }

    //Consume an object, random notify a thread which waiting on free_slot if queue is limited
    //Will block if the queue is empty
    bool wait_and_pop(T& t) {
        std::unique_lock<std::mutex> lock(_mutex);
        //1.call with lock locked
        //2.check condition and return from wait if it is statisfied
        //3.if not statisfied, unlock and put this thread in blocked atmoicially
        _resource.wait(lock, [this] {
                    return !_queue.empty() || _released;
                    });
        //when it is notified by other thread
        //1.reacquires the lock on the mutex
        //2.check condition again
        //3.the same behavior like the above

        //here, we keep the mutex again
        if (_released) {
            //other thread release this queue 
            return false;     
        }
        assert(!_queue.empty());
        t = _queue.front();
        _queue.pop();
        if (_is_fixed_size) {
            _free_slot.notify_one();
        }
        return true;
    }

    //Try consume an object, check the queue empty first
    //Return true if pop successfully
    bool try_pop(T& value) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return false;
        }
        value = _queue.front();
        _queue.pop();
        if (_is_fixed_size) {
            _free_slot.notify_one();
        }
        return true;
    }

    bool release() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_released) {
            _released = true;
        }
        _resource.notify_all();
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    size_t size() {
        return _queue.size(); 
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _resource;
    bool _released = false;

    //default to be unlimited capacity
    bool _is_fixed_size = false; 
    size_t _capacity = 0;
    std::condition_variable _free_slot; 


};

} //end g_server

#endif

