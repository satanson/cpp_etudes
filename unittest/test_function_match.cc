// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/07.
//

#include <gtest/gtest.h>

#include "function_match.hh"

namespace test {
class FunctionMatchTest : public ::testing::Test {};
TEST_F(FunctionMatchTest, test_partial_order_relation) {
    PartialOrderRelation<int> por;
    por.Put(1, 2, 3, 4);
    ASSERT_TRUE(por.Contains(1, 2));
    ASSERT_TRUE(por.Contains(1, 3));
    ASSERT_TRUE(por.Contains(1, 4));
    ASSERT_FALSE(por.Contains(2, 4));
    ASSERT_FALSE(por.Contains(4, 1));
}

TEST_F(FunctionMatchTest, test_partial_order_relation_fail) {
    PartialOrderRelation<int> por;
    por.Put(1, 2, 3, 4);
    ASSERT_ANY_THROW(por.Put(1, 2));
    ASSERT_ANY_THROW(por.Put(4, 1));
}
enum DataType {
    TYPE_INT,
    TYPE_BIGINT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_DECIMAL32,
    TYPE_DECIMAL64,
    TYPE_DECIMAL128,
};
std::string datatype_to_string(DataType dt) {
    switch (dt) {
    case TYPE_INT:
        return "int";
    case TYPE_BIGINT:
        return "bigint";
    case TYPE_FLOAT:
        return "float";
    case TYPE_DOUBLE:
        return "double";
    case TYPE_STRING:
        return "string";
    case TYPE_DECIMAL32:
        return "decimal32";
    case TYPE_DECIMAL64:
        return "decimal64";
    case TYPE_DECIMAL128:
        return "decimal128";
    }
    return "unknown";
}

std::string term_to_string(std::vector<DataType> const& term) {
    if (term.empty()) {
        return "term()";
    }
    if (term.size() == 1) {
        return std::string("term(") + datatype_to_string(term[0]) + ")";
    }
    std::stringstream ss;
    ss << "term(" << datatype_to_string(term.front());
    for (auto i = 1; i < term.size(); ++i) {
        ss << ", " << datatype_to_string(term[i]);
    }
    ss << ")";
    return ss.str();
}

TEST_F(FunctionMatchTest, test_function_match_ok) {
    PartialOrderRelation<DataType> por;
    por.PutReverse(TYPE_DOUBLE, TYPE_INT, TYPE_BIGINT, TYPE_FLOAT, TYPE_DECIMAL32, TYPE_DECIMAL64, TYPE_DECIMAL128);
    por.PutReverse(TYPE_BIGINT, TYPE_INT);
    por.PutReverse(TYPE_DECIMAL64, TYPE_DECIMAL32);
    por.PutReverse(TYPE_DECIMAL128, TYPE_DECIMAL32, TYPE_DECIMAL64);
    por.PutReverse(TYPE_STRING, TYPE_INT, TYPE_BIGINT, TYPE_FLOAT, TYPE_DOUBLE, TYPE_DECIMAL32, TYPE_DECIMAL64,
                   TYPE_DECIMAL128);
    TermDict<DataType> td(por);
    using TermType = TermDict<DataType>::TermType;
    td.Put(TermType{TYPE_INT, TYPE_INT, TYPE_INT});

    td.Put(TermType{TYPE_DOUBLE, TYPE_DOUBLE, TYPE_DOUBLE});
    td.Put(TermType{TYPE_FLOAT, TYPE_BIGINT, TYPE_DOUBLE});
    td.Put(TermType{TYPE_DECIMAL128, TYPE_DECIMAL128, TYPE_DECIMAL128});
    td.Put(TermType{TYPE_STRING, TYPE_STRING, TYPE_STRING});
    auto a = td.BestMatch(TermType{TYPE_INT, TYPE_INT, TYPE_INT});
    std::cout << term_to_string(a) << std::endl;
    auto b = td.BestMatch(TermType{TYPE_BIGINT, TYPE_INT, TYPE_INT});
    std::cout << term_to_string(b) << std::endl;
    auto c = td.BestMatch(TermType{TYPE_DECIMAL32, TYPE_INT, TYPE_INT});
    std::cout << term_to_string(c) << std::endl;
    auto d = td.BestMatch(TermType{TYPE_DECIMAL32, TYPE_DECIMAL32, TYPE_DECIMAL32});
    std::cout << term_to_string(d) << std::endl;
    auto e = td.BestMatch(TermType{TYPE_DECIMAL64, TYPE_DECIMAL64, TYPE_DECIMAL64});
    std::cout << term_to_string(e) << std::endl;
}

} // namespace test

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}