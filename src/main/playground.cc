// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/10/20.
//
#include<iostream>
#include<vector>
using std::vector;
#include<string>
using std::string;
#include<iostream>
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char **argv) {
  vector<uint8_t> bytes{
    0b1000'0000,
    0b1110'0000,
    0b1111'0000
  };

  for (uint8_t b:bytes){
    int c = ~b;
    cout<<std::hex<<(int)b<<","<<c<<std::dec<<","<<__builtin_clz(c)<<endl;
  }
}
