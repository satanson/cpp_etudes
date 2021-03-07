// CPP_ETUDES_INCLUDE_FUNCTION_MATCH_HH
// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/01.
//

#pragma once
#include "unordered_set"
#include "gtest/gtest.h"

// Collections for partial order relation over T^2
template <typename T> class PartialOrderRelation {
public:
  PartialOrderRelation() = default;
  PartialOrderRelation(PartialOrderRelation const &) = default;
  PartialOrderRelation &operator=(PartialOrderRelation const &) = default;
  ~PartialOrderRelation() = default;
  bool Contains(T const &a, T const &b) { return _r.count({a, b}); }

  template <typename... Args> void Put(T const &a, Args &&...bs) {
    ((put_one(a, std::forward<Args>(bs))), ...);
  }

private:
  struct pair_hash {
    size_t operator()(std::pair<T, T> const &e) const {
      std::hash<T> hash_fn;
      return hash_fn(e.first) * 31 + hash_fn(e.second);
    }
  };

  void put_one(T const &a, T const &b) {
    if (_r.count({a, b})) {
      throw std::invalid_argument("Relation already exists!");
    }
    if (_r.count({b, a})) {
      throw std::invalid_argument("Partial order violated!");
    }
    _r.emplace(a, b);
  }
  std::unordered_set<std::pair<T, T>, pair_hash> _r;
};

// lexical order

template <typename T> class TermDict {
public:
  using TermType = std::vector<T>;
  template <typename... Args> void Put(Args &&...terms) {
    ((put_one(std::forward<Args>(terms))), ...);
  }
  TermType BestMatch(TermType const &term) {
    if (exact_match(term)){
      return term;
    }
    return minimum_superior(term);
  }
  bool IsInvalid(TermType const &term) { return term.empty(); }

private:
  void put_one(TermType const &term) {
    if (IsInvalid(term)) {
      throw std::invalid_argument("Empty term is invalid");
    }
  }

  bool exact_match(TermType const &term) { return true; }
  TermType minimum_superior(TermType const &term) { return empty_term(); }
  TermType empty_term() { return TermType(); }
};
