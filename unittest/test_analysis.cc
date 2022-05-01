#include <gtest/gtest.h>

#include "analysis/analysis.h"
namespace analysis {
struct AnalysisTest : public ::testing::Test {};
} // namespace analysis

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}