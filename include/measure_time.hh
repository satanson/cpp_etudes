// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/08/13.
//

#pragma once
#include <chrono>
class TimeMeasure {
public:
  TimeMeasure(int64_t &cost) : _cost(cost) {
    _start = std::chrono::steady_clock::now();
  }
  ~TimeMeasure() {
    auto end = std::chrono::steady_clock::now();
    _cost = std::chrono::duration_cast<std::chrono::milliseconds>(end-_start).count();
  }
private:
  int64_t &_cost;
  std::chrono::steady_clock::time_point _start;
};