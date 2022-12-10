// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2022/11/12.
//
#include <gtest/gtest.h>
#include <llvm-c/Core.h>

#include <memory>
namespace test {
class LLVMTest : public ::testing::Test {};
typedef struct OpaqueObj* ObjRef;
TEST_F(LLVMTest, testBasic) {
    std::string s = "abc";
    ObjRef s0 = reinterpret_cast<ObjRef>(&s);
    std::string* s2 = reinterpret_cast<std::string*>(s0);
    std::cout << *s2 << std::endl;
}

TEST_F(LLVMTest, testBackInserter) {
    std::vector<int> xs{0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<int> ys;
    ys.reserve(xs.size());
    auto has3 = false;
    std::copy_if(xs.begin(), xs.end(), std::back_inserter(ys), [&has3](int x) {
        has3 |= x == 3;
        return x % 2 == 1;
    });
    for (int i = 0; i < ys.size(); ++i) {
        std::cout << ys[i] << std::endl;
    }
    std::cout << "has3:" << has3 << std::endl;
}
struct Referee {
public:
    explicit Referee(const std::string& s) : _s(s) {}
    ~Referee() = default;
    void print() const { std::cout << _s << std::endl; }

private:
    std::string _s;
};
using RefereePtr = std::shared_ptr<Referee>;
using Referees = std::vector<RefereePtr>;
struct Reference {
public:
    Reference(const Referees& referees) : _referees(referees) {}
    ~Reference() {
        std::for_each(_referees.begin(), _referees.end(), [](const auto& ref) { ref->print(); });
    }

private:
    Referees _referees;
};
using ReferencePtr = std::shared_ptr<Reference>;
using RefereesAndReference = std::tuple<Referees, ReferencePtr>;
RefereesAndReference getRefereesAndRef() {
    Referees referees;
    referees.reserve(10);
    for (int i = 0; i < 10; ++i) {
        referees.push_back(std::make_shared<Referee>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    }
    return std::make_tuple(referees, std::make_shared<Reference>(referees));
}
TEST_F(LLVMTest, testTupleReference) {
    std::cout << "begin" << std::endl;
    {
        auto [referees, ref] = getRefereesAndRef();
        std::cout << referees.size() << std::endl;
    }
    std::cout << "end" << std::endl;
}

} // namespace test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}