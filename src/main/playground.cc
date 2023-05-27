// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/20.
//
#include <iostream>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
#include <random>
using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char** argv) {
    std::vector<int> v1;
    std::vector<int> v2;

    std::random_device rd;
    std::uniform_int_distribution<int> rand;
    v1.reserve(100);
    for (int i=0; i < 100; ++i) {
        v1.push_back(rand(rd));
    }
    v2.reserve(1000);
    for (int i=0; i <10; ++i) {
        v2.insert(v2.end(), v1.begin(), v1.end());
    }
    for (auto v: v2) {
        std::cout<<v<<std::endl;
    }

}
