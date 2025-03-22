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

UTEST(Shannon_Fano, preproc_little_roundtrip) {
    auto preproc = compadre::PreprocessedPortugueseText(
        "Fui descalçar as botas, que estavam apertadas. Uma vez aliviado, respirei à larga, "
        "e deitei-me a fio comprido, enquanto os pés, e todo eu atrás deles, entrávamos numa "
        "relativa bem-aventurança. Então considerei que as botas apertadas são uma das maiores "
        "venturas da Terra, porque, fazendo doer os pés, dão azo ao prazer de as descalçar. "
        "Mortifica os pés, desgraçado, desmortifica-os depois, e aí tens a felicidade barata, "
        "ao sabor dos sapateiros e de Epicuro. [...] Inferi eu que a vida é o mais engenhoso dos "
        "fenômenos, porque só aguça a fome, com o fim de deparar a ocasião de comer, e não inventou "
        "os calos, senão porque eles aperfeiçoam a felicidade terrestre. Em verdade vos digo que toda "
        "a sabedoria humana não vale um par de botas curtas."
    );

    auto compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::ShannonFano>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::ShannonFano>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc.as_string().size(); i++) {
        ASSERT_EQ(preproc.as_string()[i], decompressed_text.as_string()[i]);
    }
}

UTEST(SFTreeNode, slip_symbol_list) {
    auto symb_list = compadre::SymbolList();

    auto A = compadre::Symbol<char>('A', 20.0f);
    auto B = compadre::Symbol<char>('B', 20.0f);
    auto C = compadre::Symbol<char>('C', 20.0f);
    auto D = compadre::Symbol<char>('D', 20.0f);


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
    ASSERT_FALSE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::BranchNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());

    node.set_content(compadre::Symbol<char>());
    ASSERT_TRUE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::BranchNode>());

    node.clear_content();
    ASSERT_TRUE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());
    ASSERT_FALSE(node.has_content_of_type<compadre::BranchNode>());

    node.set_content(compadre::BranchNode{});
    ASSERT_TRUE(node.has_content_of_type<compadre::BranchNode>());
    ASSERT_FALSE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::Symbol<char>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList>());

    node.set_content(compadre::SymbolList());
    auto symb_list = node.get_content<compadre::SymbolList>().value();
    ASSERT_TRUE(typeid(symb_list) == typeid(compadre::SymbolList{}));

    node.set_content(compadre::Symbol<char>());
    auto symb = node.get_content<compadre::Symbol<char>>().value();
    ASSERT_TRUE(typeid(symb) == typeid(compadre::Symbol<char>));
}

UTEST_MAIN()
