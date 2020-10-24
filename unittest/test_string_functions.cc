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

TEST_F(TestStringFunctions, substr_ascii) {
  StringVector s;
  s.append("123456789");
  StringVector d;

  std::vector<std::tuple<int, int, std::string>> cases = {
      {1, 2, std::string("12")},
      {1, 0, std::string()},
      {2, 100, std::string("23456789")},
      {9, 1, std::string("9")},
      {9, 100, std::string("9")},
      {10, 1, std::string()},
      {-9, 1, "1"},
      {-9, 9, "123456789"},
      {-9, 10, "123456789"},
      {-4, 1, "6"},
      {-4, 4, "6789"},
      {-4, 5, "6789"},
      {-1, 1, "9"},
      {-1, 2, "9"}
  };
  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    StringFunctions::substr<true, true>(s, d, offset, len);
    ASSERT_EQ(d.get_last_slice().to_string(), expect);
  }

  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    std::cout << "offset=" << offset << ", len=" << len << ", expect=" << expect << std::endl;
    StringFunctions::substr<false, true>(s, d, offset, len);
    ASSERT_EQ(d.get_last_slice().to_string(), expect);
  }
}

TEST_F(TestStringFunctions, substr_zh) {
  StringVector s;
  s.append("");
  StringVector d;

  std::vector<std::tuple<int, int, std::string>> cases = {
      {1, 2, std::string("12")},
      {1, 0, std::string()},
      {2, 100, std::string("23456789")},
      {9, 1, std::string("9")},
      {9, 100, std::string("9")},
      {10, 1, std::string()},
      {-9, 1, "1"},
      {-9, 9, "123456789"},
      {-9, 10, "123456789"},
      {-4, 1, "6"},
      {-4, 4, "6789"},
      {-4, 5, "6789"},
      {-1, 1, "9"},
      {-1, 2, "9"}
  };
  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    StringFunctions::substr<true, true>(s, d, offset, len);
    ASSERT_EQ(d.get_last_slice().to_string(), expect);
  }

  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    std::cout << "offset=" << offset << ", len=" << len << ", expect=" << expect << std::endl;
    StringFunctions::substr<false, true>(s, d, offset, len);
    ASSERT_EQ(d.get_last_slice().to_string(), expect);
  }
}

TEST_F(TestStringFunctions, append) {

  std::u8string u8s = u8"壹贰叁肆伍陆柒捌玖拾";
  std::string s(u8s.begin(), u8s.end());
  std::cout << s << std::endl;
  std::cout << s.size() << std::endl;
  std::cout << std::hex << (unsigned) (uint8_t) s[0] << std::endl;
  std::cout << std::hex << (unsigned) (uint8_t) s[1] << std::endl;
}

} // namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}