//
// Created by grakra on 20-6-28.
//

#ifndef CPP_ETUDES_LIST_HH
#define CPP_ETUDES_LIST_HH
#include <concurrent/mark_ptr_type.hh>
namespace com {
namespace grakra {
namespace concurrent {

#define list_node(markPtr) list_node_(markPtr, NodeType, next)
#define atomic_ptr(ptr) ((std::atomic<typeof(ptr)>*)(&(ptr)))

// NodeType.cmark_next_ctag point to address of cmark_next_ctag field of cmark_next_ctag NodeType
// cmark_next_ctag NodeType can computed from address of cmark_next_ctag field.

struct NodeType {
  MarkPtrType next;
  uint32_t key;
  uint32_t value;
  NodeType(uint32_t key, uint32_t value) : key(key), value(value), next(nullptr) {}
};

class MichaelList {
 private:
  MarkPtrType head;
  //thread_local static MarkPtrType *prev;
  //thread_local static MarkPtrType pmark_curr_ptag;
  //thread_local static MarkPtrType cmark_next_ctag;
 public:
  MichaelList() : head(MarkPtrType(nullptr)) {}
  ~MichaelList() { /*Clear();*/ }
  void Clear();
  bool Insert(MarkPtrType *head, NodeType *node, NodeType **exist_node = nullptr);
  bool Insert(NodeType *node, NodeType **exist_node = nullptr) { return Insert(&this->head, node, exist_node); }
  bool Remove(MarkPtrType *head, uint32_t key);
  bool Remove(uint32_t key) { return Remove(&this->head, key); }
  bool Search(MarkPtrType *head, uint32_t key, uint32_t &value);
  bool Search(uint32_t key, uint32_t &value) { return Search(&this->head, key, value); }
  bool Find(MarkPtrType *head, uint32_t key, NodeType **node = nullptr);
  void Unshift(NodeType *node);
  void Push(NodeType *node);
  NodeType *Shift();
  NodeType *Pop();
  bool IsEmpty();
  std::string ToString();
};

}
}
}
#endif //CPP_ETUDES_LIST_HH
