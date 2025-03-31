#include "../src/compadre.hpp"
#include <fstream>
#include <string>

std::string read_file_as_string(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary); // Abre o arquivo em modo binário para preservar caracteres
    if (!file) {
        throw std::runtime_error("Erro ao abrir o arquivo: " + filename);
    }

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}


int main() {
    using namespace compadre;

    auto bras_cubas_string = read_file_as_string("test/MemoriasPostumas.txt");

      // Abrir um arquivo para escrita
    std::ofstream outFile("memorias_postumas_preproc.txt");

    // Verificar se o arquivo foi aberto corretamente
    if (outFile.is_open()) {
        // Escrever a string no arquivo
        auto precproc_bras_cubas = PreprocessedPortugueseText(bras_cubas_string);
        outFile << precproc_bras_cubas.as_string();

        // Fechar o arquivo após a escrita
        outFile.close();
    } else {
    }

    std::println("preproc finished");
    /*

    auto compressor = Compressor< PPM<HuffmanSymbol, 0> , Huffman>();
    auto compressed_data = compressor.compress_preprocessed_portuguese_text(precproc_bras_cubas);
    //std::println("inputê data size = {}", precproc_bras_cubas.as_string().size());
    //std::println("compressed data size = {}", compressed_data.size());

    // TODO: check if its necessary reconstruct the compressor object.
    compressor = Compressor< PPM<HuffmanSymbol, 0> , Huffman>();
    auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(compressed_data);
    //std::println("decompressed = {}", decompressed_text.as_string());
    */
}
