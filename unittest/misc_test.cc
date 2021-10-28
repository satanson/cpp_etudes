// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/9/23.
//

#include <gtest/gtest.h>
#include <immintrin.h>
#include <math.h>

#include <atomic>
#include <cstdlib>
#include <guard.hh>
#include <iostream>
#include <list>
#include <memory>
#include <random>

using namespace std;
class MiscTest : public ::testing::Test {};
namespace abc {
class MiscTest {};
} // namespace abc
TEST_F(MiscTest, testTypeId) {
    cout << typeid(int).name() << endl;
    cout << typeid(0).name() << endl;
    cout << typeid(double).name() << endl;
    cout << typeid(0.1f).name() << endl;
    cout << typeid(0.1).name() << endl;
    cout << typeid(MiscTest).name() << endl;
    cout << typeid(abc::MiscTest).name() << endl;
    cout << typeid(int*).name() << endl;
    cout << typeid(int**).name() << endl;
    cout << typeid(int***).name() << endl;
    cout << typeid(const int*).name() << endl;
    cout << typeid(volatile int**).name() << endl;
    cout << typeid(const volatile int***).name() << endl;
    cout << typeid(int&).name() << endl;
    cout << sizeof(long long) << endl;
}

struct Str {
    char* p;
    size_t n;
    explicit Str(string const& s) {
        cout << "Str::ctor invoked" << endl;
        this->n = s.size();
        this->p = new char[n];
        std::copy(s.begin(), s.end(), p);
    }

    std::string ToString() {
        std::string s(n, '0');
        std::copy(p, p + n, s.begin());
        return s;
    }

    Str(Str&& that) {
        std::cout << "move ctor" << endl;
        this->n = that.n;
        this->p = that.p;
        that.p = nullptr;
        that.n = 0;
    }

    ~Str() {
        cout << "Str::dtor invoked" << endl;
        if (p != nullptr) {
            delete p;
            p = nullptr;
        }
        n = 0;
    }
};

Str returnStr(std::string const& a) {
    return Str(a);
}

Str&& returnStrRvalueRef(Str& a) {
    return static_cast<Str&&>(a);
}

struct A {
    int a;
    int b;
};

template <typename... Args>
void print0(Args&&... args) {
    (std::cout << ... << std::forward<Args>(args)) << std::endl;
}

TEST_F(MiscTest, testRValueReference) {
    print0(1, 2, 4, 5, "abc");
}

TEST_F(MiscTest, floatAdd) {
    float a = 0.3f;
    float b = 0;
    for (int i = 0; i < 1000'0000; ++i) {
        b += a;
    }
    std::cout << b << std::endl;
}

TEST_F(MiscTest, tuple) {}

template <bool abc, typename F, typename... Args>
int foobar(int a, F f, Args&&... args) {
    if constexpr (abc) {
        return a * f(std::forward<Args>(args)...);
    } else {
        return a + f(std::forward<Args>(args)...);
    }
}

template <bool is_abc>
struct AA {
    ;
    static void evaluate() {
        if constexpr (is_abc) {
            std::cout << "is_abc" << std::endl;
        } else {
            std::cout << "!is_abc" << std::endl;
        }
    }
};

template <template <bool> typename F>
void g(bool abc) {
    if (abc) {
        F<true>::evalute();
    } else {
        F<false>::evalute();
    }
}

template <template <typename, size_t...> typename Collector, size_t... Args>
std::shared_ptr<void> create_abc() {
    using Array = Collector<int, Args...>;
    return (std::shared_ptr<void>)std::make_shared<Array>();
}

TEST_F(MiscTest, foobar) {
    std::shared_ptr<void> a = create_abc<std::array, 10>();
}

template <typename T, typename = guard::Guard>
struct AAA {
    static inline void apply() { std::cout << "AAA T" << std::endl; }
};
template <typename T>
struct AAA<T, guard::TypeGuard<T, float, double>> {
    static inline void apply() { std::cout << "AAA float or double" << std::endl; }
};
template <typename T>
struct AAA<T, guard::TypeGuard<T, char, short, long, long long>> {
    static inline void apply() { std::cout << "AAA other int" << std::endl; }
};
template <>
struct AAA<int, int> {
    static inline void apply() { std::cout << "AAA int" << std::endl; }
};

TEST_F(MiscTest, testConcept) {
    AAA<char>::apply();
    AAA<float>::apply();
    AAA<double>::apply();
    AAA<int>::apply();
    AAA<long>::apply();
    AAA<bool>::apply();
    AAA<unsigned int>::apply();
    AAA<unsigned long>::apply();
}
using FieldType = int32_t;
union ExtendFieldType {
    FieldType field_type;
    struct {
#if __BYTE_ORDER == LITTLE_ENDIAN
        int16_t type;
        int8_t precision;
        int8_t scale;
#else
        int8_t scale;
        int8_t precision;
        int16_t type;
#endif
    } extend;

    ExtendFieldType(FieldType field_type) : field_type(field_type) {}
    ExtendFieldType(FieldType field_type, int precision, int scale)
            : extend({.type = (int16_t)field_type, .precision = (int8_t)precision, .scale = (int8_t)scale}) {}
    ExtendFieldType(ExtendFieldType const&) = default;
    ExtendFieldType& operator=(ExtendFieldType&) = default;
    FieldType type() const { return extend.type; }
    int precision() const { return extend.precision; }
    int scale() const { return extend.scale; }
};

TEST_F(MiscTest, testExtendField) {
    auto field = ExtendFieldType(10, 27, 9);
    ASSERT_EQ(field.type(), 10);
    ASSERT_EQ(field.precision(), 27);
    ASSERT_EQ(field.scale(), 9);
}

template <typename Op>
struct FunctorA {
    template <typename T, typename... Args>
    static inline T evaluate(const T& t, Args&&... args) {
        return Op::template evaluate(t, 'A', std::forward<Args>(args)...);
    }
};

template <typename Op>
struct FunctorB {
    template <typename T, typename... Args>
    static inline T evaluate(const T& t, Args&&... args) {
        return Op::evaluate(t, 'B', std::forward<Args>(args)...);
    }
};

struct FunctorC {
    template <typename... Args>
    static inline std::string evaluate(std::string const& s, Args&&... args) {
        std::string result(s);
        ((result.append(std::to_string(std::forward<Args>(args)))), ...);
        return result;
    }
};

TEST_F(MiscTest, testEvaluate) {
    auto s = FunctorA<FunctorB<FunctorC>>::evaluate<std::string>("abc", 1, 2);
    std::cout << s << std::endl;
    const int arg0 = 100;
    const int arg1 = 999;
    const int& arg0_ref = arg0;
    const int& arg1_ref = arg1;
    auto s1 = FunctorA<FunctorB<FunctorC>>::evaluate<std::string>("abc", arg0, arg1);
    std::cout << s1 << std::endl;
    auto s2 = FunctorA<FunctorB<FunctorC>>::evaluate<std::string>("abc", arg0_ref, arg1_ref);
    std::cout << s2 << std::endl;
}

TEST_F(MiscTest, testMod) {
    int32_t a = -9;
    int32_t b = -7;
    std::cout << a % b << endl;
}
TEST_F(MiscTest, testTuple) {
    std::tuple<std::string, std::string> t0 = std::make_tuple(std::string(4096, 'x'), std::string(4096, 'y'));
    std::tuple<std::string, std::string> t1 = std::make_tuple(std::get<1>(t0), std::get<0>(t0));
    std::cout << "t0.0=" << std::get<0>(t0) << "t0.1=" << std::get<1>(t0) << std::endl;
    std::cout << "t1.0=" << std::get<0>(t1) << "t1.1=" << std::get<1>(t1) << std::endl;
}
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

TYPE_GUARD(Int8Int16Guard, is_int8_int16, int8_t, int16_t)
TYPE_GUARD(Int32Guard, is_int32, int32_t)
TYPE_GUARD(Int64Guard, is_int64, int64_t)
TYPE_GUARD(Int128Guard, is_int128, int128_t);
TYPE_GUARD(IntGuard, is_int, int8_t, int16_t, int32_t, int64_t, int128_t)

template <typename T>
T mod(T a, T b) {
    using TT = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (is_int8_int16<TT>) {
        return TT(fmodf(a, b));
    } else if constexpr (is_int32<TT>) {
        return TT(fmod(a, b));
    } else if constexpr (is_int64<TT>) {
        return TT(fmodl(a, b));
    } else if constexpr (is_int128<TT>) {
        return a % b;
    } else {
        static_assert(is_int<TT>, "invalid type");
    }
}

template <typename T>
T min_mod_neg_one() {
    T a = T(1) << ((sizeof(T) * 8) - 1);
    T b = T(-1);
    return mod<T>(a, b);
}

template <typename T>
T min_div_neg_one() {
    T a = T(1) << ((sizeof(T) * 8) - 1);
    T b = T(-1);
    return a / b;
}

void print_int128_t(int128_t v) {
    uint128_t a = v;
    int64_t all_one = -1;
    uint64_t u_all_one = all_one;
    uint128_t lowbits_mask = u_all_one;
    std::cout << "high 64bits=" << uint64_t((a >> 64) & lowbits_mask);
    std::cout << ", low 64bits=" << uint64_t(a & lowbits_mask) << std::endl;
}

TEST_F(MiscTest, testFmod) {
    std::cout << sizeof(long double) << std::endl;
}

TEST_F(MiscTest, testModOperationWithNegativeDivisor) {
    std::cout << (int)min_mod_neg_one<int8_t>() << std::endl;
    std::cout << min_mod_neg_one<int16_t>() << std::endl;
    std::cout << min_mod_neg_one<int32_t>() << std::endl; // SIGFPE
    std::cout << min_mod_neg_one<int64_t>() << std::endl; // SIGFPE
    print_int128_t(min_mod_neg_one<int128_t>());

    std::cout << (int)min_div_neg_one<int8_t>() << std::endl;
    std::cout << min_div_neg_one<int16_t>() << std::endl;
    std::cout << min_div_neg_one<int32_t>() << std::endl; // SIGFPE
    std::cout << min_div_neg_one<int64_t>() << std::endl; // SIGFPE
    print_int128_t(min_mod_neg_one<int128_t>());
    int128_t max_int128 = int128_t(1) << ((sizeof(int128_t) * 8) - 1);
    int128_t neg_one_int128 = int128_t(-1);
    print_int128_t(max_int128);
    print_int128_t(neg_one_int128);
}
TEST_F(MiscTest, testLimit) {
    print_int128_t(std::numeric_limits<int128_t>::min());
}

TEST_F(MiscTest, testAlignOfInt128) {
    ASSERT_EQ(alignof(int128_t), 16);
}

TEST_F(MiscTest, testFloatOverflow) {
    float a = 2.85228E7;
    int32_t b = 10000;
    int32_t c = a * b;
    std::cout << c << std::endl;

    float a0 = 2.85228E15;
    int64_t b0 = 1000000L;
    int64_t c0 = a0 * b0;
    std::cout << c0 << std::endl;

    float a1 = 2.85228E35;
    int128_t b1 = 10000L;
    int128_t c1 = a1 * b1;
    print_int128_t(c1);
}

TEST_F(MiscTest, testFloat2Decimal128) {
    float f = -0.88404423f;
    int128_t d = int128_t(1);
    int128_t a = static_cast<int128_t>(static_cast<double>(f) * d);
    print_int128_t(a);
}

template <typename T, int n>
struct EXP10 {
    using type = T;
    static constexpr type value = EXP10<T, n - 1>::value * static_cast<type>(10);
};

template <typename T>
struct EXP10<T, 0> {
    using type = T;
    static constexpr type value = static_cast<type>(1);
};

inline constexpr int128_t exp10_int128(int n) {
    constexpr int128_t values[] = {
            EXP10<int128_t, 0>::value,  EXP10<int128_t, 1>::value,  EXP10<int128_t, 2>::value,
            EXP10<int128_t, 3>::value,  EXP10<int128_t, 4>::value,  EXP10<int128_t, 5>::value,
            EXP10<int128_t, 6>::value,  EXP10<int128_t, 7>::value,  EXP10<int128_t, 8>::value,
            EXP10<int128_t, 9>::value,  EXP10<int128_t, 10>::value, EXP10<int128_t, 11>::value,
            EXP10<int128_t, 12>::value, EXP10<int128_t, 13>::value, EXP10<int128_t, 14>::value,
            EXP10<int128_t, 15>::value, EXP10<int128_t, 16>::value, EXP10<int128_t, 17>::value,
            EXP10<int128_t, 18>::value, EXP10<int128_t, 19>::value, EXP10<int128_t, 20>::value,
            EXP10<int128_t, 21>::value, EXP10<int128_t, 22>::value, EXP10<int128_t, 23>::value,
            EXP10<int128_t, 24>::value, EXP10<int128_t, 25>::value, EXP10<int128_t, 26>::value,
            EXP10<int128_t, 27>::value, EXP10<int128_t, 28>::value, EXP10<int128_t, 29>::value,
            EXP10<int128_t, 30>::value, EXP10<int128_t, 31>::value, EXP10<int128_t, 32>::value,
            EXP10<int128_t, 33>::value, EXP10<int128_t, 34>::value, EXP10<int128_t, 35>::value,
            EXP10<int128_t, 36>::value, EXP10<int128_t, 37>::value, EXP10<int128_t, 38>::value,
    };
    return values[n];
}
inline constexpr int128_t get_scale_factor(int n) {
    return exp10_int128(n);
}
TEST_F(MiscTest, testFloat2Decimal128Overflow) {
    float f = 1.72587728E8;
    int128_t a = get_scale_factor(30);
    int128_t b = static_cast<int128_t>(a * static_cast<double>(f));
    bool overflow = abs(f) >= double(1) && b == int128_t(0);
    std::cout << overflow << std::endl;
    print_int128_t(a);
    print_int128_t(b);
    int64_t h = 9355999482096867328L;
    int128_t h128 = h;
    h128 <<= 64;
    std::cout << "<0:" << (h128 < 0) << ",   >0:" << (h128 > 0) << std::endl;
}
template <typename T>
void func000() {
    if constexpr (is_int128<T>) {
        ASSERT_TRUE("abc" == "abc");
    } else {
        ASSERT_TRUE("abc" == "abc");
    }
}
void func0002() {
    bool is_const = true;
    switch (0)
    case 0:
    default:
        if (const ::testing::AssertionResult gtest_ar_ = ::testing::AssertionResult((is_const)))
            ;
        else
            return ::testing::internal::AssertHelper(
                           ::testing::TestPartResult::kFatalFailure,
                           "/home/disk2/rpf/DorisDB_rpf/be/test/exprs/vectorized/"
                           "decimal_cast_expr_test_helper.h",
                           147,
                           ::testing::internal::GetBoolAssertionFailureMessage(gtest_ar_, "is_const", "false", "true")
                                   .c_str()) = ::testing::Message();
}
TEST_F(MiscTest, testFloat1) {
    float f = -0.88404423F;
    int128_t a = get_scale_factor(0);
    int128_t b = static_cast<int128_t>(a * static_cast<double>(f));
    bool overflow = abs(f) >= double(1) && b == int128_t(0);
    print_int128_t(b);
    std::cout << overflow << std::endl;
    func000<int128_t>();
    func000<int32_t>();
    func0002();
}

template <bool flag>
class ConstructorParamsWithMaybeUnusedModifier {
    int c;

public:
    ConstructorParamsWithMaybeUnusedModifier([[maybe_unused]] int a, [[maybe_unused]] int b) {
        if constexpr (flag) {
            c = a + b;
        } else {
            c = 10;
        }
    }
    int get_c() { return c; }
};

TEST_F(MiscTest, constructorParamsWithMaybeUnusedModifier) {
    ConstructorParamsWithMaybeUnusedModifier<true> trueObj(1, 1);
    ConstructorParamsWithMaybeUnusedModifier<false> falseObj(1, 2);
    std::cout << trueObj.get_c() << std::endl;
    std::cout << falseObj.get_c() << std::endl;
}

void func_tmpl(string const& a) {
    std::cout << "string-typed func: " << a << std::endl;
}
TYPE_GUARD(FloatGuard, type_is_float, float, double);
template <typename T>
void func_tmpl(FloatGuard<T> a) {
    std::cout << "float-typed func: " << a << std::endl;
}

TYPE_GUARD(IntegerGuard, type_is_integer, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t)
template <typename T>
void func_tmpl(IntegerGuard<T> a) {
    std::cout << "integer-typed func: " << a << std::endl;
}

TEST_F(MiscTest, test_function_template_partial_specialization) {
    func_tmpl<int>(100);
    func_tmpl<float>(0.2f);
    func_tmpl<double>(0.2);
    func_tmpl(std::string("abc"));
}

struct Foobar001 {
    Foobar001() { std::cout << "invoke ctor" << std::endl; }
    ~Foobar001() { std::cout << "invoke ~ctor" << std::endl; }
    Foobar001(const Foobar001& other) { std::cout << "invoke cpy ctor" << std::endl; }
    Foobar001& operator=(const Foobar001& other) { std::cout << "invoke assign" << std::endl; }
};
using Foobar001Ptr = std::unique_ptr<Foobar001>;
using Foobar001Vector = std::vector<std::tuple<size_t, Foobar001Ptr>>;
void put_foobar001(Foobar001Vector& fv, Foobar001Ptr fb) {
    fv.emplace_back(fv.size(), std::move(fb));
}
TEST_F(MiscTest, test_unqiue_ptr) {
    Foobar001Vector fv;
    fv.reserve(3);
    { put_foobar001(fv, std::unique_ptr<Foobar001>(new Foobar001())); }
    std::cout << "here" << std::endl;
    // put_foobar001(fv, std::unique_ptr<Foobar001>(new Foobar001()));
    // put_foobar001(fv, std::unique_ptr<Foobar001>(new Foobar001()));
    // put_foobar001(fv, std::unique_ptr<Foobar001>(new Foobar001()));
}

TEST_F(MiscTest, test_int_min_compare) {
    std::cout << (-2147483648 < 0) << std::endl;
    std::cout << (-2147483648 > 0) << std::endl;
    std::cout << (0 < -2147483648) << std::endl;
    std::cout << (0 > -2147483648) << std::endl;

    std::cout << (2147483647 < 1) << std::endl;
    std::cout << (2147483647 > 1) << std::endl;
    std::cout << (1 < 2147483647) << std::endl;
    std::cout << (1 > 2147483647) << std::endl;
}

TEST_F(MiscTest, test_decimal) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> length_rand(1, 10000);
    std::uniform_int_distribution<int> percent_rand(0, 100);
    for (int i = 0; i < 100; ++i) {
        std::cout << percent_rand(gen) << std::endl;
    }
}

template <typename T>
T getenv_or_default(const char* name, const T& default_value) {
    const auto* p = getenv(name);
    if (p == nullptr) {
        return default_value;
    }
    std::stringstream ss;
    ss.str(p);
    T value;
    ss >> value;
    return value;
}

TEST_F(MiscTest, getenv_or_default) {
    setenv("foobar_env", "123", 1);
    auto a = getenv_or_default<int>("foobar_env", 1000);
    std::cout << a << std::endl;
}

TEST_F(MiscTest, test_list) {
    std::list<int> a{1, 2, 3, 4, 5, 6};
    std::list<int> b{7, 8, 9, 10, 11, 12};
    a.splice(a.end(), b);
    ASSERT_TRUE(b.empty());
    for (auto it = a.begin(); it != a.end();) {
        auto e = *it;
        if (e % 2 == 0 || e % 3 == 0) {
            a.erase(it++);
        } else {
            ++it;
        }
    }
    ASSERT_TRUE(!a.empty());
    for (auto e : a) {
        ASSERT_TRUE(e % 2 != 0 && e % 3 != 0);
        std::cout << e << std::endl;
    }
}

struct A0000 {
    A0000(std::string const& s) : name(s) { std::cout << "construct A0000(" << name << ")" << std::endl; }
    bool pred() { return name.find("ab", 0) != std::string::npos; }
    ~A0000() { std::cout << "destruct A0000(" << name << ")" << std::endl; }

private:
    std::string name;
};
struct A000 : A0000 {
    A000(std::string const& a, std::string const& b, std::string const& c) : A0000(a), a0(b), a1(c) {
        std::cout << "construct A000" << std::endl;
    }
    ~A000() { std::cout << "destruct A000" << std::endl; }

private:
    A0000 a0;
    A0000 a1;
};

TEST_F(MiscTest, testCtorDtor) {
    A000 a("base", "field1", "field2");
    int b = 0;
    std::frexp(0.2, &b);
}
#include <chrono>
#include <queue>
using Driver = A0000;
using DriverPtr = std::shared_ptr<A0000>;
using DriverQueue = std::queue<DriverPtr>;
using DriverList = std::list<DriverPtr>;
void put_driver(DriverQueue& queue, const DriverPtr& driver) {
    queue.emplace(driver);
}

TEST_F(MiscTest, testQueue) {
    DriverList driver_list;
    DriverQueue driver_queue;
    driver_list.push_back(std::make_shared<Driver>("a"));
    driver_list.push_back(std::make_shared<Driver>("b"));
    driver_list.push_back(std::make_shared<Driver>("c"));
    driver_list.push_back(std::make_shared<Driver>("d"));
    driver_list.push_back(std::make_shared<Driver>("e"));
    auto driver_it = driver_list.begin();
    while (driver_it != driver_list.end()) {
        put_driver(driver_queue, *driver_it);
        driver_list.erase(driver_it++);
    }
}

TEST_F(MiscTest, testNoneOf) {
    std::vector<A0000> as{A0000("abc"), A0000("abcd"), A0000("ab")};
    std::vector<A0000> as2;
    ASSERT_TRUE(std::all_of(as.begin(), as.end(), [](auto& a) { return a.pred(); }));
    ASSERT_TRUE(std::any_of(as.begin(), as.end(), [](auto& a) { return a.pred(); }));
    ASSERT_FALSE(std::none_of(as.begin(), as.end(), [](auto& a) { return a.pred(); }));
    ASSERT_TRUE(std::all_of(as2.begin(), as2.end(), [](auto& a) { return a.pred(); }));
}

TEST_F(MiscTest, testSeconds) {
    using std::chrono::seconds;
    using std::chrono::milliseconds;
    using std::chrono::steady_clock;
    using std::chrono::duration_cast;
    auto dueTime = seconds(300);
    auto now = steady_clock::now().time_since_epoch();
    auto deadline = duration_cast<milliseconds>(now + dueTime).count();
    std::cout<<(deadline-duration_cast<milliseconds>(now).count()) <<std::endl;
    std::cout <<"now="<<now.count() <<std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}