#include "compadre.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <print>
#include <cassert>
#include <locale>

namespace compadre {

    PreprocessedPortugueseText::PreprocessedPortugueseText(const std::string& text) {
        m_text = preprocess_portuguese_text(text);
    }

    static std::unordered_map<wchar_t, char> create_accent_map() {
        const std::wstring accented = L"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ";
        const std::string unaccented = "AAAAAAECEEEEIIIIDNOOOOOxUUUUYPsaaaaaaeceeeeiiiiOnooooo0uuuuypy";
        assert(accented.size() == unaccented.size());

        std::unordered_map<wchar_t, char> accent_map;
        for (size_t i = 0; i < accented.size(); ++i) {
            accent_map[accented[i]] = unaccented[i];
        }
        return accent_map;
    }

    // TODO: write test for this function (remove static)
    static std::string remove_accents(const std::wstring& text) {
        auto accent_map = create_accent_map();

        std::string result;
        for (wchar_t ch : text) {
            if (accent_map.contains(ch)) {
                result += accent_map.at(ch);
            } else {
                result += char(ch);
            }
        }

        return result;
    }
    std::string preprocess_portuguese_text(const std::string& text) {
        static const std::unordered_set<char> allowed_chars = {
            ' ', 'E', 'A', 'O', 'S', 'R', 'I', 'N', 'D', 'M', 'U', 'T', 'C', 'L', 'P',
            'V', 'G', 'H', 'Q', 'B', 'F', 'Z', 'J', 'X', 'K', 'W', 'Y'
        };

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(text);

        auto unaccented_text = remove_accents(wide);
        auto result = std::string();
        for (char c : unaccented_text) {
            char upper_c = std::toupper(c);
            if (allowed_chars.count(upper_c)) {
                result += upper_c;
            }
        }

        // Remover espaços repetidos
        std::stringstream ss(result);
        std::string word, final_result;
        while (ss >> word) {
            if (!final_result.empty()) {
                final_result += ' ';
            }
            final_result += word;
        }

        return final_result;
    }

    void ShannonFano::compress_preprocessed_portuguese_text(PreprocessedPortugueseText& text) {
        std::println("{}", text.get_text());
    }
}
