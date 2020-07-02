//
// Created by grakra on 20-6-27.
//

#include <concurrent/hash.hh>
namespace com {
namespace grakra {
namespace concurrent {

inline size_t calc_level_nr(size_t expect_max_size) {
  assert(expect_max_size > 0);
  size_t level_nr = 1;
  while (expect_max_size > SLOT_INDEX_NR) {
    expect_max_size = (expect_max_size + SLOT_INDEX_NR - 1) / SLOT_INDEX_NR;
    ++level_nr;
  }
  return level_nr;
}

inline void free_slots(SlotArray *head, size_t level_nr) {
  if (head == nullptr) {
    return;
  }
  assert(level_nr > 0);

  if (level_nr == 1) {
    delete head;
    return;
  }

  std::vector<size_t> slot_indices(level_nr, 0);
  std::vector<SlotArray *> head_stacks;
  head_stacks.reserve(level_nr);
  head_stacks.push_back(head);
  while (!head_stacks.empty()) {
    auto &top = head_stacks.back();
    auto curr_level = head_stacks.size() - 1;
    if (curr_level + 2 < level_nr) {
      size_t &from_idx = slot_indices[curr_level];
      while (from_idx < SLOT_INDEX_NR && top->slots[from_idx] == nullptr)++from_idx;
      if (from_idx < SLOT_INDEX_NR) {
        // scan the current SlotArray's child indexed by from_idx and advance from_idx by 1
        head_stacks.push_back((SlotArray *) (top->slots[from_idx++]));
      } else {
        // all the children of SlotArray have been processed, so go back to its parent.
        delete top;
        head_stacks.pop_back();
      }
    } else {
      for (auto i = 0; i < SLOT_INDEX_NR; ++i) {
        if (top->slots[i] != nullptr) {
          free(top->slots[i]);
        }
      }
      delete top;
      head_stacks.pop_back();
    }
  }
}

static inline void **get_slot(SlotArray *head, size_t slot_i, size_t level_nr) {
  while (level_nr > 1) {
    auto idx = (slot_i >> (level_nr - 1) * SLOT_INDEX_SHIFT) & SLOT_INDEX_MASK;
    auto slot_ptr = &head->slots[idx];
    if (*slot_ptr == nullptr) {
      auto sub_head = new SlotArray();
      memset(sub_head, 0, sizeof(SlotArray));
      assert(sub_head != nullptr);
      auto atomic_slot_ptr = (std::atomic<void *> *) (slot_ptr);
      void *slot_old_value = nullptr;
      if (!atomic_slot_ptr->compare_exchange_strong(slot_old_value, sub_head)) {
        delete sub_head;
      }
    }
    head = (SlotArray *) (*slot_ptr);
    assert(head != nullptr);
    --level_nr;
  }
  return &head->slots[slot_i & SLOT_INDEX_MASK];
}

Hash::Hash(size_t expect_max_size, size_t load_factor)
    : head(nullptr),
      size(0),
      slot_nr(2),
      expect_max_size(expect_max_size),
      level_nr(calc_level_nr(expect_max_size)),
      load_factor(load_factor),
      max_slot_nr((expect_max_size + load_factor - 1) / load_factor) {
  assert(0 < expect_max_size && expect_max_size < HASH_SIZE_LIMIT);
  assert(0 < load_factor && load_factor <= expect_max_size);
  head = new SlotArray();
  assert(head != nullptr);
  auto slot_ptr = get_slot(head, 0, level_nr);
  auto node = new NodeType(0, 0);
  assert(node != nullptr);
  list.Insert(node);
  *slot_ptr = &node->next;
}

Hash::~Hash() {
  if (head != nullptr) {
    free_slots(head, level_nr);
    head = nullptr;
  }
}

uint32_t Hash::get_slot_idx(uint32_t key) {
  return key & slot_nr - 1;
}
void Hash::maybe_resize() {
  auto size_snap = this->size.load(std::memory_order_relaxed);
  auto slot_nr_snap = this->slot_nr.load(std::memory_order_relaxed);
  if (size_snap / slot_nr_snap > load_factor && (slot_nr_snap << 1) < this->max_slot_nr) {
    slot_nr.compare_exchange_strong(slot_nr_snap, slot_nr_snap << 1, std::memory_order_acq_rel);
  }
}
uint32_t dummy_key(uint32_t key) {
  return com::grakra::util::reverse_bits(key);
}

uint32_t regular_key(uint32_t key) {
  return com::grakra::util::reverse_bits(key) | 0x1;
}

MarkPtrType *Hash::ensure_slot_exists(uint32_t slot_i) {
  auto slot_ptr = get_slot(this->head, slot_i, level_nr);
  if (__builtin_expect(*slot_ptr != nullptr, 1)) {
    return (MarkPtrType *) *slot_ptr;
  }

  uint32_t missing_slot_indices[sizeof(uint32_t)];
  ssize_t msi_i = 0;
  missing_slot_indices[msi_i++] = slot_i;
  auto parent_slot_i = PARENT_SLOT(slot_i);
  while (parent_slot_i > 0) {
    slot_ptr = get_slot(this->head, parent_slot_i, level_nr);
    if (*slot_ptr != nullptr) {
      break;
    }
    missing_slot_indices[msi_i++] = parent_slot_i;
    parent_slot_i = PARENT_SLOT(parent_slot_i);
  }

  for (--msi_i; msi_i >= 0; --msi_i) {
    auto missing_slot_i = missing_slot_indices[msi_i];
    auto dummy_node = new NodeType(dummy_key(missing_slot_i), 0);
    assert(dummy_node != nullptr);
    NodeType *exist_node;
    if (!this->list.Insert(dummy_node, &exist_node)) {
      delete dummy_node;
      dummy_node = exist_node;
    }
    auto missing_slot_ptr = get_slot(this->head, missing_slot_i, this->level_nr);
    *missing_slot_ptr = &dummy_node->next;
  }
  return (MarkPtrType *) *slot_ptr;
}
bool Hash::Put(uint32_t key, uint32_t value) {
  assert(key <= HASH_KEY_LIMIT);
  maybe_resize();
  auto slot_i = get_slot_idx(key);
  auto head = ensure_slot_exists(slot_i);
  auto node = new NodeType(regular_key(key), value);
  if (!list.Insert(head, node)) {
    delete node;
    return false;
  }
  return true;
}

bool Hash::Get(uint32_t key, uint32_t &value) {
  assert(key <= HASH_KEY_LIMIT);
  auto slot_i = get_slot_idx(key);
  auto head = ensure_slot_exists(slot_i);
  return list.Search(head, regular_key(key), value);
}

} // namespace concurrent
} // namespace grakra
} // namespace com