#include <utest.h/utest.h>
#include <compadre.hpp>
#include <string>
#include <ranges>

UTEST(preprocess, portuguese_text) {
    auto text = std::string("ÀÁÂÃÄÅ àáâãäå ÉÊËéêë ÍÎÏíîï ÓÔÕÖóôõö ÚÛÜúûü Çç 1234!@#$%^&*()-_=+[]{}|;:',.<>?/`~   ");
    auto expected = std::string("AAAAAA AAAAAA EEEEEE IIIIII OOOOOOOO UUUUUU CC");
    auto preproc = compadre::preprocess_portuguese_text(text);
    ASSERT_EQ(expected.size(), preproc.size());

    for (auto const [index, num] : std::views::enumerate(expected)) {
        ASSERT_EQ(num, preproc.at(index));
    }
}

/*
UTEST(preprocess, char_frequencies) {
    float total_prob = 0.0f;
    for (auto& [key, value]: compadre::PreprocessedPortugueseText::char_frequencies) {
        total_prob += value;
    }

    float expected = 100.0f;
    ASSERT_NEAR(total_prob, expected, 0.01f);
}
*/

UTEST(shannon_fano_node, has_content_of_type) {
    auto node = compadre::SFTreeNode(compadre::SymbolList());
    node.get_content<compadre::SymbolList>();
    ASSERT_TRUE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());

    node = compadre::SFTreeNode(compadre::Symbol<char>{});
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());
    ASSERT_TRUE(node.has_content_of_type<compadre::Symbol<char>>());

    node.get_content<compadre::Symbol<char>>();
    ASSERT_TRUE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());
}

UTEST_MAIN()
