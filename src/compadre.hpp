#pragma once

#include <outbit/BitBuffer.hpp>
#include <print>
#include <string>
#include <unordered_map>

namespace compadre {

    const std::unordered_map<char, float> char_frequencies = {
        {' ', 17.00}, {'E', 14.63}, {'A', 13.72}, {'O', 10.73}, {'S', 7.81},
        {'R', 6.53},  {'I', 6.18},  {'N', 5.05},  {'D', 4.99},  {'M', 4.74},
        {'U', 4.63},  {'T', 4.34},  {'C', 3.88},  {'L', 2.78},  {'P', 2.52},
        {'V', 1.67},  {'G', 1.30},  {'H', 1.28},  {'Q', 1.20},  {'B', 1.04},
        {'F', 1.02},  {'Z', 0.47},  {'J', 0.40},  {'X', 0.27},  {'K', 0.02},
        {'W', 0.01},  {'Y', 0.01}
    };

    std::string preprocess_portuguese_text(const std::string& text);

    class PreprocessedPortugueseText {
        private:
            std::string m_text;
        public:
            PreprocessedPortugueseText(const std::string&);
            inline const std::string& get_text() { return m_text; }
    };

    class CompressionAlgorithm {
        protected:
            outbit::BitBuffer m_bitbuffer;
        public:
            virtual void compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) = 0;
    };

    class ShannonFano: public CompressionAlgorithm {
        private:
            int a;
        public:
            void compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) override;
    };

}
