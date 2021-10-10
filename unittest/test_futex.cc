// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git
//
// Created by grakra on 2021/08/13.
//

#include <gtest/gtest.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <thread>

#include "measure_time.hh"
namespace test {
class TestFutex : public ::testing::Test {};
TEST_F(TestFutex, test_futex_0) {
    volatile uint32_t uaddr = 0;
    std::thread wake_thread([&uaddr]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        syscall(SYS_futex, &uaddr, FUTEX_WAKE_PRIVATE);
    });

    int64_t cost;
    {
        TimeMeasure m(cost);
        syscall(SYS_futex, &uaddr, FUTEX_WAIT_PRIVATE, 0, NULL);
    }
    std::cout << "cost=" << cost << std::endl;
    wake_thread.join();
}

TEST_F(TestFutex, test_futex_1) {
    volatile uint32_t uaddr = 0;
    std::vector<std::thread> threads;
    std::atomic<int> count(0);
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&count, &uaddr]() {
            int64_t cost;
            {
                TimeMeasure m(cost);
                syscall(SYS_futex, &uaddr, FUTEX_WAIT_PRIVATE, 0, NULL);
                count.fetch_add(1);
            }
            std::cout << "thread=" << std::this_thread::get_id() << ", cost=" << cost << std::endl;
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // FUTEX_WALK only wake one waiter
    // syscall(SYS_futex, &uaddr, FUTEX_WAKE_PRIVATE);
    // wake up all
    syscall(SYS_futex, &uaddr, FUTEX_WAKE_PRIVATE, std::numeric_limits<int>::max());

    int64_t cost;
    { TimeMeasure m(cost); }
    for (auto& thd : threads) {
        thd.join();
    }
    ASSERT_EQ(count.load(), 10);
}
TEST_F(TestFutex, test_futex_2) {
    volatile uint32_t uaddr0 = 0;
    volatile uint32_t uaddr1 = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&uaddr0]() {
            int64_t cost;
            {
                TimeMeasure m(cost);
                syscall(SYS_futex, &uaddr0, FUTEX_WAIT_PRIVATE, 0, NULL);
            }
            std::cout << "thread=" << std::this_thread::get_id() << ", cost=" << cost << std::endl;
        });
    }
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // if uaddr0 eq 0, then wake one waiter on uaddr0 and requeue rest of all to
        // uaddr1
        syscall(SYS_futex, &uaddr0, FUTEX_CMP_REQUEUE_PRIVATE, 1, std::numeric_limits<int>::max(), &uaddr1, 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        syscall(SYS_futex, &uaddr1, FUTEX_CMP_REQUEUE_PRIVATE, 1, std::numeric_limits<int>::max(), &uaddr0, 0);
    }
    for (auto& thd : threads) {
        thd.join();
    }
}
TEST_F(TestFutex, test_time_since_epoch) {
    auto a = std::chrono::steady_clock::now().time_since_epoch();
    auto b = std::chrono::system_clock::now().time_since_epoch();
    std::cout << std::chrono::duration_cast<std::chrono::hours>(a).count() << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::hours>(b).count() << std::endl;
}

class CRObj;
using CRObjPtr = std::shared_ptr<CRObj>;
using CRObjArray = std::vector<CRObjPtr>;
class CRObjManager;
using CRObjManagerPtr = std::shared_ptr<CRObjManager>;
class CRObj {
public:
    void set_id(int id) { this->_id = id; }
    int id() { return _id; }
    void set_mgr(CRObjManager* mgr) { this->_mgr = mgr; }
    CRObjManager* mgr() { return _mgr; }
    void finalize();

private:
    int _id;
    CRObjManager* _mgr;
};

class CRObjManager {
public:
    void add_obj_array(CRObjArray&& obj_array) {
        _obj_array = std::move(obj_array);
        _obj_count.store(_obj_array.size());
    }

    bool count_down() {
        auto n = _obj_count.fetch_sub(1);
        std::cout<<"obj_count=" <<(n-1)<<std::endl;
        return n == 1;
    }

    CRObjArray& obj_array() { return _obj_array; }

private:
    CRObjArray _obj_array;
    std::atomic<size_t> _obj_count;
};

static std::shared_ptr<CRObjManager> global_manager;
void CRObj::finalize() {
    std::cout << "finalize: id=" << this->id() << std::endl;
    if (_mgr->count_down()) {
        std::cout << "reset global_manager" << std::endl;
        global_manager.reset();
    }
}

TEST_F(TestFutex, testCrossReference) {
    global_manager = std::make_shared<CRObjManager>();
    CRObjArray obj_array;
    for (int i = 0; i < 10; ++i) {
        obj_array.push_back(std::make_shared<CRObj>());
        obj_array.back()->set_id(i);
        obj_array.back()->set_mgr(global_manager.get());
    }
    global_manager->add_obj_array(std::move(obj_array));
    for (auto obj : global_manager->obj_array()) {
        obj->finalize();
    }
}

} // namespace test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}