// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/20.
//
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string s0(10, 'x');
    std::vector<uint8_t> s1;
    s1.resize(s0.size());
    s1.insert(s1.begin(), (uint8_t*)s0.data(), (uint8_t*)s0.data() + s0.size());
    s0.append("y");
    printf("s0=%s, s1=%s", s0.c_str(), (uint8_t*)s1.data());
    return 0;
}
