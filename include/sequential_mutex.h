// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/9.
//

#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
namespace concurrency {
template <typename T>
class thread_local_object {
public:
    thread_local_object() { pthread_key_create(&_key, free); }
    ~thread_local_object() { pthread_key_delete(_key); }
    thread_local_object<T>& operator=(T&& t) { pthread_setspecific(_key, new T(std::forward<T>(t))); return *this;}
    operator T() {
        auto* p = pthread_getspecific(_key);
        if (p == nullptr) {
            *this = T();
            return T();
        } else {
            return *(T*)p;
        }
    }
private:
    pthread_key_t _key;
};
class seq_mutex {
public:
    seq_mutex(const std::vector<size_t> timing) : _timing(timing) {}
    void lock() {
        std::unique_lock require_lock(_mutex);
        assert(_timing_idx <_timing.size());
        while (_timing[_timing_idx] != get_thread_id()) {
            _cond_var.wait(require_lock);
        }
    }
    bool try_lock() {
        std::unique_lock require_lock(_mutex);
        assert(_timing_idx <_timing.size());
        return _timing[_timing_idx] == get_thread_id();
    }
    void unlock() {
        std::unique_lock require_lock(_mutex);
        assert(_timing_idx <_timing.size());
        if (_timing[_timing_idx] == get_thread_id()) {
            ++_timing_idx;
            _cond_var.notify_all();
        }
    }

    void lock_shared() { lock(); }
    bool try_lock_shared() { return try_lock(); }
    void unlock_shared() { unlock(); }
    void set_thread_id(int i) { _thread_id = i; }
    size_t get_thread_id() { return _thread_id; }

private:
    std::mutex _mutex;
    std::condition_variable _cond_var;
    std::vector<size_t> _timing;
    size_t _timing_idx = 0;
    thread_local_object<size_t> _thread_id;
};
} // namespace concurrency