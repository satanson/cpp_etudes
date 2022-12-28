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
#include <fstream>
#include <functional>
#include <guard.hh>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
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
    int64_t h = (int64_t)9355999482096867328L;
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
    Foobar001& operator=(const Foobar001& other) {
        std::cout << "invoke assign" << std::endl;
        return *this;
    }
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
    std::cout << (deadline - duration_cast<milliseconds>(now).count()) << std::endl;
    std::cout << "now=" << now.count() << std::endl;
}

TEST_F(MiscTest, testSetEqualRange) {
    std::set<int> a;
    for (int i = 0; i < 10; ++i) {
        a.insert(i * 2);
    }
    for (auto it = a.begin(); it != a.end(); ++it) {
        std::cout << *it << ", ";
    }
    std::cout << "\n";

    for (int i = 0; i < 10; ++i) {
        int v = i * 3 - 4;
        std::cout << "seek v=" << v << " :";
        auto bounds = a.equal_range(v);
        bounds.second++;
        for (auto it = bounds.first; it != bounds.second; ++it) {
            std::cout << *it << ", ";
        }
        std::cout << "\n";
    }
}

struct NoExceptClass {
    NoExceptClass() = default;
    ~NoExceptClass() = default;
    void do_except_work() noexcept {
        vector<int> a;
        auto& b = a.back();
        std::cout << b << "can not reach here\n";
    }
    void do_except_work2() {
        vector<int> a;
        auto& b = a.back();
        std::cout << b << "can not reach here\n";
    }
};
TEST_F(MiscTest, testNoExceptClass) {
    NoExceptClass no_except_class;
    // no_except_class.do_except_work();
    // no_except_class.do_except_work2();
}
TEST_F(MiscTest, testStringPackedInteger) {
    int64_t a = 0xdeadbeef;
    std::string s;
    s.resize(sizeof(a));
    memcpy(s.data(), &a, sizeof(a));
    std::reverse(s.begin(), s.end());
    int64_t b = 0;
    int64_t c = 0;
    std::copy(s.begin(), s.end(), (char*)&b);
    std::copy(s.rbegin(), s.rend(), (char*)&c);
    std::cout << std::hex << "b=" << b << ", c=" << c << std::endl;
}
std::string return_string(const string& s) {
    return std::string("abc") + s;
}
void print_c_str(const char* s) {
    std::string s0 = s;
    std::cout << s0 << std::endl;
}
TEST_F(MiscTest, testStringCstr) {
    for (int i = 0; i < 100; ++i) {
        print_c_str(return_string("_124").c_str());
    }
}
TEST_F(MiscTest, testConstStringAssignment) {
    std::string s = "abcdefg";
    std::map<std::string, std::string> str2str;
    const std::string& s_value = s;
    const std::string& s_key = "key";
    str2str.insert(std::make_pair(s_key, s_value));
    str2str.clear();
    const string s1 = rand() > 10 ? s : "";
}
template <typename T>
std::shared_ptr<T> invoke_func(std::function<std::shared_ptr<T>(void)> create_func) {
    return create_func();
}
TEST_F(MiscTest, testInvokeFunc) {
    int a = 100;
    std::string s = "abc";
    auto s2 = invoke_func<std::string>([a, s]() mutable {
        s.append(std::to_string(a));
        return std::shared_ptr<std::string>(new std::string(s));
    });
    std::cout << *s2 << std::endl;
}

TEST_F(MiscTest, testTransactionId) {
    uint64_t t = 0x00ff'0000'ff00'0000llu;
    uint32_t x = t >> 32;
    uint32_t y = t & ~0llu >> 32;
    uint32_t z = t & (~1llu >> 32);
    uint32_t w = (t & ~1llu) >> 32;
    std::cout << x << "," << y << "," << z << "," << w << std::endl;
}
template <typename T>
void print_vec(std::vector<T> const& vec) {
    std::cout << "cap=" << vec.capacity() << ", size=" << vec.size() << ", data=[";
    for (auto& e : vec) {
        std::cout << e << ", ";
    }
    std::cout << "]" << std::endl;
}

TEST_F(MiscTest, testAssign) {
    vector<int> v;
    v.assign(10, 1);
    print_vec(v);
    v.assign(2, 2);
    print_vec(v);
    v.assign(11, 3);
    print_vec(v);
    v.assign(10, 1);
    size_t count = 0;
    for (int i = 0; i < 10; ++i) {
        v[i] &= (i % 2 == 0);
        count += (i % 2 == 0);
    }
    print_vec(v);
    std::cout << count << std::endl;
    std::shared_ptr<int> a;
    a = a;
    std::shared_ptr<int> b(a);
}
class B0 {
public:
    B0(int i) : i(i) {
        std::cout << "ctor B0"
                  << "(" << i << ")" << std::endl;
    }
    ~B0() {
        std::cout << "dtor B0"
                  << "(" << i << ")" << std::endl;
    }

private:
    int i;
};

class B1 {
public:
    B1() {
        b1 = std::make_shared<B0>(1);
        b2 = std::make_shared<B0>(2);
        b3 = std::make_shared<B0>(3);
        std::cout << "ctor B1" << std::endl;
    }
    ~B1() {
        std::cout << "dtor B1" << std::endl;
        b2.reset();
    }

private:
    std::shared_ptr<B0> b1;
    std::shared_ptr<B0> b2;
    std::shared_ptr<B0> b3;
};
TEST_F(MiscTest, TestB1) {
    B1 b1;
}
TEST_F(MiscTest, TestUniquePtr) {
    std::vector<std::unique_ptr<B1>> bs;
    //bs.resize(10);
    for (auto i = 0; i < 10; ++i) {
        bs.push_back(std::make_unique<B1>());
    }
}

TEST_F(MiscTest, TestIteratorBackwardVector) {
    std::vector<int> vec{1, 2, 3, 4, 5, 6};
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        std::cout << *it << std::endl;
    }
    std::function<void()> f;
    f();
}

static inline std::string base_name_of_conjugate_op(const std::string& s) {
    std::string lc;
    lc.resize(s.size());
    std::transform(s.begin(), s.end(), lc.begin(), [](char c) { return (char)std::tolower(c); });
    const char* source = "_source";
    const char* sink = "_sink";
    if (memcmp(lc.data() + lc.size() - 7, source, 7) == 0) {
        lc.resize(lc.size() - 7);
    } else if (memcmp(lc.data() + lc.size() - 5, sink, 5) == 0) {
        lc.resize(lc.size() - 5);
    }
    return lc;
}

TEST_F(MiscTest, testBaseNameOfConjugateName) {
    std::cout << base_name_of_conjugate_op("abc_sink") << std::endl;
    std::cout << base_name_of_conjugate_op("abcd_source") << std::endl;
    std::cout << base_name_of_conjugate_op("Abc_sInk") << std::endl;
    std::cout << base_name_of_conjugate_op("abCD_sOURce") << std::endl;
}

struct C1 {
    std::vector<int> f0;
    std::unordered_map<int, int> f1;
};

TEST_F(MiscTest, testUniquePtrAssignment) {
    std::unique_ptr<C1> c1 = std::make_unique<C1>();
    c1->f0.push_back(1);
    c1->f0.push_back(2);
    c1->f0.push_back(3);
    c1->f1[1] = 11;
    c1->f1[2] = 22;
    c1->f1[3] = 33;
    c1 = std::make_unique<C1>();
    std::cout << c1->f0.size() << std::endl;
    std::cout << c1->f1.size() << std::endl;
}
#include <any>
TEST_F(MiscTest, testAny) {
    std::any a = 1;
    std::cout << a.has_value() << std::endl;
    a.reset();
    std::cout << a.has_value() << std::endl;
}

void invoke_f(const std::string& s, void (*f)(const std::string&)) {
    f(s);
}
TEST_F(MiscTest, testFunction) {
    int a = 100;
    invoke_f("foobar", [](const std::string& s) {
        std::string r;
        r.append(s);
        r.append(std::to_string(100));
        std::cout << r << std::endl;
    });
}

template <typename, template <typename...> typename, typename...>
struct detector {
    using value_t = std::false_type;
};

template <template <typename...> typename Op, typename... Args>
struct detector<std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
};

template <template <typename...> typename Op, typename... Args>
using is_detect = typename detector<void, Op, Args...>::value_t;

template <typename T, typename TRef = T&&>
TRef __mydeclval(int);
template <typename T>
T __mydeclval(long);

template <typename...>
struct mydeclval_preventor {
    static constexpr bool stop = false;
};
template <typename T>
auto mydeclval() -> decltype(__mydeclval<T>(0)) {
    static_assert(mydeclval_preventor<T>::stop, "abc");
    return __mydeclval<T>(0);
}

namespace test_polymorphic_dispatch {
class Fish {
public:
    void swim() { std::cout << "Fish can swim" << std::endl; }
};
class Bird {
public:
    void fly() { std::cout << "Bird can fly" << std::endl; }
};
template <typename T>
class Animal {
public:
    template <typename S>
    using can_fly_t = decltype(mydeclval<S>().fly());
    template <typename S>
    using can_swim_t = decltype(mydeclval<S>().swim());
    static constexpr bool can_fly = is_detect<can_fly_t, T>::value;
    static constexpr bool can_swim = is_detect<can_swim_t, T>::value;
    void behavior() {
        if constexpr (can_fly) {
            t.fly();
        } else if constexpr (can_swim) {
            t.swim();
        }
    }

private:
    T t;
};

} // namespace test_polymorphic_dispatch

TEST_F(MiscTest, testStaticDispatch) {
    namespace tpd = test_polymorphic_dispatch;
    tpd::Animal<tpd::Fish> animal0;
    tpd::Animal<tpd::Bird> animal1;
    animal0.behavior();
    animal1.behavior();
}

template <typename DesiredTypeName>
inline std::string getTypeName() {
    std::string Name = __PRETTY_FUNCTION__;

    std::string Key = "DesiredTypeName = ";
    Name = Name.substr(Name.find(Key));
    assert(!Name.empty() && "Unable to find the template parameter!");
    Name = Name.substr(Key.size());

    assert(Name.back() == ']' && "Name doesn't end in the substitution key!");
    return Name.substr(0, Name.size() - 1);
}
TEST_F(MiscTest, testGetTypeName) {
    namespace tpd = test_polymorphic_dispatch;
    using T = tpd::Animal<tpd::Fish>;
    std::cout << getTypeName<T>() << std::endl;
}
using MapFunc = std::function<double(double)>;
using ReduceFunc = std::function<double(double, double)>;
static ReduceFunc add_func = [](double a, double b) { return a + b; };
static MapFunc pi_map1_func = [](double a) {
    long l = (long)a;
    long s = -(l & 1L);
    return (double)((((l << 1) + 1L) ^ s) - s);
};
static MapFunc pi_map2_func = [](double a) { return 4.0 / a; };

TEST_F(MiscTest, testPi) {
    std::vector<double> data;
    data.resize(1000);
    for (auto i = 0; i < data.size(); ++i) {
        data[i] = (double)i;
    }
    for (auto i = 0; i < data.size(); ++i) {
        data[i] = pi_map1_func(data[i]);
    }
    for (auto i = 0; i < data.size(); ++i) {
        data[i] = pi_map2_func(data[i]);
    }
    double result = 0;
    for (auto i = 0; i < data.size(); ++i) {
        result = add_func(result, data[i]);
    }
    std::cout << result << std::endl;
}

template <typename _Tp, typename = void>
struct my_is_referenceable : public false_type {};

template <typename _Tp>
struct my_is_referenceable<_Tp, __void_t<_Tp&>> : public true_type {};

void void_func() {}
int int_func() {
    return 0;
}
TEST_F(MiscTest, testMeta) {
    static_assert(my_is_referenceable<int>::value, "abc");
    using a = my_is_referenceable<std::function<void(void)>>;
    static_assert(a::value, "abc");
    using b = my_is_referenceable<void>;
    //static_assert(b::value, "abc");
    using c = my_is_referenceable<decltype(int_func())>;
    static_assert(c::value, "abc");
}

struct ReferenceMethod {
    void print() & { std::cout << "invoked"; }
};

TEST_F(MiscTest, testReferenceMethod) {
    ReferenceMethod a;
    a.print();
    //ReferenceMethod().print();
}

template <typename... Args>
struct SizeOfVariadicTemplate {
    static constexpr int n = sizeof...(Args);
};

TEST_F(MiscTest, testAlign) {
    auto check = [](int64_t n) { return (n & ~((-1) << 3)) == 0; };
    for (int i = 0; i < 100; ++i) {
        std::cout << "i=" << i << "; ans=" << check(i) << std::endl;
    }

    std::cout << SizeOfVariadicTemplate<int, int, float, double>::n << std::endl;
}

// sum: [int]->int
template <int... xs>
struct Sum {};

// sum x  = x
template <int x>
struct Sum<x> {
    static constexpr int value = x;
};
// sum [x:xs] = x + sum(xs)
template <int x, int... xs>
struct Sum<x, xs...> {
    static constexpr int value = x + Sum<xs...>::value;
};

TEST_F(MiscTest, testSum) {
    std::cout << Sum<1, 2, 3, 4, 5, 6, 7>::value << std::endl;
    //using func = void(int);
    //using func = std::function<void(int)>;
    auto f = [](int a) {};
    using func = decltype(f);
    //static_assert(std::is_function_v<func>,"abc");
}

template <typename Callable>
struct CallAs {};

template <typename ReturnT, typename... ParamTypes>
struct FunctorD {
    using CallT = ReturnT (*)(void*, ParamTypes...);

    template <typename CallAsT>
    static ReturnT CallImpl(void* p, ParamTypes... params) {
        CallAsT& func = *reinterpret_cast<CallAsT*>(p);
        return func(params...);
    }
    template <typename CallableT, typename CallAsT>
    FunctorD(CallableT func, CallAs<CallAsT>) {
        f = new char[sizeof(CallableT)];
        new (f) CallableT(std::move(func));
        callImpl = &CallImpl<CallAsT>;
    }

    ReturnT operator()(ParamTypes... params) { return callImpl(f, params...); }
    CallT callImpl;
    void* f;
};
struct FunctorE {
    int operator()(int a, int b) { return a + b; }
};
TEST_F(MiscTest, testCallAs) {
    FunctorD<int, int, int> f1(FunctorE(), CallAs<FunctorE>{});
    std::cout << f1(1, 1) << std::endl;
}

TEST_F(MiscTest, testInt128Div) {
    int128_t a = 1;
    int128_t b = 0;
    int128_t c = a / b;
}
TEST_F(MiscTest, testReduce) {
    std::vector<std::string> xs = {"1", "2", "3", "4", "5"};
    auto x = std::transform_reduce(
            xs.begin(), xs.end(), 0L, [](size_t a, size_t b) { return a + b; },
            [](const std::string& x) { return strtoul(x.c_str(), nullptr, 10); });
    std::cout << x << std::endl;
}

void fast_mul(int64_t a, int64_t b, int128_t* c) {
    int128_t c0 = a * b;
    *c = c0;
}
void fast_mul(int128_t a, int128_t b, int128_t* c) {
    int128_t c0 = a * b;
    *c = c0;
}

TEST_F(MiscTest, testFastMul) {
    int64_t a = 1L << 62;
    int64_t b = 1L << 62;
    int128_t c0 = 0;
    int128_t c1 = 0;
    fast_mul(a, b, &c0);
    fast_mul((int128_t)a, (int128_t)b, &c1);
    print_int128_t(c0);
    print_int128_t(c1);
    ASSERT_EQ(c0, c1);
}

TEST_F(MiscTest, testI32XI32YieldsI64) {
    int32_t a = 1 << 31;
    int32_t b = 1 << 31;
    int64_t c = a * b;
    int64_t c1 = (int64_t)a * (int64_t)b;
    std::cout << c << std::endl;
    std::cout << c1 << std::endl;
}
static int64_t asm_mul32(int32_t x, int32_t y) {
    union {
        int64_t i64;
        struct {
#if __BYTE_ORDER == LITTLE_ENDIAN
            int32_t low;
            int32_t high;
#else
            int32_t high;
            int32_t low;
#endif
        } s;
    } z;
    __asm__ __volatile__(
            "mov %[x], %%eax\n\t"
            "imul %[y]\n\t"
            "mov %%edx, %[high]\n\t"
            "mov %%eax, %[low]"
            : [high] "=r"(z.s.high), [low] "=r"(z.s.low)
            : [x] "r"(x), [y] "r"(y)
            : "cc", "rax", "rdx");
    return z.i64;
}

TEST_F(MiscTest, testMul32) {
    std::cout << asm_mul32(1 << 31, 1 << 31) << std::endl;
    std::cout << (1L << 31) * (1L << 31) << std::endl;
    ASSERT_EQ((int128_t)1 * (int128_t)2, (int128_t)1 * 2);
}

#include <glog/logging.h>

#include <optional>
#include <shared_mutex>
#include <thread>
#include <type_traits>

template <typename T, typename = std::enable_if_t<std::is_copy_assignable<T>::value, T>>
class TlsObject {
public:
    TlsObject() { DCHECK(!pthread_key_create(&_key, ::free)); }
    ~TlsObject() { DCHECK(!pthread_key_delete(_key)); }
    template <typename... Args>
    void set(Args&&... args) {
        void* old = pthread_getspecific(_key);
        if (old != nullptr) {
            delete (T*)old;
        }
        auto obj = new T(std::forward<Args>(args)...);
        DCHECK(!pthread_setspecific(_key, obj));
    }

    std::optional<T> get() const {
        void* old = pthread_getspecific(_key);
        if (old == nullptr) {
            return {};
        }
        return *(T*)old;
    }

private:
    pthread_key_t _key;
};

class AssertHeldSharedMutex : public std::shared_mutex {
    enum AssertHeldState {
        UNLOCKED = 0,
        SHARED_LOCK = 1,
        EXCLUSIVE_LOCK = 2,
    };
    struct AssertHeldInfo {
        explicit AssertHeldInfo(AssertHeldState state) : state(state) {}
        AssertHeldState state;
    };

public:
    void lock() {
        shared_mutex::lock();
        _held_info.set(AssertHeldState::EXCLUSIVE_LOCK);
    }
    bool try_lock() {
        if (shared_mutex::try_lock()) {
            _held_info.set(AssertHeldState::EXCLUSIVE_LOCK);
            return true;
        }
        return false;
    }
    void unlock() {
        shared_mutex::unlock();
        _held_info.set(AssertHeldState::UNLOCKED);
    }

    // Shared ownership

    void lock_shared() {
        shared_mutex::lock_shared();
        _held_info.set(AssertHeldState::SHARED_LOCK);
    }

    bool try_lock_shared() {
        if (shared_mutex::try_lock_shared()) {
            _held_info.set(AssertHeldState::SHARED_LOCK);
            return true;
        }
        return false;
    }

    void unlock_shared() {
        shared_mutex::unlock_shared();
        _held_info.set(AssertHeldState::UNLOCKED);
    }
    void assert_held_shared() {
        auto info = _held_info.get();
        DCHECK(info.has_value() && info.value().state == AssertHeldState::SHARED_LOCK);
    }
    void assert_held_exclusive() {
        auto info = _held_info.get();
        DCHECK(info.has_value() && info.value().state == AssertHeldState::EXCLUSIVE_LOCK);
    }

private:
    TlsObject<AssertHeldInfo> _held_info;
};

TEST_F(MiscTest, testLockHeld0) {
    struct SharedSomething {
        AssertHeldSharedMutex mutex;
        int a = 0;
    };
    std::shared_ptr<SharedSomething> sth = std::make_shared<SharedSomething>();
    std::vector<std::thread> write_threads;
    std::vector<std::thread> read_threads;
    std::atomic<bool> stop = false;
    for (int i = 0; i < 3; ++i) {
        write_threads.emplace_back(
                [&stop, i](std::shared_ptr<SharedSomething> sth) {
                    while (!stop) {
                        std::unique_lock<AssertHeldSharedMutex> wlock(sth->mutex);
                        sth->mutex.assert_held_exclusive();
                        sth->a = i;
                        this_thread::sleep_for(100ms);
                    }
                },
                sth);
    }

    for (int i = 0; i < 3; ++i) {
        read_threads.emplace_back(
                [&stop, i](std::shared_ptr<SharedSomething> sth) {
                    while (!stop) {
                        std::shared_lock<AssertHeldSharedMutex> rlock(sth->mutex);
                        sth->mutex.assert_held_shared();
                        std::cout << sth->a << std::endl;
                        this_thread::sleep_for(100ms);
                    }
                },
                sth);
    }

    this_thread::sleep_for(1000ms);
    stop.store(true);
    for (auto& thd : write_threads) {
        thd.join();
    }
    for (auto& thd : read_threads) {
        thd.join();
    }
}
TEST_F(MiscTest, testAssertLock) {
    AssertHeldSharedMutex a;
    a.assert_held_shared();
}

TEST_F(MiscTest, testPthreadKeyCreate) {
    for (int i = 0; i < 1000000000; ++i) {
        pthread_key_t key;
        int r = pthread_key_create(&key, ::free);
        if (r != 0) {
            std::cout << "i=" << i << ", r=" << r << ", error=" << strerror(r);
            break;
        }
    }
    EAGAIN;
}

struct Range {
    int from;
    int to;
    bool operator<(const Range& rhs) const { return this->to < rhs.to; }

    bool operator==(const Range& rhs) const { return this->from == rhs.from && this->to == rhs.to; }
};

TEST_F(MiscTest, testOrderedMap) {
    std::map<Range, Range> m;
    std::vector<Range> ranges{{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},
                              {8, 8}, {9, 9}, {1, 4}, {4, 5}, {6, 9}, {1, 5}, {3, 6}};
    for (const auto& r : ranges) {
        m.insert(std::pair<Range, Range>(r, r));
    }
    for (const auto& r : ranges) {
        auto it = m.find(r);
        DCHECK(it != m.end());
        DCHECK(it->first == r);
    }
}

struct Range2 {
    int from;
    int to;
    bool operator<(const Range2& rhs) const {
        if (this->from < rhs.from) {
            return true;
        } else if (this->from > rhs.from) {
            return false;
        } else {
            return this->to < rhs.to;
        }
    }

    bool operator==(const Range2& rhs) const { return this->from == rhs.from && this->to == rhs.to; }
};

TEST_F(MiscTest, testOrderedMap2) {
    std::map<Range2, Range2> m;
    std::vector<Range2> ranges{{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},
                               {8, 8}, {9, 9}, {1, 4}, {4, 5}, {6, 9}, {1, 5}, {3, 6}};
    for (const auto& r : ranges) {
        m.insert(std::pair<Range2, Range2>(r, r));
    }
    for (const auto& r : ranges) {
        auto it = m.find(r);
        DCHECK(it != m.end());
        DCHECK(it->first == r);
    }
}
#include <absl/container/flat_hash_map.h>
TEST_F(MiscTest, flatHashMap) {
    absl::flat_hash_map<std::string, std::string> m;
    m.emplace(std::make_pair<std::string, std::string>("a", "b"));
    ASSERT_TRUE(m.find("a") != m.end());
    std::cout << m["a"] << std::endl;
}

TEST_F(MiscTest, testNaN) {
    std::cout << 1.0 / 0.0 << std::endl;
}
#include <any>
#include <variant>
TEST_F(MiscTest, testVariant) {
    std::variant<int, float, double, std::string> v;
    std::variant<int, short, float, double, std::string> v1;
    std::cout << sizeof(v) << std::endl;
    std::cout << sizeof(v1) << std::endl;
    std::any a;

    std::cout << sizeof(a) << std::endl;
}

#include <absl/strings/str_join.h>

template <typename Type>
struct ColumnBuilder {
    void append(const Type& value) { data.push_back(value); }
    //template<typename T, typename=std::enable_if_t<std::is_arithmetic_v<T>&&!std::is_same_v<T, Type>, T>>
    //void append(const T& value) {
    //    append(static_cast<Type>(value));
    //}
    void print() { std::cout << absl::StrJoin(data, ",") << std::endl; }

private:
    std::vector<Type> data;
};

TEST_F(MiscTest, testAppend) {
    ColumnBuilder<int> x;
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
    double y = 3.1415926;
    x.append(0.45);
    x.append(0.3434);
    x.append(y);
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
    x.print();
}

const std::vector<std::string>& func_return_const_ref() {
    return std::vector<std::string>{};
}

TEST_F(MiscTest, TestConstRef) {
    if (func_return_const_ref().empty()) {
        //std::cout<<func_return_const_ref().size()<<std::endl;
    }
}

template <typename... Args>
bool read_values(std::string path, Args&... values) {
    std::ifstream ifs;
    ifs.open(path);
    if (!ifs.good() || !ifs.is_open()) {
        return false;
    }
    (ifs >> ... >> values);
    bool ok = (bool)ifs;
    ifs.close();
    return ok;
}

TEST_F(MiscTest, testReadValues) {
    std::ofstream ofs;
    ofs.open("foobar.1");
    ofs << "10000 30000 40000 abcdefg 3.1415926";
    ofs.flush();
    ofs.close();
    int64_t a = 0;
    int32_t b = 0;
    uint16_t c = 0;
    std::string s = "";
    double d = 0.0;
    ASSERT_TRUE(read_values("foobar.1", a, b, c, s, d));
    std::cout << a << "," << b << ", " << c << "," << s << "," << d << std::endl;
    int e =0;
    ASSERT_FALSE(read_values("foobar.1", a, b, c, s, d, e));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}