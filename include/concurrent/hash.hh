// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/7/2.
//

#ifndef CPP_ETUDES_HASH_HH
#define CPP_ETUDES_HASH_HH
#include <cassert>
#include <concurrent/list.hh>
#include <util/bits_op.hh>
#include <memory>
#include <cstring>
#include <vector>
namespace com {
namespace grakra {
namespace concurrent {

#define LOG2(n) __builtin_ctzll(n)
#define CLEAR_MSB(n) ((n) & (((typeof(n))1) << (sizeof(n) - 1 - __builtin_clz(n))))
#define PARENT_SLOT(n) CLEAR_MSB(n)

constexpr size_t SLOT_INDEX_SHIFT = 12 - LOG2(sizeof(void *));
constexpr size_t SLOT_INDEX_NR = 1 << SLOT_INDEX_SHIFT;
constexpr size_t SLOT_INDEX_MASK = SLOT_INDEX_NR - 1;
constexpr size_t HASH_SIZE_LIMIT = UINT32_MAX >> 1;
constexpr size_t HASH_KEY_LIMIT = HASH_SIZE_LIMIT;

struct alignas(4096) SlotArray {
  void *slots[SLOT_INDEX_NR];
};

class Hash {
  SlotArray *head;
  MichaelList list;
  std::atomic<uint32_t> size;
  std::atomic<size_t> slot_nr;
  const size_t expect_max_size;
  const size_t level_nr;
  const size_t load_factor;
  const size_t max_slot_nr;
 public:
  Hash(size_t expect_max_size, size_t load_factor);
  ~Hash();
  MichaelList &get_list() { return this->list; }
  size_t get_expect_max_size() { return this->expect_max_size; }
  size_t get_level_nr() { return this->level_nr; }
  size_t get_load_factor() { return this->load_factor; }
  size_t get_max_slot_nr() { return this->max_slot_nr; }
  uint32_t get_slot_nr() { return this->slot_nr.load(std::memory_order_relaxed); }
  size_t get_size() { return this->size.load(std::memory_order_relaxed); }

  bool Put(uint32_t key, uint32_t value);
  bool Get(uint32_t key, uint32_t &value);
 private:
  Hash(Hash const &) = delete;
  Hash &operator=(Hash const &) = delete;
  uint32_t get_slot_idx(uint32_t key);
  MarkPtrType **get_slot(size_t slot_i, bool create_if_not_exists);
  MarkPtrType **get_or_create_slot(size_t slot_i) {
    return get_slot(slot_i, true);
  }
  MarkPtrType **get_slot_if_exists(size_t slot_i) {
    return get_slot(slot_i, false);
  }
  void free_slots();
  void maybe_resize();
  MarkPtrType *ensure_slot_exists(uint32_t slot_i);
};
} // namespace concurrent
} // namespace grakra
} // namespace com
#endif //CPP_ETUDES_HASH_HH
