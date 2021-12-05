// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/07/21.
//

#include <folly/MPMCQueue.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <random>
#include <thread>
#include <tuple>
using namespace std;
class AsyncTest : public ::testing::Test {};

TEST_F(AsyncTest, testPromise) {
    std::promise<int> prom;
    auto future = prom.get_future();
    std::thread thd(
            [](promise<int> prom) {
                std::this_thread::sleep_for(50ms);
                prom.set_value(10);
                std::cout << "prom.set_value" << std::endl;
            },
            std::move(prom));
    auto a = future.get();
    std::cout << "future.get()=" << a << std::endl;
    ASSERT_EQ(a, 10);
    thd.detach();
}

TEST_F(AsyncTest, testPromise2) {
    std::promise<int> prom;
    auto future = prom.get_future();
    std::thread thd(
            [](promise<int> prom) {
                std::this_thread::sleep_for(50ms);
                prom.set_value(10);
                std::cout << "prom.set_value" << std::endl;
            },
            std::move(prom));
    auto a = future.get();
    std::cout << "future.get()=" << a << std::endl;
    ASSERT_EQ(a, 10);
    thd.detach();
}

TEST_F(AsyncTest, testMPMCQueue) {
    auto q = std::make_shared<folly::MPMCQueue<int>>(1024);
    std::vector<std::thread> threads;
    std::atomic<bool> cancel(false);
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&q, &cancel]() {
            while (!cancel.load()) {
                int e;
                q->blockingRead(e);
                std::cout << "thread_id=" << std::this_thread::get_id() << ", e=" << e << std::endl;
            }
        });
    }
    for (int i = 0; i < 10; ++i) {
        q->write(i + 1);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cancel.store(true);
    for (int i = 0; i < 10; ++i) {
        q->write(-1);
    }
    for (int i = 0; i < 10; ++i) {
        threads[i].join();
    }
}

template <typename T>
class exclusive_ptr {
    using type = T;
    using pointer_type = std::unique_ptr<type>;

public:
    constexpr exclusive_ptr() {}
    constexpr exclusive_ptr(nullptr_t) {}
    explicit exclusive_ptr(pointer_type&& v) : _value(v.release()) {}
    exclusive_ptr(const exclusive_ptr& other) : _value(other.release()) {}
    exclusive_ptr(exclusive_ptr&& other) : _value(other.release()) {}

    template <typename U>
    exclusive_ptr(exclusive_ptr<U>&& other) : _value(other.release()) {}
    template <typename U>
    exclusive_ptr(const exclusive_ptr<U>& other) : _value(other.release()) {}

    exclusive_ptr& operator=(const exclusive_ptr& other) {
        this->reset(other.release());
        return this;
    }
    exclusive_ptr& operator=(exclusive_ptr&& other) {
        this->reset(other.release());
        return *this;
    }
    exclusive_ptr& operator=(nullptr_t) {
        this->_value = nullptr;
        return *this;
    }
    template <typename U>
    exclusive_ptr& operator=(exclusive_ptr<U>&& other) {
        this->reset(other.release());
        return *this;
    }
    template <typename U>
    exclusive_ptr& operator=(const exclusive_ptr<U>& other) {
        this->reset(other.release());
        return *this;
    }

    explicit operator bool() { return _value; }
    type* get() const { return _value.get(); }
    void reset() const { _value.reset(); }
    void reset(type* ptr) const { _value.reset(ptr); }
    type* release() const { return _value.release(); }
    type* operator->() const { return _value.get(); }
    type& operator*() const { return *_value; }
    pointer_type get_unique() const { return std::move(_value); }

private:
    mutable pointer_type _value;
};

template <typename T, typename... Args>
static inline exclusive_ptr<T> make_exclusive(Args&&... args) {
    return exclusive_ptr<T>(std::make_unique<T>(std::forward<Args>(args)...));
}

TEST_F(AsyncTest, testPromiseMove) {
    std::list<std::function<void()>> queue;
    std::mutex mutex;
    std::condition_variable cv_empty;
    std::atomic<bool> shutdown(false);
    std::thread thd([&queue, &mutex, &cv_empty, &shutdown]() {
        while (!shutdown.load()) {
            std::cout << "enter" << std::endl;
            std::unique_lock<std::mutex> lock(mutex);
            if (queue.empty()) {
                cv_empty.wait(lock, [&shutdown, &queue]() { return shutdown.load() || !queue.empty(); });
            }
            if (shutdown.load()) {
                break;
            }
            auto task = queue.front();
            queue.pop_front();
            task();
        }
    });
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    auto reg = make_exclusive<int>(12);
    for (int i = 0; i < 10; ++i) {
        std::cout << "round i=" << i << std::endl;
        auto prom = make_exclusive<std::promise<exclusive_ptr<int>>>();
        auto future = prom->get_future();
        auto func = [prom, reg](const std::string& op) mutable {
            auto a = *reg;
            if (op == "inc1") {
                a += 1;
            } else if (op == "dec1") {
                a += 1;
            } else if (op == "mul2") {
                a *= 2;
            } else if (op == "div2") {
                a /= 2;
            } else {
            }
            prom->set_value(reg);
        };
        {
            std::unique_lock<std::mutex> lock(mutex);
            std::function<void()> task = [func]() mutable { func("inc1"); };
            queue.push_back(task);
            cv_empty.notify_one();
        }
        reg = std::move(future.get());
    }
    shutdown.store(true);
    cv_empty.notify_one();
    thd.join();
    std::cout << "result=" << *reg << std::endl;
}

TEST_F(AsyncTest, testWaitFuture) {
    auto prom = make_exclusive<std::promise<int>>();
    auto fut = prom->get_future();

    // mover<PromisePtr> prom_movable(std::move(prom));
    int a = 10;
    auto task = [prom, a]() mutable { prom->set_value(a + 1); };
    std::thread thd(std::move(task));

    thd.join();
    auto fut_status = fut.wait_for(std::chrono::seconds(0));
    ASSERT_EQ(fut_status, std::future_status::ready);
    auto b = fut.get();
    std::cout << "b=" << b << std::endl;
}

TEST_F(AsyncTest, testWaitFuture2) {
    auto prom = make_exclusive<std::promise<int>>();
    auto fut = prom->get_future();

    // mover<PromisePtr> prom_movable(std::move(prom));
    int a = 10;
    std::vector<std::function<void()>> functions;
    std::function<void()> new_task = [prom, a]() mutable {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        prom->set_value(10);
    };
    functions.emplace_back(new_task);
    auto task = functions.front();
    std::thread thd(task);
    {
        auto fut2 = std::move(fut);
        auto fut2_status = fut2.wait_for(std::chrono::seconds(0));
        ASSERT_NE(fut2_status, std::future_status::ready);
    }
    thd.join();
}

TEST_F(AsyncTest, testOptional) {
    std::optional<int> a;
    a = 10;
    ASSERT_TRUE(a.has_value());
    auto b = std::move(a);
    ASSERT_EQ(b, 10);
    a.reset();
    ASSERT_FALSE(a.has_value());
}

TEST_F(AsyncTest, testPromise3) {
    auto computation = make_exclusive<int>(2);
    for (int i = 0; i < 1000; i++) {
        auto computation_prom = make_exclusive<std::promise<exclusive_ptr<int>>>();
        auto fut = computation_prom->get_future();
        std::function<void()> task = [computation, computation_prom]() mutable {
            *computation += 2;
            computation_prom->set_value(computation);
        };
        thread thd(task);
        thd.join();
        computation = fut.get();
    }
    ASSERT_EQ(*computation, 2002);
}
class BaseFoo {
public:
    virtual ~BaseFoo() = default;
};
class DerivedFoo final : public BaseFoo {};

TEST_F(AsyncTest, testUniquePtr) {
    unique_ptr<BaseFoo> a;
    a = std::make_unique<DerivedFoo>();
}

std::vector<std::unique_ptr<BaseFoo>> get_vector_unique_ptr() {
    std::vector<std::unique_ptr<BaseFoo>> v;
    v.emplace_back(std::make_unique<BaseFoo>());
    v.emplace_back(std::make_unique<BaseFoo>());
    return v;
}

TEST_F(AsyncTest, testVectorUniquePtr) {
    std::unordered_map<int, std::vector<std::unique_ptr<BaseFoo>>> a;
    std::vector<std::unique_ptr<BaseFoo>> v = get_vector_unique_ptr();
    a.emplace(10, std::move(v));
    auto& v2 = a[10];
    for (auto& e : v2) {
        std::cout << e.get() << std::endl;
    }
}

TEST_F(AsyncTest, testExclusivePtr) {
    exclusive_ptr<BaseFoo> foo;
    foo = make_exclusive<DerivedFoo>();
    exclusive_ptr<BaseFoo> foo2 = make_exclusive<DerivedFoo>();
}

TEST_F(AsyncTest, testChronoNow) {
    using milliseconds = std::chrono::milliseconds;
    int64_t deadline = std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now().time_since_epoch() +
                                                                std::chrono::seconds(5))
                               .count();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    int64_t now = std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    std::cout << "deadline=" << deadline << ", now=" << now << std::endl;
}

TEST_F(AsyncTest, testCallOnce) {
    std::once_flag once;
    std::atomic<int> counter{0};
    std::vector<std::thread> threads;
    for(int i=0; i< 10; ++i){
        threads.emplace_back([i,&once,&counter](){
           call_once(once, [&counter](){counter++;});
           ASSERT_EQ(counter.load(), 1);
        });
    }
    for (auto& thd: threads){
        thd.join();
    }
    std::cout<<"counter="<<counter.load()<<std::endl;
}
struct AAAAAA {
    AAAAAA()=default;
    AAAAAA(AAAAAA &&a){
        std::cout<<"move ctor"<<std::endl;
    }
    ~AAAAAA(){
        std::cout<<"dtor"<<std::endl;
    }
    AAAAAA& operator=(AAAAAA&& a) {
        std::cout<<"move assign"<<std::endl;
    }
};
TEST_F(AsyncTest, testAssignRightValueToRightValueRef){
    std::vector<std::shared_ptr<AAAAAA>> x;
    x.emplace_back(std::make_shared<AAAAAA>());
    {
        const auto& b = std::move(x.back());
        std::cout<<"L0"<<std::endl;
        x.pop_back();
        std::cout<<"L1"<<endl;
    }
    std::atomic<int>a;
    --a;

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}