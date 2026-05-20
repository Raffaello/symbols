#include <gtest/gtest.h>

#include <AST.hpp>
#include <ParserLL1.hpp>
#include <array>

TEST(AST, update_root_false)
{
    AST ast;

    auto                        pRoot    = AST::LeafNum::make(0);
    auto                        pNodeUpd = AST::LeafNum::make(1);
    std::unique_ptr<AST::INode> null     = nullptr;
    auto                        pNode    = pRoot.get();
    ast.setRoot(std::move(pRoot));

    ASSERT_FALSE(ast.updateNode(pNode, null));
    ASSERT_FALSE(ast.updateNode(nullptr, null));
    ASSERT_FALSE(ast.updateNode(pNode, pNodeUpd));
    ASSERT_NE(nullptr, pNodeUpd);
}

TEST(AST, update_root)
{
    LexScanner scanner(std::make_unique<std::istringstream>("1+x*2"));
    ParserLL1  parser(scanner);

    ASSERT_TRUE(parser.parse());

    auto& ast = parser.ast();
    ast.print();

    auto pNodeBin = dynamic_cast<const AST::NodeBin*>(ast.getRoot());
    ASSERT_NE(nullptr, pNodeBin);

    // pNodeBin->l // number 1
    auto      l = dynamic_cast<const AST::LeafNum*>(pNodeBin->l.get());
    ast_num_t v;
    ASSERT_TRUE(l->getValue(l, v));
    ASSERT_EQ(1, v);
    auto pNodeUpd = AST::LeafNum::make(10);

    EXPECT_TRUE(ast.updateNode(pNodeBin->l.get(), pNodeUpd));
    EXPECT_EQ(nullptr, pNodeUpd);
    ast.print();

    l = dynamic_cast<const AST::LeafNum*>(pNodeBin->l.get());
    ASSERT_TRUE(l->getValue(l, v));
    ASSERT_EQ(10, v);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
