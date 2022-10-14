#include <gtest/gtest.h>

#include "analysis/analysis.h"
namespace analysis {
struct AnalysisTest : public ::testing::Test {};
TEST_F(AnalysisTest, TestConstructCfg) {
    auto const1 = ExprFactory<LiteralExpr<SimpleType::TYPE_INT32>>::create(1);
    auto var1 = ExprFactory<VarDefExpr>::create("a", const1);
    auto var2 = ExprFactory<VarDefExpr>::create("b", ExprFactory<LiteralExpr<TYPE_INT32>>::create(2));
    auto var1_use = ExprFactory<VarUseExpr>::create("a");
    auto var2_use = ExprFactory<VarUseExpr>::create("b");
    auto var1_add_var2 = ExprFactory<ArithmeticExpr>::create(OP_ADD, std::vector<ExprPtr>{var1_use, var2_use});
    auto var3 = ExprFactory<VarDefExpr>::create("c", var1_add_var2);
    std::cout << const1->to_string() << std::endl;
    std::cout << var1->to_string() << std::endl;
    std::cout << var2->to_string() << std::endl;
    std::cout << var3->to_string() << std::endl;
    auto stmt1 = StmtFactory<SimpleStmt>::create(var1);
    auto stmt2 = StmtFactory<SimpleStmt>::create(var2);
    auto stmt3 = StmtFactory<SimpleStmt>::create(var3);
    std::cout << stmt1->to_string() << std::endl;
    std::cout << stmt2->to_string() << std::endl;
    std::cout << stmt3->to_string() << std::endl;
    auto blk1 = std::make_shared<StmtBlock>(std::vector{stmt1, stmt2, stmt2});

    auto var4 = ExprFactory<VarDefExpr>::create(
            "a",
            ExprFactory<ArithmeticExpr>::create(OP_ADD, std::vector{ExprFactory<VarUseExpr>::create("a"),
                                                                    ExprFactory<LiteralExpr<TYPE_INT32>>::create(1)}));
    auto stmt4 = StmtFactory<SimpleStmt>::create(var4);
    auto stmt5 = StmtFactory<BranchStmt>::create(
            1, std::optional{3},
            ExprFactory<RelationExpr>::create(OP_LT, std::vector{ExprFactory<VarUseExpr>::create("a"),
                                                                 ExprFactory<LiteralExpr<TYPE_INT32>>::create(10)

                                                     }));
    auto blk2 = std::make_shared<StmtBlock>(Statements{stmt4, stmt5});
    auto blk3 = std::make_shared<StmtBlock>(Statements{});
    Cfg cfg;
    blk1->set_id(1);
    blk1->set_succ_ids({2});
    blk2->set_id(2);
    blk2->set_succ_ids({1, 3});
    blk2->set_pred_ids({1});
    blk3->set_id(3);
    blk3->set_pred_ids({2});
    cfg.add(std::move(blk1));
    cfg.add(std::move(blk2));
    cfg.add(std::move(blk3));
}
} // namespace analysis

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
