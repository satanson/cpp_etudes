// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 20-6-27.
//

#ifndef CPP_ETUDES_TAGGED_POINTER_HH
#define CPP_ETUDES_TAGGED_POINTER_HH
#include <atomic>
#include <cstdint>
#include <sstream>
#include <string>
namespace com {
namespace grakra {
namespace concurrent {
#define typeof __typeof__
#define list_node_(ptr, type, member)                                \
    ({                                                               \
        typeof(ptr) __mptr = (ptr);                                  \
        (type*)(((char*)__mptr) - __builtin_offsetof(type, member)); \
    })
#define list_next(markPtr) (reinterpret_cast<MarkPtrType*>((markPtr).get()))

// in x86_64 arch, Linux's linear addresses use the lowest 48 bits,
// highest 16 bits are identical to 47th bit(MSB of linear addresses),
// so we can use highest 16 bits for keeping tag.
union MarkPtrType {
public:
    void* ptr;
    struct {
        uint16_t mark : 1;
        const uint64_t p : 46;
        const uint16_t sign : 1;
        uint16_t tag : 16;
    };

    MarkPtrType() : ptr(nullptr) {
        this->mark = 0;
        this->tag = 0;
    }

    explicit MarkPtrType(void* ptr) : ptr(ptr) {
        this->mark = 0;
        this->tag = 0;
    }

    MarkPtrType(void* ptr, uint8_t mark, uint16_t tag) : ptr(ptr) {
        this->mark = mark;
        this->tag = tag;
    }

    MarkPtrType(MarkPtrType const& other) : ptr(other.ptr) {}

    MarkPtrType& operator=(MarkPtrType const& other) {
        this->ptr = other.ptr;
        return *this;
    }

    void next(MarkPtrType const& next) { *this = next; }

    MarkPtrType* next() { return list_next(*this); }

    void* get() {
        MarkPtrType p = *this;
        p.mark = 0;
        p.tag = p.sign == 1 ? 0xffff : 0;
        return p.ptr;
    }

    MarkPtrType& mark_delele() {
        this->mark = 1;
        return *this;
    }

    MarkPtrType& unmark_delete() {
        this->mark = 0;
        return *this;
    }

    bool is_mark_delete() { return this->mark == 1; }

    uint16_t get_tag() { return this->tag; }

    MarkPtrType& set_tag(uint16_t tag) {
        this->tag = tag;
        return *this;
    }

    std::string ToString() {
        std::stringstream ss;
        ss << "MarkPtrType{tag=" << this->tag << ", mark=" << (this->mark) << ", real_ptr=" << this->get() << "}";
        return ss.str();
    }
    bool equal_to(MarkPtrType const& other) { return this->ptr == other.ptr; }
};

} // namespace concurrent
} // namespace grakra
} // namespace com

#endif // CPP_ETUDES_TAGGED_POINTER_HH
