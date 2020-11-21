// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/11/10.
//

#ifndef CPP_ETUDES_INCLUDE_RAW_CONTAINER_HH_
#define CPP_ETUDES_INCLUDE_RAW_CONTAINER_HH_

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace raw {
// C++ Reference recommend to use this allocator implementation to
// prevent containers resize invocation from initializing the allocated
// memory space unnecessarily.
// https://stackoverflow.com/questions/21028299/is-this-behavior-of-vectorresizesize-type-n-under-c11-and-boost-container/21028912#21028912
// Allocator adaptor that interposes construct() calls to
// convert value initialization into default initialization.
template<typename T, typename A = std::allocator<T>>
class RawAllocator : public A {
  typedef std::allocator_traits<A> a_t;

 public:
  template<typename U>
  struct rebind {
    using other = RawAllocator<U, typename a_t::template rebind_alloc<U>>;
  };

  using A::A;

  template<typename U>
  void construct(U *ptr) noexcept(std::is_nothrow_default_constructible<U>::value) {
    ::new(static_cast<void *>(ptr)) U;
  }
  template<typename U, typename... Args>
  void construct(U *ptr, Args &&... args) {
    a_t::construct(static_cast<A &>(*this), ptr, std::forward<Args>(args)...);
  }
};

using RawString = std::basic_string<char, std::char_traits<char>, RawAllocator<char>>;
// From cpp reference: "A trivial destructor is a destructor that performs no action. Objects with
// trivial destructors don't require a delete-expression and may be disposed of by simply
// deallocating their storage. All data types compatible with the C language (POD types)
// are trivially destructible."
// Types with trivial destructors is safe when when move content from a RawVector<T> into
// a std::vector<U> and both T and U the same bit width, i.e.
// doris::raw::RawVector<int8_t> a;
// a.resize(100);
// std::vector<uint8_t> b = std::move(reinterpret_cast<std::vector<uint8_t>&>(a));
template<class T, std::enable_if_t<std::is_trivially_destructible_v<T>, T> = 0>
using RawVector = std::vector<T, RawAllocator<T>>;

template<class T, std::enable_if_t<std::is_trivially_destructible_v<T>, T> = 0>
static inline void make_room(std::vector<T> &v, size_t n) {
  RawVector<T> rv;
  rv.resize(n);
  v.swap(reinterpret_cast<std::vector<T> &>(rv));
}
static inline void make_room(std::string &v, size_t n) {
  RawString rs;
  rs.resize(n);
  v.swap(reinterpret_cast<std::string &>(rs));
}
} // namespace raw

#endif //CPP_ETUDES_INCLUDE_RAW_CONTAINER_HH_
