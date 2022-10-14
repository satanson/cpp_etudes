// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#pragma once
#include <atomic>
#include <optional>
namespace async {
template <typename T>
class ConcurrentLinkedList {
    struct Node {
        std::atomic<Node*> next;
        T value;
    };

public:
    void add(const T& t) {
        auto* node = new Node{.next = head.next, .value = t};
        auto* old_head = head.next;
        while (!head.next.compare_exchange_strong(old_head, node)) {
            node->next = head.next;
        }
    }

private:
    Node head;
};

} // namespace async