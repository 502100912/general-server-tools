#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <stdio.h>
#include <queue>
#include <mutex>
#include <assert.h>
#include <condition_variable>
namespace gserver {

//thred-safe blocking queue, support fixed capacity
template <typename T, int MAXSIZE = 0>
class BlockingQueue {
public:
    BlockingQueue():_capacity(MAXSIZE) {
        if (MAXSIZE > 0) {
            _is_fixed_size = true;
        }
    }

    void put(const T& t) {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_is_fixed_size) {
            _free_slot.wait(lock, [this] {return _queue.size() < _capacity;});
            assert(_queue.size() < _capacity);
        }
        _queue.push(t);
        _resource.notify_one();
    }

    T& wait_and_pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        //1.call with lock locked
        //2.check condition and return from wait if it is statisfied
        //3.if not statisfied, unlock and put this thread in blocked atmoicially
        _resource.wait(lock, [this] {return !_queue.empty();});
        //when it is notified by other thread
        //1.reacquires the lock on the mutex
        //2.check condition again
        //3.the same behavior like the above

        //here, we keep the mutex again
        assert(!_queue.empty());
        T& t = _queue.front();
        _queue.pop();
        if (_is_fixed_size) {
            _free_slot.notify_one();
        }
        return t;
    }

    bool try_pop(T& value) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return false;
        }
        value = _queue.front();
        _queue.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _resource;

    //default to be unlimited capacity
    bool _is_fixed_size = false; 
    size_t _capacity = 0;
    std::condition_variable _free_slot; 

};

} //end g_server

#endif

