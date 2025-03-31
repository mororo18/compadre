#include <utest.h/utest.h>
#include <compadre.hpp>
#include <string>
#include <fstream>
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

auto print_compression_info(compadre::CompressionInfo comp_info){
    std::println("CompressionInfo = ( entropy={}, avg_len={} )",
            comp_info.entropy, comp_info.avg_lenght);
}

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

std::string read_file_as_string(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary); // Abre o arquivo em modo binário para preservar caracteres
    if (!file) {
        throw std::runtime_error("Erro ao abrir o arquivo: " + filename);
    }

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}


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

UTEST(Huffman, preproc_roundtrip) {

    auto bras_cubas_string = read_file_as_string("MemoriasPostumas.txt");
    std::println("preproc finished");
    auto preproc_bras_cubas = compadre::PreprocessedPortugueseText(bras_cubas_string);

    auto compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc_bras_cubas);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = compadre::Compressor<compadre::PreprocessedPortugueseText::StaticModel, compadre::Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(preproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }
}

UTEST(PPM_Huffman, preproc_little_roundtrip_test) {
    using namespace compadre;

    auto preproc_test = PreprocessedPortugueseText("aii");
    auto compressor = Compressor< PPM<HuffmanSymbol, 0> , Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc_test);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = Compressor< PPM<HuffmanSymbol, 0> , Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc_test.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc_test.as_string().size(); i++) {
        ASSERT_EQ(preproc_test.as_string()[i], decompressed_text.as_string()[i]);
    }
}


UTEST(PPM_Huffman, preproc_little_roundtrip) {
    using namespace compadre;

    auto compressor = Compressor< PPM<HuffmanSymbol, 10> , Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc_machado);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = Compressor< PPM<HuffmanSymbol, 10> , Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(preproc_machado.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < preproc_machado.as_string().size(); i++) {
        ASSERT_EQ(preproc_machado.as_string()[i], decompressed_text.as_string()[i]);
    }
}

UTEST(PPM_Huffman, preproc_roundtrip) {
    using namespace compadre;

    auto bras_cubas_string = read_file_as_string("MemoriasPostumas.txt");
    auto precproc_bras_cubas = PreprocessedPortugueseText(bras_cubas_string);

    auto compressor = Compressor< PPM<HuffmanSymbol, 2> , Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(precproc_bras_cubas);

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = Compressor< PPM<HuffmanSymbol, 2> , Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());

    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }
}

UTEST(PPM_Huffman, leonardo) {
    using namespace compadre;

    auto bras_cubas_string = read_file_as_string("MemoriasPostumas.txt");
    auto precproc_bras_cubas = PreprocessedPortugueseText(bras_cubas_string);

    // ===== K=0 =====
    std::println("K=0");
    auto compressor = Compressor< PPM<HuffmanSymbol, 0> , Huffman>();

    auto inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    auto fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    std::chrono::duration<double, std::milli> duracao = fim - inicio;
    std::println("Tempo de compressao: {}as", duracao.count() / 1000.0);


    print_compression_info(compressor.compression_info());

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = Compressor< PPM<HuffmanSymbol, 0> , Huffman>();

    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de descompressao: {}s", duracao.count() / 1000.0);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());
    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }

    // ===== K=1 =====
    std::println("K=1");
    auto compressor_one = Compressor< PPM<HuffmanSymbol, 1> , Huffman>();
    compressed_data = compressor_one.compress_preprocessed_portuguese_text(precproc_bras_cubas);

    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    compressed_data = compressor_one.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de compressao: {}s", duracao.count() / 1000.0);

    print_compression_info(compressor_one.compression_info());

    compressor_one = Compressor< PPM<HuffmanSymbol, 1> , Huffman>();
    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    decompressed_text = compressor_one.decompress_preprocessed_portuguese_text(compressed_data);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de descompressao: {}s", duracao.count() / 1000.0);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());
    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }



    // ===== K=2 =====
    std::println("K=2");
    auto compressor_two = Compressor< PPM<HuffmanSymbol, 2> , Huffman>();
    compressed_data = compressor_two.compress_preprocessed_portuguese_text(precproc_bras_cubas);

    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    compressed_data = compressor_two.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de compressao: {}s", duracao.count() / 1000.0);

    print_compression_info(compressor_two.compression_info());

    compressor_two = Compressor< PPM<HuffmanSymbol, 2> , Huffman>();
    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    decompressed_text = compressor_two.decompress_preprocessed_portuguese_text(compressed_data);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de descompressao: {}s", duracao.count() / 1000.0);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());
    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }



    // ===== K=3 =====
    std::println("K=3");
    auto compressor_three = Compressor< PPM<HuffmanSymbol, 3> , Huffman>();
    compressed_data = compressor_three.compress_preprocessed_portuguese_text(precproc_bras_cubas);

    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    compressed_data = compressor_three.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de compressao: {}s", duracao.count() / 1000.0);

    print_compression_info(compressor_three.compression_info());

    compressor_three = Compressor< PPM<HuffmanSymbol, 3> , Huffman>();
    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    decompressed_text = compressor_three.decompress_preprocessed_portuguese_text(compressed_data);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de descompressao: {}s", duracao.count() / 1000.0);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());
    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }




    // ===== K=4 =====
    std::println("K=4");
    auto compressor_four = Compressor< PPM<HuffmanSymbol, 4> , Huffman>();
    compressed_data = compressor_four.compress_preprocessed_portuguese_text(precproc_bras_cubas);

    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    compressed_data = compressor_four.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de compressao: {}s", duracao.count() / 1000.0);

    print_compression_info(compressor_four.compression_info());

    compressor_four = Compressor< PPM<HuffmanSymbol, 4> , Huffman>();
    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    decompressed_text = compressor_four.decompress_preprocessed_portuguese_text(compressed_data);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de descompressao: {}s", duracao.count() / 1000.0);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());
    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }

    // ===== K=5 =====
    std::println("K=5");
    auto compressor_five = Compressor< PPM<HuffmanSymbol, 5> , Huffman>();
    compressed_data = compressor_five.compress_preprocessed_portuguese_text(precproc_bras_cubas);

    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    compressed_data = compressor_five.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de compressao: {}s", duracao.count() / 1000.0);

    print_compression_info(compressor_five.compression_info());

    compressor_five = Compressor< PPM<HuffmanSymbol, 5> , Huffman>();
    inicio = std::chrono::high_resolution_clock::now();  // Marca o tempo inicial
    decompressed_text = compressor_five.decompress_preprocessed_portuguese_text(compressed_data);
    fim = std::chrono::high_resolution_clock::now();  // Marca o tempo final
    duracao = fim - inicio;
    std::println("Tempo de descompressao: {}s", duracao.count() / 1000.0);

    ASSERT_EQ(precproc_bras_cubas.as_string().size(), decompressed_text.as_string().size());
    for (std::size_t i = 0; i < precproc_bras_cubas.as_string().size(); i++) {
        ASSERT_EQ(precproc_bras_cubas.as_string()[i], decompressed_text.as_string()[i]);
    }
}



UTEST_MAIN()
