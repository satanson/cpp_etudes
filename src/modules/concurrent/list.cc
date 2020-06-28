//
// Created by grakra on 20-6-27.
//
#include <concurrent/list.hh>
#include <atomic>
#include <cassert>
namespace com {
namespace grakra {
namespace concurrent {

using MarkPtrType = com::grakra::concurrent::MarkPtrType;

thread_local MarkPtrType *MichaelList::prev(nullptr);
thread_local MarkPtrType MichaelList::pmark_curr_ptag(nullptr);
thread_local MarkPtrType MichaelList::cmark_next_ctag(nullptr);

bool MichaelList::IsEmpty() {
  return this->head.get() == nullptr;
}

void MichaelList::Unshift(NodeType *node) {
  assert(node != nullptr);
  node->next = this->head;
  this->head = MarkPtrType(&node->next, 0, 0);
}

NodeType *MichaelList::Shift() {
  assert(!this->IsEmpty());
  MarkPtrType *p = list_next(this->head);
  this->head = *p;
  return list_node(p);
}

void MichaelList::Push(NodeType *node) {
  assert(node != nullptr);
  MarkPtrType *p = &this->head;
  while (list_next(*p) != nullptr) {
    p = list_next(*p);
  }
  node->next = *p;
  *p = MarkPtrType(&node->next, 0, 0);
}

NodeType *MichaelList::Pop() {
  assert(!this->IsEmpty());
  MarkPtrType *p = &this->head;
  MarkPtrType *pp = nullptr;
  while (list_next(*p) != nullptr) {
    pp = p;
    p = list_next(*p);
  }
  *pp = *p;
  return list_node(p);
}

std::string MichaelList::ToString() {
  std::stringstream ss;
  ss << "MichaelList[";
  MarkPtrType *p = &this->head;
  while (list_next(*p) != nullptr) {
    p = list_next(*p);
    auto node = list_node(p);
    ss << node->key << "=>" << node->value << "; ";
  }
  ss << "]";
  return ss.str();
}

MichaelList::~MichaelList() {
  MarkPtrType *p = list_next(this->head);
  while (p != nullptr) {
    auto node = list_node(p);
    p = list_next(*p);
    delete node;
  }
}

bool MichaelList::Insert(MarkPtrType *head, NodeType *node) {
  while (true) {
    if (Find(head, node->key)) {
      return false;
    }
    node->next = MarkPtrType(list_next(pmark_curr_ptag), 0, 0);
    auto prev_old = MarkPtrType(list_next(pmark_curr_ptag), 0, pmark_curr_ptag.get_tag());
    auto prev_new = MarkPtrType(&node->next, 0, pmark_curr_ptag.get_tag() + 1);
    if (prev->loc.compare_exchange_strong(prev_old.ptr, prev_new.ptr)) {
      return true;
    }
  }
}
bool MichaelList::Remove(MarkPtrType *head, int32_t key) {
  while (true) {
    if (!Find(head, key)) {
      return false;
    }
    auto curr = list_next(pmark_curr_ptag);
    auto curr_new = MarkPtrType(list_next(cmark_next_ctag), 1, cmark_next_ctag.get_tag() + 1);
    if (!curr->loc.compare_exchange_strong(cmark_next_ctag.ptr, curr_new.ptr)) {
      continue;
    }
    auto prev_new = MarkPtrType(list_next(cmark_next_ctag), 0, pmark_curr_ptag.get_tag() + 1);
    if (prev->loc.compare_exchange_strong(pmark_curr_ptag.ptr, prev_new.ptr)) {
      delete list_node(list_next(pmark_curr_ptag));
    } else {
      Find(head, key);
    }
    return true;
  }
}

bool MichaelList::Search(MarkPtrType *head, int32_t key, int32_t &value) {
  if (!Find(head, key)) {
    return false;
  }
  auto node = list_node(list_next(pmark_curr_ptag));
  value = node->value;
  return !list_next(pmark_curr_ptag)->is_mark_delete();
}

bool MichaelList::Find(MarkPtrType *head, int32_t key) {
  assert(head != nullptr);
  try_again:
  prev = head;
  pmark_curr_ptag = *prev;
  while (true) {
    if (list_next(pmark_curr_ptag) == nullptr) {
      return false;
    }
    cmark_next_ctag = *list_next(pmark_curr_ptag);
    auto ckey = list_node(list_next(pmark_curr_ptag))->key;
    // read prev again, if prev is mutated or marked, then retry from scratch.
    auto prev_old = MarkPtrType(pmark_curr_ptag.get(), 0, pmark_curr_ptag.get_tag());
    if (!prev->equal_to(prev_old)) {
      goto try_again;
    }
    if (!cmark_next_ctag.is_mark_delete()) {
      if (ckey >= key) {
        return ckey == key;
      }
      // advance prev
      prev = list_next(pmark_curr_ptag);
    } else {
      auto prev_new = MarkPtrType(cmark_next_ctag.get(), 0, pmark_curr_ptag.get_tag() + 1);
      if (prev->loc.compare_exchange_strong(prev_old.ptr, prev_new.ptr)) {
        delete list_node(list_next(pmark_curr_ptag));
        // now prev->get_tag() == pmark_curr_ptag.get_tag()+1
        cmark_next_ctag.set_tag(pmark_curr_ptag.get_tag() + 1);
      } else {
        goto try_again;
      }
    }
    // advance pmark_curr_ptag, so pmark_curr_ptag keep consistent with *prev
    pmark_curr_ptag = cmark_next_ctag;
  }
}

} // concurrent
} // grakra
} // com