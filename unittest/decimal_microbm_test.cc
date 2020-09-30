//
// Created by grakra on 2020/9/29.
//

#include <gtest/gtest.h>
#include <decimal_microbm/decimal_microbm.hh>

namespace test {
class DecimalMicroBMTest : public testing::Test {

};
TEST_F(DecimalMicroBMTest, TestBatchCompute){
  int128_t zero = static_cast<int128_t>(0);
  std::vector<int128_t> lhs(512, zero);
  std::vector<int128_t> rhs(512, zero);
  std::vector<int128_t> result(512,zero);
}

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
