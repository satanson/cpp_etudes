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
      {1, 2, "12"},
      {1, 0, ""},
      {2, 100, "23456789"},
      {9, 1, "9"},
      {9, 100, "9"},
      {10, 1, ""},
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

TEST_F(TestStringFunctions, substr_ascii_no_length) {
  StringVector s;
  s.append("123456789");
  StringVector d;

  std::vector<std::tuple<int, std::string>> cases = {
      {-9, "123456789"},
      {1, "123456789"},
      {2, "23456789"},
      {9, "9"},
      {10, ""},
      {-9, "123456789"},
      {-4, "6789"},
      {-1, "9"}
  };
  for (auto &e:cases) {
    auto[offset, expect] = e;
    StringFunctions::substr<true, false>(s, d, offset, 0);
    ASSERT_EQ(d.get_last_slice().to_string(), expect);
  }

  for (auto &e:cases) {
    auto[offset, expect] = e;
    std::cout << "offset=" << offset << ", expect=" << expect << std::endl;
    StringFunctions::substr<false, false>(s, d, offset, 0);
    ASSERT_EQ(d.get_last_slice().to_string(), expect);
  }
}

TEST_F(TestStringFunctions, substr_zh) {
  std::u8string u8s = u8"壹贰叁肆伍陆柒捌玖";
  StringVector s;
  std::string byte_s(u8s.begin(), u8s.end());
  s.append(byte_s);
  StringVector d;

  std::vector<std::tuple<int, int, std::u8string>> cases = {
      {1, 2, u8"壹贰"},
      {1, 0, u8""},
      {2, 100, u8"贰叁肆伍陆柒捌玖"},
      {9, 1, u8"玖"},
      {9, 100, u8"玖"},
      {10, 1, u8""},
      {-9, 1, u8"壹"},
      {-9, 9, u8"壹贰叁肆伍陆柒捌玖"},
      {-9, 10, u8"壹贰叁肆伍陆柒捌玖"},
      {-4, 1, u8"陆"},
      {-4, 4, u8"陆柒捌玖"},
      {-4, 5, u8"陆柒捌玖"},
      {-1, 1, u8"玖"},
      {-1, 2, u8"玖"}
  };
  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    std::string expect_bytes(expect.begin(), expect.end());
    std::cout << "offset=" << offset << ", len=" << len << ", expect=" << expect_bytes << std::endl;
    StringFunctions::substr<true, true>(s, d, offset, len);
    ASSERT_EQ(d.get_last_slice().to_string(), expect_bytes);
  }

  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    std::string expect_bytes(expect.begin(), expect.end());
    std::cout << "offset=" << offset << ", len=" << len << ", expect=" << expect_bytes << std::endl;
    StringFunctions::substr<false, true>(s, d, offset, len);
    ASSERT_EQ(d.get_last_slice().to_string(), expect_bytes);
  }
}

TEST_F(TestStringFunctions, substr_zh_no_length) {
  std::u8string u8s = u8"壹贰叁肆伍陆柒捌玖";
  StringVector s;
  std::string byte_s(u8s.begin(), u8s.end());
  s.append(byte_s);
  StringVector d;

  std::vector<std::tuple<int, std::u8string>> cases = {
      {1, u8"壹贰叁肆伍陆柒捌玖"},
      {2, u8"贰叁肆伍陆柒捌玖"},
      {3, u8"叁肆伍陆柒捌玖"},
      {4, u8"肆伍陆柒捌玖"},
      {5, u8"伍陆柒捌玖"},
      {6, u8"陆柒捌玖"},
      {7, u8"柒捌玖"},
      {8, u8"捌玖"},
      {9, u8"玖"},
      {10, u8""},
      {-9, u8"壹贰叁肆伍陆柒捌玖"},
      {-8, u8"贰叁肆伍陆柒捌玖"},
      {-7, u8"叁肆伍陆柒捌玖"},
      {-6, u8"肆伍陆柒捌玖"},
      {-5, u8"伍陆柒捌玖"},
      {-4, u8"陆柒捌玖"},
      {-3, u8"柒捌玖"},
      {-2, u8"捌玖"},
      {-1, u8"玖"},
      {-10, u8""},
  };
  for (auto &e:cases) {
    auto[offset, expect] = e;
    std::string expect_bytes(expect.begin(), expect.end());
    StringFunctions::substr<true, false>(s, d, offset, 0);
    std::cout << "offset=" << offset
              << ", expect=" << expect_bytes
              << ", actual=" << d.get_last_slice().to_string()
              << std::endl;
    ASSERT_EQ(d.get_last_slice().to_string(), expect_bytes);
  }

  for (auto &e:cases) {
    auto[offset, expect] = e;
    std::string expect_bytes(expect.begin(), expect.end());
    std::cout << "offset=" << offset << ", expect=" << expect_bytes << std::endl;
    StringFunctions::substr<false, false>(s, d, offset, 0);
    ASSERT_EQ(d.get_last_slice().to_string(), expect_bytes);
  }
}

TEST_F(TestStringFunctions, substr_mixed) {
  std::string s;
  s.append({(char) 0b0111'1111});
  s.append({(char) 0b1101'1111, (char) 0b1011'1111});
  s.append({(char) 0b1110'1111, (char) 0b1011'1111, (char) 0b1011'1111});
  s.append({(char) 0b1111'0111, (char) 0b1011'1111, (char) 0b1011'1111, (char) 0b1011'1111});

  s.append({(char) 0b0111'1111});
  s.append({(char) 0b1101'1111, (char) 0b1011'1111});
  s.append({(char) 0b1110'1111, (char) 0b1011'1111, (char) 0b1011'1111});
  s.append({(char) 0b1111'0111, (char) 0b1011'1111, (char) 0b1011'1111, (char) 0b1011'1111});

  s.append({(char) 0b0111'1111});
  s.append({(char) 0b1101'1111, (char) 0b1011'1111});
  s.append({(char) 0b1110'1111, (char) 0b1011'1111, (char) 0b1011'1111});
  s.append({(char) 0b1111'0111, (char) 0b1011'1111, (char) 0b1011'1111, (char) 0b1011'1111});

  StringVector src;
  StringVector dst;
  src.append(s);

  std::vector<std::tuple<int, int, std::string>>
      cases = {
      {1, 1, "\x7f"},
      {1, 2, "\x7f\xdf\xbf"},
      {1, 3, "\x7f\xdf\xbf\xef\xbf\xbf"},
      {1, 4, "\x7f\xdf\xbf\xef\xbf\xbf\xf7\xbf\xbf\xbf"},
      {1, 100, s},
      {2, 1, "\xdf\xbf"},
      {2, 2, "\xdf\xbf\xef\xbf\xbf"},
      {2, 3, "\xdf\xbf\xef\xbf\xbf\xf7\xbf\xbf\xbf"},
      {2, 4, "\xdf\xbf\xef\xbf\xbf\xf7\xbf\xbf\xbf\x7f"},
      {2, 100, s.substr(1)},
      {3, 1, "\xef\xbf\xbf"},
      {3, 2, "\xef\xbf\xbf\xf7\xbf\xbf\xbf"},
      {3, 3, "\xef\xbf\xbf\xf7\xbf\xbf\xbf\x7f"},
      {3, 4, "\xef\xbf\xbf\xf7\xbf\xbf\xbf\x7f\xdf\xbf"},
      {3, 100, s.substr(3)},
      {4, 2, "\xf7\xbf\xbf\xbf\x7f"},
      {4, 3, "\xf7\xbf\xbf\xbf\x7f\xdf\xbf"},
      {4, 4, "\xf7\xbf\xbf\xbf\x7f\xdf\xbf\xef\xbf\xbf"},
      {4, 100, s.substr(6)},
      {5, 1, "\x7f"},
      {5, 2, "\x7f\xdf\xbf"},
      {5, 3, "\x7f\xdf\xbf\xef\xbf\xbf"},
      {5, 4, "\x7f\xdf\xbf\xef\xbf\xbf\xf7\xbf\xbf\xbf"},
      {5, 100, s.substr(10)},
      {-12, 2, s.substr(0, 3)},
      {-11, 3, s.substr(1, 9)},
      {-10, 4, s.substr(3, 10)},
      {-9, 5, s.substr(6, 14)},
      {-8, 6, s.substr(10, 13)},
      {-7, 7, s.substr(11, 19)},
      {-6, 8, s.substr(13, 17)},
      {-5, 9, s.substr(16, 14)},
      {-4, 10, s.substr(20, 10)},
      {-3, 11, s.substr(21, 9)},
      {-2, 12, s.substr(23, 7)},
      {-1, 13, s.substr(26, 4)},
  };

  for (auto &e:cases) {
    auto[offset, len, expect] = e;
    std::string expect_bytes(expect.begin(), expect.end());
    std::cout << "offset=" << offset << ", len=" << len << ", expect=" << expect_bytes << std::endl;
    StringFunctions::substr<false, true>(src, dst, offset, len);
    ASSERT_EQ(dst.get_last_slice().to_string(), expect_bytes);
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

TEST_F(TestStringFunctions, compareSubstr) {
  prepare_utf8_data data;
  auto &src = data.string_data;
  StringVector dst0;
  StringVector dst1;
  const auto size = src.size();
  StringFunctions::substr_old(src, dst0, 1, 1);
  StringFunctions::substr<true, true>(src, dst1, 1, 1);
  auto &&res0 = dst0.to_vector();
  auto &&res1 = dst1.to_vector();
  for (int i = 0; i < size; ++i) {
    ASSERT_EQ(res0[i], res1[i]);
  }
}

TEST_F(TestStringFunctions, compareSubstr_15_25) {
  setenv("VECTOR_SIZE", "65536", 1);
  setenv("MIN_LENGTH", "15", 1);
  setenv("MAX_LENGTH", "25", 1);
  prepare_utf8_data data;
  auto &src = data.string_data;
  StringVector dst0;
  StringVector dst1;
  const auto size = src.size();
  StringFunctions::substr_old(src, dst0, 5, 10);
  StringFunctions::substr<true, true>(src, dst1, 5, 10);
  auto &&res0 = dst0.to_vector();
  auto &&res1 = dst1.to_vector();
  for (int i = 0; i < size; ++i) {
    ASSERT_EQ(res0[i], res1[i]);
  }
}

TEST_F(TestStringFunctions, upper) {
  std::vector<std::string> cases = {
      "11111111111111abcdefg",
      "1abcdefg",
      "abcdefghigklmnopqrstuvwxyz",
      "aaaaabcdefghigklmnopqrstuvwxyz0000000",
      "aAAAABCDEFGHIGklmnopqrstuvwxyz0000000",
  };
  for (auto &s:cases) {
    ASSERT_EQ(
        StringFunctions::upper_new(StringFunctions::lower_new(StringFunctions::upper_new(s))),
        StringFunctions::upper_old(s));

    ASSERT_EQ(
        StringFunctions::lower_new(StringFunctions::upper_new(s)),
        StringFunctions::lower_old(StringFunctions::upper_old(s)));

    StringVector src;
    src.append(s);
    StringVector dst_upper_old, dst_upper_new, dst_lower_old, dst_lower_new;
    StringFunctions::lower_vector_new(src, dst_lower_new);
    StringFunctions::lower_vector_old(src, dst_lower_old);
    StringFunctions::upper_vector_new(src, dst_upper_new);
    StringFunctions::upper_vector_old(src, dst_upper_old);
    ASSERT_EQ(dst_lower_new.get_last_slice().to_string(), StringFunctions::lower_new(s));
    ASSERT_EQ(dst_lower_old.get_last_slice().to_string(), StringFunctions::lower_new(s));
    ASSERT_EQ(dst_upper_new.get_last_slice().to_string(), StringFunctions::upper_new(s));
    ASSERT_EQ(dst_upper_old.get_last_slice().to_string(), StringFunctions::upper_new(s));
  }
}

TEST_F(TestStringFunctions, case2) {
  std::vector<std::string> cases = {
      "11111111111111abcdefg",
      "1abcdefg",
      "abcdefghigklmnopqrstuvwxyz",
      "aaaaabcdefghigklmnopqrstuvwxyz0000000",
      "aAAAABCDEFGHIGklmnopqrstuvwxyz0000000",
  };

  StringVector src;
  for (auto &s: cases){
    src.append(s);
  }

  StringVector dst_upper_old, dst_upper_new, dst_lower_old, dst_lower_new;
  StringFunctions::lower_vector_new(src, dst_lower_new);
  StringFunctions::lower_vector_old(src, dst_lower_old);
  StringFunctions::upper_vector_new(src, dst_upper_new);
  StringFunctions::upper_vector_old(src, dst_upper_old);
  ASSERT_EQ(dst_upper_old.size(), cases.size());
  ASSERT_EQ(dst_upper_new.size(), cases.size());
  ASSERT_EQ(dst_lower_old.size(), cases.size());
  ASSERT_EQ(dst_lower_new.size(), cases.size());
  for (auto i=0; i < src.size(); ++i){
    auto upper_old_s = dst_upper_old.get_slice(i).to_string();
    auto upper_new_s = dst_upper_new.get_slice(i).to_string();
    auto lower_old_s = dst_lower_old.get_slice(i).to_string();
    auto lower_new_s = dst_lower_new.get_slice(i).to_string();
    ASSERT_EQ(upper_new_s, upper_old_s);
    ASSERT_EQ(lower_new_s, lower_old_s);
  }
}

} // namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}