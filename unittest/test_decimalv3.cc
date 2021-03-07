//
// Created by grakra on 2020/12/7.
//

#include <decimal/decimal.hh>
#include <decimalv3.hh>
#include <gtest/gtest.h>
#include <iostream>
namespace test {
class TestDecimalV3 : public testing::Test {};

TEST_F(TestDecimalV3, EXP10) {
  std::cout << EXP10<int32_t, 0>::value << std::endl;
  std::cout << EXP10<int32_t, 1>::value << std::endl;
  std::cout << EXP10<int32_t, 2>::value << std::endl;
  std::cout << EXP10<int32_t, 3>::value << std::endl;
  std::cout << EXP10<int32_t, 4>::value << std::endl;
  std::cout << EXP10<int64_t, 18>::value << std::endl;
  std::cout << to_string(EXP10<int128_t, 18>::value, 27, 9) << std::endl;
  std::cout << to_string(EXP10<int128_t, 27>::value, 27, 9) << std::endl;
  std::cout << is_underlying_type_of_decimal<int32_t> << std::endl;
}
template <typename T>
constexpr bool is_integer = (std::is_integral_v<T> && std::is_signed_v<T>) ||
                            std::is_same_v<T, int128_t>;
TEST_F(TestDecimalV3, Int32MulFloat32) {
  float a = 2147483647.0f;
  int32_t b = 10;
  int32_t c = a * b;
  std::cout << "a=" << a << std::endl;
  std::cout << "b=" << b << std::endl;
  std::cout << "c=" << c << std::endl;
  std::cout << "int8_t:" << is_integer<int8_t> << std::endl;
  std::cout << "uint8_t:" << is_integer<uint8_t> << std::endl;
  std::cout << "int8_t:" << is_integer<int16_t> << std::endl;
  std::cout << "uint8_t:" << is_integer<uint16_t> << std::endl;
  std::cout << "int32_t:" << is_integer<int32_t> << std::endl;
  std::cout << "uint32_t:" << is_integer<uint32_t> << std::endl;
  std::cout << "int64_t:" << is_integer<int64_t> << std::endl;
  std::cout << "uint64_t:" << is_integer<uint64_t> << std::endl;
  std::cout << "int128_t:" << is_integer<int128_t> << std::endl;
  std::cout << "uint128_t:" << is_integer<uint128_t> << std::endl;
}

} // namespace test

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}