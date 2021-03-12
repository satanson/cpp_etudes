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
#include <list>
#include <memory>

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
  template <typename... Args> void PutReverse(T const &b, Args &&...as) {
    ((put_one(std::forward<Args>(as), b)), ...);
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
  TermDict(PartialOrderRelation<T> const& por): partial_orders(por) {
    term_array.template emplace_back(empty_term());
    term_tree.self = 0;
    term_tree.parent = &term_tree;
  }
  using TermType = std::vector<T>;
  using TermArray = std::vector<TermType>;
  template <typename... Args> void Put(Args &&...terms) {
    ((put_one(std::forward<Args>(terms))), ...);
  }
  TermType BestMatch(TermType const &term) {
    return minimum_superior(term);
  }
  bool IsInvalid(TermType const &term) { return term.empty(); }

private:
  enum POCompareResult {
    GT,
    LT,
    EQ,
    UNKNOWN,
  };

  struct TermTree {
    TermTree() = default;
    TermTree(TermTree &&) = delete;
    TermTree(TermTree &) = delete;
    TermTree &operator=(TermTree &&) const = delete;
    TermTree &operator=(TermTree &) const = delete;
    ~TermTree() {
      auto it = children.begin();
      auto end = children.end();
      for (; it != end; ++it) {
        delete *it;
      }
    }
    size_t self;
    std::list<TermTree *> children;
    TermTree *parent;
  };

  POCompareResult compare_term(TermType const &lhs, TermType const &rhs) {
    if (IsInvalid(rhs)) {
      throw std::invalid_argument("Invalid term");
    }
    if (IsInvalid(lhs)) {
      return POCompareResult::GT;
    }
    auto n = std::min(lhs.size(), rhs.size());
    bool is_eq = (lhs.size() == rhs.size());
    bool is_gt = true;
    bool is_lt = true;
    for (auto i = 0; i < n; ++i) {
      auto &a = lhs[i];
      auto &b = rhs[i];
      if (a == b){
        continue;
      }

      is_eq = false;
      if (partial_orders.Contains(a, b)) {
        is_gt = false;
      } else if (partial_orders.Contains(b, a)) {
        is_lt = false;
      } else {
        return POCompareResult::UNKNOWN;
      }
    }
    if (is_eq) {
      return POCompareResult::EQ;
    }
    if (is_gt) {
      return POCompareResult::GT;
    }
    if (is_lt) {
      return POCompareResult::LT;
    }
    return POCompareResult::UNKNOWN;
  }

  POCompareResult lookup(TermTree *tree, TermType const &new_term,
                         TermTree **node) {
    if (IsInvalid(new_term)) {
      throw std::invalid_argument("Empty term is invalid");
    }
    auto result = compare_term(term_array[tree->self], new_term);
    switch (result) {
    case UNKNOWN:
    case LT:
    case EQ:
      *node = tree;
      return result;
    }
    for (auto& subtree : tree->children) {
      auto child_result = lookup(subtree, new_term, node);
      if (child_result != UNKNOWN) {
        return child_result;
      }
    }
    *node = tree;
    return GT;
  }

  void put_one(TermType const &term) {
    if (IsInvalid(term)) {
      throw std::invalid_argument("Invalid term");
    }
    TermTree *node;
    auto result = lookup(&term_tree, term, &node);
    if (result == EQ) {
      throw std::invalid_argument("Duplicate term");
    }
    if (result == UNKNOWN) {
      throw std::invalid_argument("Impossible error");
    }
    size_t term_idx = term_array.size();
    term_array.push_back(term);
    auto new_node = new TermTree();
    new_node->self = term_idx;
    if (result == LT) {
      // add node into new_node as children
      new_node->children.push_back(node);
      // set new_node's parent to node's parent
      new_node->parent = node->parent;
      // set node's parent to new node
      node->parent = new_node;

      auto child_it = new_node->parent->children.begin();
      auto child_end = new_node->parent->children.end();
      for (; child_it != child_end; ++child_it) {
        if ((*child_it)->self == node->self) {
          *child_it = new_node;
          break;
        }
      }
    } else {
      new_node->parent = node;
      node->children.push_back(new_node);
    }
  }
  TermType minimum_superior(TermType const &term) {
    TermTree *node;
    auto result = lookup(&term_tree, term, &node);
    if (result == EQ) {
      return term_array[node->self];
    }
    if (result == LT) {
      return term_array[node->parent->self];
    }
    if (result == GT) {
      return term_array[node->self];
    }
    return empty_term();
  }
  TermType empty_term() { return TermType(); }
  TermTree term_tree;
  TermArray term_array;
  PartialOrderRelation<T> partial_orders;
};
