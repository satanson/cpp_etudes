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
        std::cout << "obj_count=" << (n - 1) << std::endl;
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

struct Chunk {
    explicit Chunk(const std::string& s) : data(s) {}
    std::string data;
};

class LocalShuffleFabric {
public:
    LocalShuffleFabric(int num_sinkers, int num_sources);

    bool has_output(const int o);

    bool need_input(const int i);

    void add_chunk(const int i, const int o, Chunk* chunk);

    Chunk* get_chunk(int i, int o);

    void set_input_finished(const int i);

    void set_output_finished(const int o);

    bool is_source_finished(const int o);

    bool is_sink_finished(const int i);

    bool is_finished(const int i, const int o);

    size_t num_sinkers() const { return _num_sinkers; }
    size_t num_sources() const { return _num_sources; }

private:
    const int _num_sinkers;
    const int _num_sources;
    std::vector<std::atomic<uintptr_t>> _fabric;
};

struct ChunkState {
    static constexpr auto OUTPUT_FINISHED_BIT = 0x1ul;
    static constexpr auto INPUT_FINISHED_BIT = 0x2ul;
    static constexpr auto POINTER_MASK_BITS = 0x7ul;

    explicit ChunkState(uintptr_t data) : data(data) {}
    ChunkState(uintptr_t data, bool output_finished, bool input_finished)
            : data(data | (output_finished ? OUTPUT_FINISHED_BIT : 0ul) | (input_finished ? INPUT_FINISHED_BIT : 0ul)) {
    }
    ChunkState(Chunk* chunk, bool output_finished, bool input_finished)
            : ChunkState(uintptr_t(chunk), output_finished, input_finished) {}

    bool is_output_finished() { return (data & OUTPUT_FINISHED_BIT) == OUTPUT_FINISHED_BIT; }
    bool is_input_finished() { return (data & INPUT_FINISHED_BIT) == INPUT_FINISHED_BIT; }
    Chunk* chunk() { return (Chunk*)(data & ~POINTER_MASK_BITS); }
    bool need_input() { return !is_input_finished() && !is_output_finished() && chunk() == nullptr; }
    bool is_sink_finished() { return is_input_finished() || is_output_finished(); }

    uintptr_t data;
};

LocalShuffleFabric::LocalShuffleFabric(int num_sinkers, int num_sources)
        : _num_sinkers(num_sinkers), _num_sources(num_sources), _fabric(num_sinkers * num_sources) {
    for (int i = 0; i < _fabric.size(); ++i) {
        _fabric[i] = 0;
    }
}

bool LocalShuffleFabric::has_output(const int o) {
    for (int i = 0; i < _num_sinkers; ++i) {
        auto idx = i * _num_sources + o;
        auto state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
        if (!state.is_output_finished() && state.chunk() != nullptr) {
            return true;
        }
    }
    return false;
}

bool LocalShuffleFabric::need_input(const int i) {
    for (int o = 0; o < _num_sources; ++o) {
        auto idx = i * _num_sources + o;
        auto state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
        if (!state.need_input()) {
            return false;
        }
    }
    return true;
}

void LocalShuffleFabric::add_chunk(const int i, const int o, Chunk* chunk) {
    auto idx = i * _num_sources + o;
    auto old_state = ChunkState(nullptr, false, false);
    ChunkState new_state(chunk, false, false);
    if (!_fabric[idx].compare_exchange_strong(old_state.data, new_state.data)) {
        if (old_state.is_output_finished() || old_state.is_input_finished()) {
            delete chunk;
        }
    }
}

Chunk* LocalShuffleFabric::get_chunk(int i, int o) {
    auto idx = i * _num_sources + o;
    auto state = ChunkState(_fabric[idx].load(std::memory_order_acquire));

    if (state.is_output_finished()) {
        auto* chunk = state.chunk();
        if (chunk != nullptr) {
            delete chunk;
            ChunkState new_state(uintptr_t(0), true, true);
            _fabric[idx].store(new_state.data);
        }
        return nullptr;
    }

    if (state.is_input_finished()) {
        ChunkState new_state(nullptr, false, true);
        _fabric[idx].store(new_state.data);
        return state.chunk();
    }

    if (state.chunk() == nullptr) {
        return nullptr;
    }

    ChunkState new_state(nullptr, state.is_output_finished(), state.is_input_finished());
    while (!_fabric[idx].compare_exchange_strong(state.data, new_state.data)) {
        new_state = ChunkState(nullptr, state.is_output_finished(), state.is_input_finished());
    }
    return state.chunk();
}

void LocalShuffleFabric::set_input_finished(const int i) {
    for (int o = 0; o < _num_sources; ++o) {
        auto idx = i * _num_sources + o;
        auto old_state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
        ChunkState new_state(old_state.chunk(), old_state.is_output_finished(), true);
        while (!_fabric[idx].compare_exchange_strong(old_state.data, new_state.data)) {
            new_state = ChunkState(old_state.chunk(), old_state.is_output_finished(), true);
        }
    }
}

void LocalShuffleFabric::set_output_finished(const int o) {
    for (int i = 0; i < _num_sinkers; ++i) {
        auto idx = i * _num_sources + o;
        auto old_state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
        ChunkState new_state(nullptr, true, old_state.is_input_finished());
        while (!_fabric[idx].compare_exchange_strong(old_state.data, new_state.data)) {
            new_state = ChunkState(nullptr, true, old_state.is_input_finished());
        }

        auto* chunk = old_state.chunk();
        if (chunk != nullptr) {
            delete chunk;
        }
    }
}

bool LocalShuffleFabric::is_source_finished(const int o) {
    for (int i = 0; i < _num_sinkers; ++i) {
        auto idx = i * _num_sources + o;
        auto state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
        if (!state.is_output_finished() && (!state.is_input_finished() || state.chunk() != nullptr)) {
            return false;
        }
    }
    return true;
}

bool LocalShuffleFabric::is_sink_finished(const int i) {
    for (int o = 0; o < _num_sources; ++o) {
        auto idx = i * _num_sources + o;
        auto state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
        if (!state.is_sink_finished()) {
            return false;
        }
    }
    return true;
}

bool LocalShuffleFabric::is_finished(const int i, const int o) {
    const auto idx = i * _num_sources + o;
    auto state = ChunkState(_fabric[idx].load(std::memory_order_acquire));
    return state.is_output_finished() || state.is_output_finished();
}

TEST_F(TestFutex, testFabric) {
    auto fabric = std::make_shared<LocalShuffleFabric>(3, 4);
    for (int o = 0; o < fabric->num_sources(); ++o) {
        ASSERT_FALSE(fabric->is_source_finished(o));
        ASSERT_FALSE(fabric->has_output(o));
    }
    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        ASSERT_FALSE(fabric->is_sink_finished(i));
        ASSERT_TRUE(fabric->need_input(i));
    }
    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        for (int o = 0; o < fabric->num_sources(); ++o) {
            ASSERT_TRUE(fabric->get_chunk(i, o) == nullptr);
        }
    }
    auto chunk1 = std::make_unique<Chunk>("(1 to 1)");
    fabric->add_chunk(1, 1, chunk1.get());
    ASSERT_TRUE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_TRUE(fabric->need_input(2));
    ASSERT_FALSE(fabric->has_output(0));
    ASSERT_TRUE(fabric->has_output(1));
    ASSERT_FALSE(fabric->has_output(2));
    ASSERT_FALSE(fabric->has_output(3));
    for (int o = 0; o < fabric->num_sources(); ++o) {
        ASSERT_FALSE(fabric->is_source_finished(o));
    }
    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        ASSERT_FALSE(fabric->is_sink_finished(i));
    }
    auto* chunk = fabric->get_chunk(1, 1);
    ASSERT_EQ(chunk->data, "(1 to 1)");
    for (int o = 0; o < fabric->num_sources(); ++o) {
        ASSERT_FALSE(fabric->is_source_finished(o));
        ASSERT_FALSE(fabric->has_output(o));
    }

    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        ASSERT_FALSE(fabric->is_sink_finished(i));
        ASSERT_TRUE(fabric->need_input(i));
    }

    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        for (int o = 0; o < fabric->num_sources(); ++o) {
            ASSERT_TRUE(fabric->get_chunk(i, o) == nullptr);
        }
    }
    auto chunk2 = std::make_unique<Chunk>("2 to 3");
    auto chunk3 = std::make_unique<Chunk>("0 to 2");
    fabric->add_chunk(1, 1, chunk1.get());
    ASSERT_TRUE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_TRUE(fabric->need_input(2));
    fabric->add_chunk(2, 3, chunk2.get());
    ASSERT_TRUE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_FALSE(fabric->need_input(2));
    fabric->add_chunk(0, 2, chunk3.get());
    ASSERT_FALSE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_FALSE(fabric->need_input(2));

    ASSERT_FALSE(fabric->has_output(0));
    ASSERT_TRUE(fabric->has_output(1));
    ASSERT_TRUE(fabric->has_output(2));
    ASSERT_TRUE(fabric->has_output(3));
    fabric->set_input_finished(0);
    fabric->set_input_finished(1);
    fabric->set_input_finished(2);
    ASSERT_FALSE(fabric->has_output(0));
    ASSERT_TRUE(fabric->has_output(1));
    ASSERT_TRUE(fabric->has_output(2));
    ASSERT_TRUE(fabric->has_output(3));
    ASSERT_TRUE(fabric->is_source_finished(0));
    ASSERT_FALSE(fabric->is_source_finished(1));
    ASSERT_FALSE(fabric->is_source_finished(2));
    ASSERT_FALSE(fabric->is_source_finished(3));
    ASSERT_FALSE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_FALSE(fabric->need_input(2));
    ASSERT_TRUE(fabric->is_sink_finished(0));
    ASSERT_TRUE(fabric->is_sink_finished(1));
    ASSERT_TRUE(fabric->is_sink_finished(2));

    ASSERT_TRUE(fabric->has_output(2));
    ASSERT_EQ(fabric->get_chunk(0, 2)->data, "0 to 2");
    ASSERT_FALSE(fabric->has_output(2));
    ASSERT_TRUE(fabric->is_source_finished(2));

    ASSERT_TRUE(fabric->has_output(3));
    ASSERT_EQ(fabric->get_chunk(2, 3)->data, "2 to 3");
    ASSERT_FALSE(fabric->has_output(3));
    ASSERT_TRUE(fabric->is_source_finished(3));
}

TEST_F(TestFutex, testFabric2) {
    auto fabric = std::make_shared<LocalShuffleFabric>(3, 4);
    for (int o = 0; o < fabric->num_sources(); ++o) {
        ASSERT_FALSE(fabric->is_source_finished(o));
        ASSERT_FALSE(fabric->has_output(o));
    }
    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        ASSERT_FALSE(fabric->is_sink_finished(i));
        ASSERT_TRUE(fabric->need_input(i));
    }
    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        for (int o = 0; o < fabric->num_sources(); ++o) {
            ASSERT_TRUE(fabric->get_chunk(i, o) == nullptr);
        }
    }
    auto chunk1 = std::make_unique<Chunk>("(1 to 1)");
    fabric->add_chunk(1, 1, chunk1.get());
    ASSERT_TRUE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_TRUE(fabric->need_input(2));
    ASSERT_FALSE(fabric->has_output(0));
    ASSERT_TRUE(fabric->has_output(1));
    ASSERT_FALSE(fabric->has_output(2));
    ASSERT_FALSE(fabric->has_output(3));
    for (int o = 0; o < fabric->num_sources(); ++o) {
        ASSERT_FALSE(fabric->is_source_finished(o));
    }
    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        ASSERT_FALSE(fabric->is_sink_finished(i));
    }
    auto* chunk = fabric->get_chunk(1, 1);
    ASSERT_EQ(chunk->data, "(1 to 1)");
    for (int o = 0; o < fabric->num_sources(); ++o) {
        ASSERT_FALSE(fabric->is_source_finished(o));
        ASSERT_FALSE(fabric->has_output(o));
    }

    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        ASSERT_FALSE(fabric->is_sink_finished(i));
        ASSERT_TRUE(fabric->need_input(i));
    }

    for (int i = 0; i < fabric->num_sinkers(); ++i) {
        for (int o = 0; o < fabric->num_sources(); ++o) {
            ASSERT_TRUE(fabric->get_chunk(i, o) == nullptr);
        }
    }
    auto chunk2 = std::make_unique<Chunk>("2 to 3");
    auto chunk3 = std::make_unique<Chunk>("0 to 2");
    fabric->add_chunk(1, 1, chunk1.release());
    ASSERT_TRUE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_TRUE(fabric->need_input(2));
    fabric->add_chunk(2, 3, chunk2.release());
    ASSERT_TRUE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_FALSE(fabric->need_input(2));
    fabric->add_chunk(0, 2, chunk3.release());
    ASSERT_FALSE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_FALSE(fabric->need_input(2));

    ASSERT_FALSE(fabric->has_output(0));
    ASSERT_TRUE(fabric->has_output(1));
    ASSERT_TRUE(fabric->has_output(2));
    ASSERT_TRUE(fabric->has_output(3));
    fabric->set_output_finished(0);
    fabric->set_output_finished(1);
    fabric->set_output_finished(2);
    fabric->set_output_finished(3);
    ASSERT_FALSE(fabric->has_output(0));
    ASSERT_FALSE(fabric->has_output(1));
    ASSERT_FALSE(fabric->has_output(2));
    ASSERT_FALSE(fabric->has_output(3));
    ASSERT_TRUE(fabric->is_source_finished(0));
    ASSERT_TRUE(fabric->is_source_finished(1));
    ASSERT_TRUE(fabric->is_source_finished(2));
    ASSERT_TRUE(fabric->is_source_finished(3));
    ASSERT_FALSE(fabric->need_input(0));
    ASSERT_FALSE(fabric->need_input(1));
    ASSERT_FALSE(fabric->need_input(2));
    ASSERT_TRUE(fabric->is_sink_finished(0));
    ASSERT_TRUE(fabric->is_sink_finished(1));
    ASSERT_TRUE(fabric->is_sink_finished(2));

    ASSERT_FALSE(fabric->has_output(2));
    ASSERT_EQ(fabric->get_chunk(0, 2), nullptr);
    ASSERT_FALSE(fabric->has_output(2));
    ASSERT_TRUE(fabric->is_source_finished(2));

    ASSERT_FALSE(fabric->has_output(3));
    ASSERT_EQ(fabric->get_chunk(2, 3), nullptr);
    ASSERT_FALSE(fabric->has_output(3));
    ASSERT_TRUE(fabric->is_source_finished(3));
}

} // namespace test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}