//
// Created by grakra on 2020/10/20.
//

#include <gtest/gtest.h>
#include <string_functions.hh>

namespace test {
class TestStringFunctions : public testing::Test {
 public:
  template<typename F>
  static inline void compare(F &&f) {
    prepare_utf8_data prepare_data;
    auto &data = prepare_data.data;
    for (auto &s:data) {
      f(s);
    }
  }
};

TEST_F(TestStringFunctions, testGenUtf8String) {
  //auto utf8_1bytes_strings = StringFunctions::gen_utf8_vector({1,0,0,0,0,0}, 100, 10,20);
  //StringFunctions::stat_utf8(utf8_1bytes_strings);
  auto utf8_2bytes_strings = StringFunctions::gen_utf8_vector({1, 1, 3, 400, 0, 0}, 1000, 60, 60);
  StringFunctions::stat_utf8(utf8_2bytes_strings);
}
TEST_F(TestStringFunctions, compare_utf8_length_vs_avx2) {
  compare([](std::string const &s) {
    ASSERT_EQ(StringFunctions::utf8_length(s), StringFunctions::utf8_length_simd_avx2(s));
  });
}

TEST_F(TestStringFunctions, compare_utf8_length_vs_sse2) {
  compare([](std::string const &s) {
    ASSERT_EQ(StringFunctions::utf8_length(s), StringFunctions::utf8_length_simd_sse2(s));
  });
}

TEST_F(TestStringFunctions, compare_utf8_length3) {
  compare([](std::string const &s) {
    ASSERT_EQ(StringFunctions::utf8_length(s), StringFunctions::utf8_length3(s));
  });
}

} // namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  RUN_ALL_TESTS();
}