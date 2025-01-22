#include "compadre.hpp"

int main() {
    auto preproc = compadre::PreprocessedPortugueseText("eíta bóú");

    auto compressor = compadre::ShannonFano();
    compressor.compress_preprocessed_portuguese_text(preproc);

    return 0;
}
