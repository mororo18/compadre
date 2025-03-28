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

// TODO: make this const
static
auto preproc_machado = compadre::PreprocessedPortugueseText(
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

UTEST(Shannon_Fano, preproc_little_roundtrip) {

    auto compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::ShannonFano>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc_machado);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::ShannonFano>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc_machado.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc_machado.as_string().size(); i++) {
        ASSERT_EQ(preproc_machado.as_string()[i], decompressed_text.as_string()[i]);
    }
}

UTEST(SFTreeNode, split_symbol_list) {
    auto symb_list = compadre::SymbolList<compadre::Symbol<char, uint32_t>>();

    auto A = compadre::Symbol('A', 20U);
    auto B = compadre::Symbol('B', 20U);
    auto C = compadre::Symbol('C', 20U);
    auto D = compadre::Symbol('D', 20U);


    symb_list.push(A);

    // SymbolList size equals to 1
    {
        auto [left, right] = compadre::SFTreeNode::slip_symbol_list(symb_list);
        ASSERT_EQ(left.size(), 1UL);
        ASSERT_EQ(right.size(), 0UL);
    }

    symb_list.push(B);
    symb_list.push(C);

    // SymbolList size equals to 3
    {
        auto [left, right] = compadre::SFTreeNode::slip_symbol_list(symb_list);
        ASSERT_EQ(left.size(), 1UL);
        ASSERT_EQ(right.size(), 2UL);
    }

    symb_list.push(D);

    // SymbolList size equals to 4
    {
        auto [left, right] = compadre::SFTreeNode::slip_symbol_list(symb_list);
        ASSERT_EQ(left.size(), 2UL);
        ASSERT_EQ(right.size(), 2UL);
    }
}
UTEST(SFTreeNode, inner_content_related_methods) {
    auto node = compadre::SFTreeNode(compadre::SymbolList<compadre::SFSymbol>());
    ASSERT_TRUE(node.has_content_of_type<compadre::SymbolList<compadre::SFSymbol>>());
    ASSERT_FALSE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::BranchNode>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFSymbol>());

    node.set_content(compadre::SFSymbol());
    ASSERT_TRUE(node.has_content_of_type<compadre::SFSymbol>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList<compadre::SFSymbol>>());
    ASSERT_FALSE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::BranchNode>());

    node.clear_content();
    ASSERT_TRUE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFSymbol>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList<compadre::SFSymbol>>());
    ASSERT_FALSE(node.has_content_of_type<compadre::BranchNode>());

    node.set_content(compadre::BranchNode{});
    ASSERT_TRUE(node.has_content_of_type<compadre::BranchNode>());
    ASSERT_FALSE(node.is_empty());
    ASSERT_FALSE(node.has_content_of_type<compadre::SFSymbol>());
    ASSERT_FALSE(node.has_content_of_type<compadre::SymbolList<compadre::SFSymbol>>());

    node.set_content(compadre::SymbolList<compadre::SFSymbol>());
    auto symb_list = node.get_content<compadre::SymbolList<compadre::SFSymbol>>().value();
    ASSERT_TRUE(typeid(symb_list) == typeid(compadre::SymbolList<compadre::SFSymbol>));

    node.set_content(compadre::SFSymbol());
    auto symb = node.get_content<compadre::SFSymbol>().value();
    ASSERT_TRUE(typeid(symb) == typeid(compadre::SFSymbol));
}

// TODO: Add validation to the test
/*
UTEST(Huffman, generate_code_tree) {
    using namespace compadre;
    auto symb_list = typename SymbolListType<Huffman>::type();
    auto A = HuffmanSymbol('A', 5);
    auto B = HuffmanSymbol('B', 3);
    auto C = HuffmanSymbol('C', 1);
    auto D = HuffmanSymbol('D', 1);
    auto rho = HuffmanSymbol();
    rho.set_attribute(1);

    symb_list.push(A);
    symb_list.push(B);
    symb_list.push(C);
    symb_list.push(D);
    symb_list.push(rho);

    Huffman::generate_code_tree(symb_list);
}
*/

UTEST(Huffman, preproc_little_roundtrip) {

    auto compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc_machado);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc_machado.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc_machado.as_string().size(); i++) {
        ASSERT_EQ(preproc_machado.as_string()[i], decompressed_text.as_string()[i]);
    }
}

UTEST(PPM_Huffman, preproc_little_roundtrip) {
    using namespace compadre;

    auto preproc_input = PreprocessedPortugueseText("eita");
    auto compressor = Compressor<PPM<HuffmanSymbol, 0>, Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc_input);

    // TODO: check if its necessary reconstruct the compressor object.
    /*
    compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc_input.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc_input.as_string().size(); i++) {
        ASSERT_EQ(preproc_input.as_string()[i], decompressed_text.as_string()[i]);
    }
    */
}


UTEST_MAIN()
