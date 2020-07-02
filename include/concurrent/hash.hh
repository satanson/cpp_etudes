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

constexpr size_t SLOT_INDEX_SHIFT = 12 - LOG2(sizeof(void *) / 8);
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
  MichaelList &GetList() { return this->list; }
  bool Put(uint32_t key, uint32_t value);
  bool Get(uint32_t key, uint32_t &value);
 private:
  Hash(Hash const &) = delete;
  Hash &operator=(Hash const &) = delete;
  uint32_t get_slot_idx(uint32_t key);
  void maybe_resize();
  MarkPtrType *ensure_slot_exists(uint32_t slot_i);
};
} // namespace concurrent
} // namespace grakra
} // namespace com
#endif //CPP_ETUDES_HASH_HH
