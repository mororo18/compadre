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

UTEST(SFTreeNode, slip_symbol_list) {
    auto symb_list = compadre::SymbolList();

    auto A = compadre::Symbol {
        .m_symbol=char('A'),
        .m_probability=20.0f,
    };

    auto B = compadre::Symbol {
        .m_symbol=char('B'),
        .m_probability=20.0f,
    };

    auto C = compadre::Symbol {
        .m_symbol=char('C'),
        .m_probability=20.0f,
    };

    auto D = compadre::Symbol {
        .m_symbol=char('D'),
        .m_probability=20.0f,
    };

    symb_list.push_char_symbol(A);

    // SymbolList size equals to 1
    {
        auto [left, right] = compadre::SFTreeNode::slip_symbol_list(symb_list);
        ASSERT_EQ(left.size(), 1UL);
        ASSERT_EQ(right.size(), 0UL);
    }

    symb_list.push_char_symbol(B);
    symb_list.push_char_symbol(C);

    // SymbolList size equals to 3
    {
        auto [left, right] = compadre::SFTreeNode::slip_symbol_list(symb_list);
        ASSERT_EQ(left.size(), 1UL);
        ASSERT_EQ(right.size(), 2UL);
    }

    symb_list.push_char_symbol(D);

    // SymbolList size equals to 4
    {
        auto [left, right] = compadre::SFTreeNode::slip_symbol_list(symb_list);
        ASSERT_EQ(left.size(), 2UL);
        ASSERT_EQ(right.size(), 2UL);
    }
}
UTEST(SFTreeNode, inner_content_related_methods) {
    auto node = compadre::SFTreeNode(compadre::SymbolList());
    ASSERT_TRUE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());

    node.set_content(compadre::Symbol<char>{});
    ASSERT_TRUE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());

    node.set_content(compadre::SFTreeNode::EmptyNode{});
    ASSERT_TRUE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());

    node.set_content(compadre::SFTreeNode::BranchNode{});
    ASSERT_TRUE(node.has_content_of_type<compadre::SFTreeNode::BranchNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFTreeNode::EmptyNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());

    node.set_content(compadre::SymbolList());
    auto symb_list = node.get_content<compadre::SymbolList>().value();
    ASSERT_TRUE(typeid(symb_list) == typeid(compadre::SymbolList{}));

    node.set_content(compadre::Symbol<char>{});
    auto symb = node.get_content<compadre::Symbol<char>>().value();
    ASSERT_TRUE(typeid(symb) == typeid(compadre::Symbol<char>{}));
}

UTEST_MAIN()
