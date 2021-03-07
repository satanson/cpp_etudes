// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git
//
// Created by grakra on 2020/12/19.
//

#include <gtest/gtest.h>
#include <guard.hh>

class TestGuard : public ::testing::Test {};

using int128_t = __int128;
using uint128_t = unsigned __int128;

struct Decimal32 {
  int32_t d;
};
struct Decimal64 {
  int64_t d;
};
struct Decimal128 {
  int128_t d;
};
struct Char {
  char *p;
  size_t len;
};
struct VarChar {
  char *p;
  size_t len;
};

TYPE_GUARD(BoolGuard, is_bool, bool)
TYPE_GUARD(SignedIntegerGuard, is_signed_integer, int8_t, int16_t, int32_t,
           int64_t, int128_t)
TYPE_GUARD(UnsignedIntegerGuard, is_unsigned_integer, uint8_t, uint16_t,
           uint32_t, uint64_t, uint128_t)
TYPE_GUARD(FloatGuard, is_float, float, double)
TYPE_GUARD(DecimalGuard, is_decimal, Decimal32, Decimal64, Decimal128)
TYPE_GUARD(CharGuard, is_char, Char, VarChar)
UNION_TYPE_GUARD(IntegerGuard, is_integer, is_signed_integer_struct,
                 is_unsigned_integer_struct)
UNION_TYPE_GUARD(NumberGuard, is_number, is_integer_struct, is_bool_struct,
                 is_float_struct)
UNION_TYPE_GUARD(NumericGuard, is_numeric, is_number_struct, is_decimal_struct)

TEST_F(TestGuard, testTypePredicates) {
  ASSERT_TRUE(is_numeric<bool>);
  ASSERT_TRUE(is_numeric<int8_t>);
  ASSERT_TRUE(is_numeric<int16_t>);
  ASSERT_TRUE(is_numeric<int32_t>);
  ASSERT_TRUE(is_numeric<int64_t>);
  ASSERT_TRUE(is_numeric<int128_t>);
  ASSERT_TRUE(is_numeric<uint8_t>);
  ASSERT_TRUE(is_numeric<uint16_t>);
  ASSERT_TRUE(is_numeric<uint32_t>);
  ASSERT_TRUE(is_numeric<uint64_t>);
  ASSERT_TRUE(is_numeric<uint128_t>);
  ASSERT_TRUE(is_numeric<float>);
  ASSERT_TRUE(is_numeric<double>);
  ASSERT_TRUE(is_numeric<Decimal32>);
  ASSERT_TRUE(is_numeric<Decimal64>);
  ASSERT_TRUE(is_numeric<Decimal128>);
  ASSERT_FALSE(is_numeric<Char>);
  ASSERT_FALSE(is_numeric<VarChar>);

  ASSERT_TRUE(is_number<bool>);
  ASSERT_TRUE(is_number<int8_t>);
  ASSERT_TRUE(is_number<int16_t>);
  ASSERT_TRUE(is_number<int32_t>);
  ASSERT_TRUE(is_number<int64_t>);
  ASSERT_TRUE(is_number<int128_t>);
  ASSERT_TRUE(is_number<uint8_t>);
  ASSERT_TRUE(is_number<uint16_t>);
  ASSERT_TRUE(is_number<uint32_t>);
  ASSERT_TRUE(is_number<uint64_t>);
  ASSERT_TRUE(is_number<uint128_t>);
  ASSERT_TRUE(is_number<float>);
  ASSERT_TRUE(is_number<double>);
  ASSERT_FALSE(is_number<Decimal32>);
  ASSERT_FALSE(is_number<Decimal64>);
  ASSERT_FALSE(is_number<Decimal128>);
  ASSERT_FALSE(is_number<Char>);
  ASSERT_FALSE(is_number<VarChar>);

  ASSERT_FALSE(is_char<bool>);
  ASSERT_FALSE(is_char<int8_t>);
  ASSERT_FALSE(is_char<int16_t>);
  ASSERT_FALSE(is_char<int32_t>);
  ASSERT_FALSE(is_char<int64_t>);
  ASSERT_FALSE(is_char<int128_t>);
  ASSERT_FALSE(is_char<uint8_t>);
  ASSERT_FALSE(is_char<uint16_t>);
  ASSERT_FALSE(is_char<uint32_t>);
  ASSERT_FALSE(is_char<uint64_t>);
  ASSERT_FALSE(is_char<uint128_t>);
  ASSERT_FALSE(is_char<float>);
  ASSERT_FALSE(is_char<double>);
  ASSERT_FALSE(is_char<Decimal32>);
  ASSERT_FALSE(is_char<Decimal64>);
  ASSERT_FALSE(is_char<Decimal128>);
  ASSERT_TRUE(is_char<Char>);
  ASSERT_TRUE(is_char<VarChar>);

  ASSERT_FALSE(is_decimal<bool>);
  ASSERT_FALSE(is_decimal<int8_t>);
  ASSERT_FALSE(is_decimal<int16_t>);
  ASSERT_FALSE(is_decimal<int32_t>);
  ASSERT_FALSE(is_decimal<int64_t>);
  ASSERT_FALSE(is_decimal<int128_t>);
  ASSERT_FALSE(is_decimal<uint8_t>);
  ASSERT_FALSE(is_decimal<uint16_t>);
  ASSERT_FALSE(is_decimal<uint32_t>);
  ASSERT_FALSE(is_decimal<uint64_t>);
  ASSERT_FALSE(is_decimal<uint128_t>);
  ASSERT_FALSE(is_decimal<float>);
  ASSERT_FALSE(is_decimal<double>);
  ASSERT_TRUE(is_decimal<Decimal32>);
  ASSERT_TRUE(is_decimal<Decimal64>);
  ASSERT_TRUE(is_decimal<Decimal128>);
  ASSERT_FALSE(is_decimal<Char>);
  ASSERT_FALSE(is_decimal<VarChar>);

  ASSERT_TRUE(is_bool<bool>);
  ASSERT_FALSE(is_bool<int8_t>);
  ASSERT_FALSE(is_bool<int16_t>);
  ASSERT_FALSE(is_bool<int32_t>);
  ASSERT_FALSE(is_bool<int64_t>);
  ASSERT_FALSE(is_bool<int128_t>);
  ASSERT_FALSE(is_bool<uint8_t>);
  ASSERT_FALSE(is_bool<uint16_t>);
  ASSERT_FALSE(is_bool<uint32_t>);
  ASSERT_FALSE(is_bool<uint64_t>);
  ASSERT_FALSE(is_bool<uint128_t>);
  ASSERT_FALSE(is_bool<float>);
  ASSERT_FALSE(is_bool<double>);
  ASSERT_FALSE(is_bool<Decimal32>);
  ASSERT_FALSE(is_bool<Decimal64>);
  ASSERT_FALSE(is_bool<Decimal128>);
  ASSERT_FALSE(is_bool<Char>);
  ASSERT_FALSE(is_bool<VarChar>);

  ASSERT_FALSE(is_float<bool>);
  ASSERT_FALSE(is_float<int8_t>);
  ASSERT_FALSE(is_float<int16_t>);
  ASSERT_FALSE(is_float<int32_t>);
  ASSERT_FALSE(is_float<int64_t>);
  ASSERT_FALSE(is_float<int128_t>);
  ASSERT_FALSE(is_float<uint8_t>);
  ASSERT_FALSE(is_float<uint16_t>);
  ASSERT_FALSE(is_float<uint32_t>);
  ASSERT_FALSE(is_float<uint64_t>);
  ASSERT_FALSE(is_float<uint128_t>);
  ASSERT_TRUE(is_float<float>);
  ASSERT_TRUE(is_float<double>);
  ASSERT_FALSE(is_float<Decimal32>);
  ASSERT_FALSE(is_float<Decimal64>);
  ASSERT_FALSE(is_float<Decimal128>);
  ASSERT_FALSE(is_float<Char>);
  ASSERT_FALSE(is_float<VarChar>);

  ASSERT_FALSE(is_unsigned_integer<bool>);
  ASSERT_FALSE(is_unsigned_integer<int8_t>);
  ASSERT_FALSE(is_unsigned_integer<int16_t>);
  ASSERT_FALSE(is_unsigned_integer<int32_t>);
  ASSERT_FALSE(is_unsigned_integer<int64_t>);
  ASSERT_FALSE(is_unsigned_integer<int128_t>);
  ASSERT_TRUE(is_unsigned_integer<uint8_t>);
  ASSERT_TRUE(is_unsigned_integer<uint16_t>);
  ASSERT_TRUE(is_unsigned_integer<uint32_t>);
  ASSERT_TRUE(is_unsigned_integer<uint64_t>);
  ASSERT_TRUE(is_unsigned_integer<uint128_t>);
  ASSERT_FALSE(is_unsigned_integer<float>);
  ASSERT_FALSE(is_unsigned_integer<double>);
  ASSERT_FALSE(is_unsigned_integer<Decimal32>);
  ASSERT_FALSE(is_unsigned_integer<Decimal64>);
  ASSERT_FALSE(is_unsigned_integer<Decimal128>);
  ASSERT_FALSE(is_unsigned_integer<Char>);
  ASSERT_FALSE(is_unsigned_integer<VarChar>);

  ASSERT_FALSE(is_signed_integer<bool>);
  ASSERT_TRUE(is_signed_integer<int8_t>);
  ASSERT_TRUE(is_signed_integer<int16_t>);
  ASSERT_TRUE(is_signed_integer<int32_t>);
  ASSERT_TRUE(is_signed_integer<int64_t>);
  ASSERT_TRUE(is_signed_integer<int128_t>);
  ASSERT_FALSE(is_signed_integer<uint8_t>);
  ASSERT_FALSE(is_signed_integer<uint16_t>);
  ASSERT_FALSE(is_signed_integer<uint32_t>);
  ASSERT_FALSE(is_signed_integer<uint64_t>);
  ASSERT_FALSE(is_signed_integer<uint128_t>);
  ASSERT_FALSE(is_signed_integer<float>);
  ASSERT_FALSE(is_signed_integer<double>);
  ASSERT_FALSE(is_signed_integer<Decimal32>);
  ASSERT_FALSE(is_signed_integer<Decimal64>);
  ASSERT_FALSE(is_signed_integer<Decimal128>);
  ASSERT_FALSE(is_signed_integer<Char>);
  ASSERT_FALSE(is_signed_integer<VarChar>);

  ASSERT_FALSE(is_integer<bool>);
  ASSERT_TRUE(is_integer<int8_t>);
  ASSERT_TRUE(is_integer<int16_t>);
  ASSERT_TRUE(is_integer<int32_t>);
  ASSERT_TRUE(is_integer<int64_t>);
  ASSERT_TRUE(is_integer<int128_t>);
  ASSERT_TRUE(is_integer<uint8_t>);
  ASSERT_TRUE(is_integer<uint16_t>);
  ASSERT_TRUE(is_integer<uint32_t>);
  ASSERT_TRUE(is_integer<uint64_t>);
  ASSERT_TRUE(is_integer<uint128_t>);
  ASSERT_FALSE(is_integer<float>);
  ASSERT_FALSE(is_integer<double>);
  ASSERT_FALSE(is_integer<Decimal32>);
  ASSERT_FALSE(is_integer<Decimal64>);
  ASSERT_FALSE(is_integer<Decimal128>);
  ASSERT_FALSE(is_integer<Char>);
  ASSERT_FALSE(is_integer<VarChar>);
}
template <typename T, typename = guard::Guard> struct ReturnString {
  static inline std::string get() { return "unknown"; }
};

template <typename T> struct ReturnString<T, FloatGuard<T>> {
  static inline std::string get() { return "float"; }
};

template <typename T> struct ReturnString<T, IntegerGuard<T>> {
  static inline std::string get() { return "integer"; }
};

template <typename T> struct ReturnString<T, BoolGuard<T>> {
  static inline std::string get() { return "bool"; }
};

template <typename T> struct ReturnString<T, DecimalGuard<T>> {
  static inline std::string get() { return "decimal"; }
};

template <typename T> struct ReturnString<T, CharGuard<T>> {
  static inline std::string get() { return "char"; }
};

TEST_F(TestGuard, testTypeGuard) {
  ASSERT_EQ(ReturnString<bool>::get(), "bool");
  ASSERT_EQ(ReturnString<int8_t>::get(), "integer");
  ASSERT_EQ(ReturnString<int16_t>::get(), "integer");
  ASSERT_EQ(ReturnString<int32_t>::get(), "integer");
  ASSERT_EQ(ReturnString<int64_t>::get(), "integer");
  ASSERT_EQ(ReturnString<int128_t>::get(), "integer");
  ASSERT_EQ(ReturnString<uint8_t>::get(), "integer");
  ASSERT_EQ(ReturnString<uint16_t>::get(), "integer");
  ASSERT_EQ(ReturnString<uint32_t>::get(), "integer");
  ASSERT_EQ(ReturnString<uint64_t>::get(), "integer");
  ASSERT_EQ(ReturnString<uint128_t>::get(), "integer");
  ASSERT_EQ(ReturnString<float>::get(), "float");
  ASSERT_EQ(ReturnString<double>::get(), "float");
  ASSERT_EQ(ReturnString<Decimal32>::get(), "decimal");
  ASSERT_EQ(ReturnString<Decimal64>::get(), "decimal");
  ASSERT_EQ(ReturnString<Decimal128>::get(), "decimal");
  ASSERT_EQ(ReturnString<Char>::get(), "char");
  ASSERT_EQ(ReturnString<VarChar>::get(), "char");
  ASSERT_EQ(ReturnString<std::string>::get(), "unknown");
}
enum DB_TYPE {
  DB_TYPE_BOOLEAN,
  DB_TYPE_TINYINT,
  DB_TYPE_SMALLINT,
  DB_TYPE_INT,
  DB_TYPE_BIGINT,
  DB_TYPE_LARGEINT,
  DB_TYPE_FLOAT,
  DB_TYPE_DOUBLE,
  DB_TYPE_DECIMAL32,
  DB_TYPE_DECIMAL64,
  DB_TYPE_DECIMAL128,
  DB_TYPE_CHAR,
  DB_TYPE_VARCHAR,
  DB_TYPE_INVALID,
};

VALUE_GUARD(DB_TYPE, BooleanTypeGuard, is_bool_type, DB_TYPE_BOOLEAN)
VALUE_GUARD(DB_TYPE, IntegerTypeGuard, is_integer_type, DB_TYPE_TINYINT,
            DB_TYPE_SMALLINT, DB_TYPE_INT, DB_TYPE_BIGINT, DB_TYPE_LARGEINT);
VALUE_GUARD(DB_TYPE, FloatTypeGuard, is_float_type, DB_TYPE_FLOAT,
            DB_TYPE_DOUBLE)
VALUE_GUARD(DB_TYPE, DecimalTypeGuard, is_decimal_type, DB_TYPE_DECIMAL32,
            DB_TYPE_DECIMAL64, DB_TYPE_DECIMAL128)
VALUE_GUARD(DB_TYPE, CharTypeGuard, is_char_type, DB_TYPE_CHAR, DB_TYPE_VARCHAR)
UNION_VALUE_GUARD(DB_TYPE, NumberTypeGuard, is_number_type, is_bool_type_struct,
                  is_integer_type_struct, is_float_type_struct);
UNION_VALUE_GUARD(DB_TYPE, NumericTypeGuard, is_numeric_type,
                  is_number_type_struct, is_decimal_type_struct)

TEST_F(TestGuard, testValuePredicates) {
  ASSERT_TRUE(is_numeric_type<DB_TYPE_BOOLEAN>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_TINYINT>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_SMALLINT>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_INT>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_BIGINT>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_LARGEINT>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_FLOAT>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_DOUBLE>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_DECIMAL32>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_DECIMAL64>);
  ASSERT_TRUE(is_numeric_type<DB_TYPE_DECIMAL128>);
  ASSERT_FALSE(is_numeric_type<DB_TYPE_CHAR>);
  ASSERT_FALSE(is_numeric_type<DB_TYPE_VARCHAR>);

  ASSERT_TRUE(is_number_type<DB_TYPE_BOOLEAN>);
  ASSERT_TRUE(is_number_type<DB_TYPE_TINYINT>);
  ASSERT_TRUE(is_number_type<DB_TYPE_SMALLINT>);
  ASSERT_TRUE(is_number_type<DB_TYPE_INT>);
  ASSERT_TRUE(is_number_type<DB_TYPE_BIGINT>);
  ASSERT_TRUE(is_number_type<DB_TYPE_LARGEINT>);
  ASSERT_TRUE(is_number_type<DB_TYPE_FLOAT>);
  ASSERT_TRUE(is_number_type<DB_TYPE_DOUBLE>);
  ASSERT_FALSE(is_number_type<DB_TYPE_DECIMAL32>);
  ASSERT_FALSE(is_number_type<DB_TYPE_DECIMAL64>);
  ASSERT_FALSE(is_number_type<DB_TYPE_DECIMAL128>);
  ASSERT_FALSE(is_number_type<DB_TYPE_CHAR>);
  ASSERT_FALSE(is_number_type<DB_TYPE_VARCHAR>);

  ASSERT_FALSE(is_char_type<DB_TYPE_BOOLEAN>);
  ASSERT_FALSE(is_char_type<DB_TYPE_TINYINT>);
  ASSERT_FALSE(is_char_type<DB_TYPE_SMALLINT>);
  ASSERT_FALSE(is_char_type<DB_TYPE_INT>);
  ASSERT_FALSE(is_char_type<DB_TYPE_BIGINT>);
  ASSERT_FALSE(is_char_type<DB_TYPE_LARGEINT>);
  ASSERT_FALSE(is_char_type<DB_TYPE_FLOAT>);
  ASSERT_FALSE(is_char_type<DB_TYPE_DOUBLE>);
  ASSERT_FALSE(is_char_type<DB_TYPE_DECIMAL32>);
  ASSERT_FALSE(is_char_type<DB_TYPE_DECIMAL64>);
  ASSERT_FALSE(is_char_type<DB_TYPE_DECIMAL128>);
  ASSERT_TRUE(is_char_type<DB_TYPE_CHAR>);
  ASSERT_TRUE(is_char_type<DB_TYPE_VARCHAR>);

  ASSERT_FALSE(is_decimal_type<DB_TYPE_BOOLEAN>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_TINYINT>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_SMALLINT>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_INT>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_BIGINT>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_LARGEINT>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_FLOAT>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_DOUBLE>);
  ASSERT_TRUE(is_decimal_type<DB_TYPE_DECIMAL32>);
  ASSERT_TRUE(is_decimal_type<DB_TYPE_DECIMAL64>);
  ASSERT_TRUE(is_decimal_type<DB_TYPE_DECIMAL128>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_CHAR>);
  ASSERT_FALSE(is_decimal_type<DB_TYPE_VARCHAR>);

  ASSERT_TRUE(is_bool_type<DB_TYPE_BOOLEAN>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_TINYINT>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_SMALLINT>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_INT>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_BIGINT>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_LARGEINT>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_FLOAT>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_DOUBLE>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_DECIMAL32>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_DECIMAL64>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_DECIMAL128>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_CHAR>);
  ASSERT_FALSE(is_bool_type<DB_TYPE_VARCHAR>);

  ASSERT_FALSE(is_float_type<DB_TYPE_BOOLEAN>);
  ASSERT_FALSE(is_float_type<DB_TYPE_TINYINT>);
  ASSERT_FALSE(is_float_type<DB_TYPE_SMALLINT>);
  ASSERT_FALSE(is_float_type<DB_TYPE_INT>);
  ASSERT_FALSE(is_float_type<DB_TYPE_BIGINT>);
  ASSERT_FALSE(is_float_type<DB_TYPE_LARGEINT>);
  ASSERT_TRUE(is_float_type<DB_TYPE_FLOAT>);
  ASSERT_TRUE(is_float_type<DB_TYPE_DOUBLE>);
  ASSERT_FALSE(is_float_type<DB_TYPE_DECIMAL32>);
  ASSERT_FALSE(is_float_type<DB_TYPE_DECIMAL64>);
  ASSERT_FALSE(is_float_type<DB_TYPE_DECIMAL128>);
  ASSERT_FALSE(is_float_type<DB_TYPE_CHAR>);
  ASSERT_FALSE(is_float_type<DB_TYPE_VARCHAR>);

  ASSERT_FALSE(is_integer_type<DB_TYPE_BOOLEAN>);
  ASSERT_TRUE(is_integer_type<DB_TYPE_TINYINT>);
  ASSERT_TRUE(is_integer_type<DB_TYPE_SMALLINT>);
  ASSERT_TRUE(is_integer_type<DB_TYPE_INT>);
  ASSERT_TRUE(is_integer_type<DB_TYPE_BIGINT>);
  ASSERT_TRUE(is_integer_type<DB_TYPE_LARGEINT>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_FLOAT>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_DOUBLE>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_DECIMAL32>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_DECIMAL64>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_DECIMAL128>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_CHAR>);
  ASSERT_FALSE(is_integer_type<DB_TYPE_VARCHAR>);
}

template <DB_TYPE Type, typename = guard::Guard> struct ReturnStringV {
  static inline std::string get() { return "unknown"; }
};

template <DB_TYPE Type> struct ReturnStringV<Type, FloatTypeGuard<Type>> {
  static inline std::string get() { return "float"; }
};

template <DB_TYPE Type> struct ReturnStringV<Type, IntegerTypeGuard<Type>> {
  static inline std::string get() { return "integer"; }
};

template <DB_TYPE Type> struct ReturnStringV<Type, BooleanTypeGuard<Type>> {
  static inline std::string get() { return "bool"; }
};

template <DB_TYPE Type> struct ReturnStringV<Type, DecimalTypeGuard<Type>> {
  static inline std::string get() { return "decimal"; }
};

template <DB_TYPE Type> struct ReturnStringV<Type, CharTypeGuard<Type>> {
  static inline std::string get() { return "char"; }
};

TEST_F(TestGuard, testValueGuard) {
  ASSERT_EQ(ReturnStringV<DB_TYPE_BOOLEAN>::get(), "bool");
  ASSERT_EQ(ReturnStringV<DB_TYPE_TINYINT>::get(), "integer");
  ASSERT_EQ(ReturnStringV<DB_TYPE_SMALLINT>::get(), "integer");
  ASSERT_EQ(ReturnStringV<DB_TYPE_INT>::get(), "integer");
  ASSERT_EQ(ReturnStringV<DB_TYPE_BIGINT>::get(), "integer");
  ASSERT_EQ(ReturnStringV<DB_TYPE_LARGEINT>::get(), "integer");
  ASSERT_EQ(ReturnStringV<DB_TYPE_FLOAT>::get(), "float");
  ASSERT_EQ(ReturnStringV<DB_TYPE_DOUBLE>::get(), "float");
  ASSERT_EQ(ReturnStringV<DB_TYPE_DECIMAL32>::get(), "decimal");
  ASSERT_EQ(ReturnStringV<DB_TYPE_DECIMAL64>::get(), "decimal");
  ASSERT_EQ(ReturnStringV<DB_TYPE_DECIMAL128>::get(), "decimal");
  ASSERT_EQ(ReturnStringV<DB_TYPE_CHAR>::get(), "char");
  ASSERT_EQ(ReturnStringV<DB_TYPE_VARCHAR>::get(), "char");
  ASSERT_EQ(ReturnStringV<DB_TYPE_INVALID>::get(), "unknown");
}

struct Foobar01 {
  using ReturnType = int;
  static inline ReturnType get_int() { return 0xdeadbeef; }
};

struct Foobar02 {
  using ReturnType = std::string;
  static inline ReturnType get_string() { return "foobar"; }
};

TYPE_GUARD(Foobar01Guard, is_foobar01_class, Foobar01);
TYPE_GUARD(Foobar02Guard, is_foobar02_class, Foobar02);
TYPE_GUARD(FoobarGuard, is_foobar_class, Foobar01, Foobar02);

template <typename T> typename T::ReturnType call_foobar() {
  if constexpr (is_foobar01_class<T>) {
    return T::get_int();
  } else if constexpr (is_foobar02_class<T>) {
    return T::get_string();
  } else {
    static_assert(is_foobar_class<T>, "Invalid foobar class");
  }
}

TEST_F(TestGuard, testCallFoobar) {
  auto a = call_foobar<Foobar01>();
  std::cout << "a=" << a << std::endl;
  auto b = call_foobar<Foobar02>();
  std::cout << "b=" << b << std::endl;
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
