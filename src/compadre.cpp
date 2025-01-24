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

    const std::array<char, 27> PreprocessedPortugueseText::char_list = {
        ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
        'X', 'Y', 'Z'
    };

    std::unordered_map<char, float> PreprocessedPortugueseText::char_frequencies = {
        {' ', 17.00}, {'E', 14.63}, {'A', 13.72}, {'O', 10.73}, {'S', 7.81},
        {'R', 6.53},  {'I', 6.18},  {'N', 5.05},  {'D', 4.99},  {'M', 4.74},
        {'U', 4.63},  {'T', 4.34},  {'C', 3.88},  {'L', 2.78},  {'P', 2.52},
        {'V', 1.67},  {'G', 1.30},  {'H', 1.28},  {'Q', 1.20},  {'B', 1.04},
        {'F', 1.02},  {'Z', 0.47},  {'J', 0.40},  {'X', 0.27},  {'K', 0.02},
        {'W', 0.01},  {'Y', 0.01}
    };

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
            char upper_c = char(std::toupper(c));
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

    void SymbolList::push_char_symbol(Symbol<char> symb) {
        m_list.push_back(symb);
    }

    void SymbolList::sort() {
        std::ranges::sort(m_list,
            [](decltype(m_list)::value_type a, decltype(m_list)::value_type b) {
                return a.m_propability < b.m_propability;
            });
    }

    SFTreeNode::SFTreeNode(NodeContent content)
        : m_content(std::move(content)),
        m_left_index(std::nullopt),
        m_right_index(std::nullopt)
    {
    }

    template<typename T>
    bool SFTreeNode::content_of_type() {
        return std::holds_alternative<T>(m_content);
    }

    template<typename T>
    bool SFTreeNode::content_of_type() const {
        return std::holds_alternative<T>(m_content);
    }

    std::size_t ShannonFanoTree::push_node(const SFTreeNode& node) {
        assert(not node.is_empty());

        for (auto [node_index, tree_node]: std::views::enumerate(m_tree)) {
            if (tree_node.is_empty()) {
                tree_node = node;
                tree_node.m_index = node_index;
                return node_index;
            }
        }

        m_tree.push_back(node);
        m_tree.back().m_index = m_tree.size() - 1;
        return m_tree.size() - 1;
    }

    void ShannonFanoTree::add_right_child_to(SFTreeNode& node, const SFTreeNode& child) {
        auto child_index = push_node(child); 
        node.m_right_index = child_index;
    }

    void ShannonFanoTree::add_left_child_to(SFTreeNode& node, const SFTreeNode& child) {
        auto child_index = push_node(child); 
        node.m_left_index = child_index;
    }

    void ShannonFano::compress_preprocessed_portuguese_text(PreprocessedPortugueseText& text) {
        std::println("{}", text.get_text());
        auto tree = ShannonFanoTree();
        auto symb_list = SymbolList();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto ch_probability = PreprocessedPortugueseText::char_frequencies.at(ch);
            auto symb = Symbol {
                .m_symbol = ch,
                .m_propability = ch_probability,
            };

            symb_list.push_char_symbol(symb);
        }

        auto root = SFTreeNode(symb_list);
    }
}
