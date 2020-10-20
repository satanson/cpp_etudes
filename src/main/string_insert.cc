//
// Created by grakra on 2020/10/20.
//
#include<string>
#include<cstdint>
#include<vector>
#include<cstdlib>

int main(int argc, char** argv){
  std::string s0(10, 'x');
  std::vector<uint8_t> s1;
  s1.resize(s0.size());
  s1.insert(s1.begin(), (uint8_t*)s0.data(), (uint8_t*)s0.data()+s0.size());
  s0.append("y");
  printf("s0=%s, s1=%s", s0.c_str(), (uint8_t*)s1.data());
  return 0;
}