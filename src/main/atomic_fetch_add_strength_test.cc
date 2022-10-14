// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/24.
//

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace std;

volatile atomic<int64_t> count_var = 0;
volatile atomic<int> start_flags = 0;
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Missing args\nrun " << argv[0] << " <thread_num>" << endl;
        exit(0);
    }
    int thread_num = atoi(argv[1]);
    assert(thread_num >= 1);
    vector<thread> threads;
    threads.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back([] {
            while (start_flags.load(std::memory_order_acquire) == 0) {
                std::this_thread::sleep_for(10ms);
            }
            while (true) {
                count_var.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    start_flags.store(1, std::memory_order_release);
    auto start_time = std::chrono::steady_clock::now();
    auto deadline_time = start_time + 10s;
    long i = 0;
    while (std::chrono::steady_clock::now() < deadline_time) {
        std::this_thread::sleep_for(1s);
        std::cout << "runing " << i << " ..." << std::endl;
        ++i;
    }
    auto end_time = std::chrono::steady_clock::now();
    auto m = count_var.load(std::memory_order_relaxed);
    cout << "m=" << m << std::endl;
    auto micro_secs = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    cout << "fetch_add: " << m / micro_secs * 1'000'000 << endl;
    for (auto& thd : threads) {
        thd.detach();
    }
    for (auto& thd : threads) {
        thd.join();
    }
    return 0;
}
