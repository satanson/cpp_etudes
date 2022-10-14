// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/4.
//

#ifndef CPP_ETUDES_INCLUDE_FUNCTION_ARGS_MORE_OR_LESS_HH_
#define CPP_ETUDES_INCLUDE_FUNCTION_ARGS_MORE_OR_LESS_HH_

#include <cstdint>
#include <ctime>
#include <iostream>
#include <vector>

class ColumnViewer {
public:
    ColumnViewer() {
        _not_const_flag = 0;
        _null_flag = 0;
        data = new int[1];
        data[0] = 5;
        null_data = new uint8_t[1];
        null_data[0] = false;
    }

    inline const int value(const size_t idx) const { return data[idx * _not_const_flag]; }

    inline const bool is_null(const size_t idx) const { return null_data[idx * _null_flag]; }

    inline const bool is_null_just_return_false(const size_t idx) const { return false; }

private:
    int* data;
    uint8_t* null_data;
    int _not_const_flag;
    int _null_flag;
};

class ColumnBuilder {
public:
    ColumnBuilder() : _is_null(false){};
    void reserve() {
        data.reserve(4000);
        null_data.reserve(4000);
    }
    void append_value(int f1) {
        data.emplace_back(f1);
        null_data.emplace_back(false);
    }
    void append_value(int f1, bool is_null) {
        data.emplace_back(f1);
        null_data.emplace_back(is_null);
    }
    void reset() {
        data.clear();
        null_data.clear();
    }

private:
    std::vector<int> data;
    std::vector<uint8_t> null_data;
    bool _is_null;
};

#endif // CPP_ETUDES_INCLUDE_FUNCTION_ARGS_MORE_OR_LESS_HH_
