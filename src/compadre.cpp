#include "compadre.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <print>
#include <cassert>
#include <locale>
#include <utility>

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

    bool SymbolList::is_sorted() {
        return std::ranges::is_sorted(m_list,
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

    std::pair<SymbolList, SymbolList> SFTreeNode::slip_symbol_list(SymbolList& symb_list) {
        float total_propability = 0.0f;

        for (auto symb: symb_list) {
            total_propability += symb.m_propability;
        }

        std::println("total probability {}.", total_propability);

        float half_propability = total_propability / 2.0f;

        std::size_t split_index = 0;
        float min_diff = std::numeric_limits<float>::max();

        auto current_total = 0.0f;
        for (auto [symb_index, symb]: std::views::enumerate(symb_list)) {
            current_total += symb.m_propability;
            float diff_to_half = std::abs(half_propability - current_total);

            if (diff_to_half < min_diff) {
                min_diff = diff_to_half;
                split_index = symb_index;
            }
        }

        std::println("min diff probability {}.", min_diff);

        auto left = SymbolList();
        auto right = SymbolList();

        for (auto [symb_index, symb]: std::views::enumerate(symb_list)) {
            assert(symb_index >= 0);
            if (std::size_t(symb_index) <= split_index) {
                left.push_char_symbol(symb);
            } else {
                right.push_char_symbol(symb);
            }
        }

        assert(left.is_sorted());
        assert(right.is_sorted());
        
        return std::make_pair(left, right);
    }

    ShannonFanoTree::ShannonFanoTree(SymbolList& symb_list) {
        auto sorted = symb_list;
        sorted.sort();
        auto root = SFTreeNode(sorted);
        push_node(root);
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

    std::size_t ShannonFanoTree::add_right_child_to(std::size_t parent_index, const SFTreeNode& child) {
        auto child_index = push_node(child); 
        auto& parent = get_node_ref_from_index(parent_index);
        parent.m_right_index = child_index;

        return child_index;
    }

    std::size_t ShannonFanoTree::add_left_child_to(std::size_t parent_index, const SFTreeNode& child) {
        auto child_index = push_node(child); 
        auto& parent = get_node_ref_from_index(parent_index);
        parent.m_left_index = child_index;

        return child_index;
    }

    void ShannonFanoTree::generate_codes() {
        assert(m_tree.size() == 1);

        auto root = m_tree.back();
        std::vector<SFTreeNode> stack{root};

        while (not stack.empty()) {
            std::println("iter");
            auto node = stack.back();
            stack.pop_back();

            if (node.has_content_of_type<Symbol<char>>()) {
                std::println("Has Symbol as content.");
                continue;
            }

            // NOLINT(bugprone-unchecked-optional-access)
            auto symb_list_opt = node.get_content<SymbolList>();

            if (not symb_list_opt.has_value()) {
                std::println("node {} doenst has symbol_list.", node.m_index.value());
            }

            auto symb_list = symb_list_opt.value();
            assert(symb_list.is_sorted());

            auto [left_list, right_list] = SFTreeNode::slip_symbol_list(symb_list);

            std::println("parent size {}. left size {}, right {}.", symb_list.size(), left_list.size(), right_list.size());

            auto left_content = SFTreeNode::NodeContent(left_list);
            if (left_list.size() == 1) {
                left_content = left_list.front();
            }

            auto right_content = SFTreeNode::NodeContent(right_list);
            if (right_list.size() == 1) {
                right_content = right_list.front();
            }

            auto left_child = SFTreeNode(left_content);
            auto right_child = SFTreeNode(right_content);

            auto parent_index = node.index();


            // Add nodes to tree
            auto left_index = add_left_child_to(parent_index, left_child);
            auto right_index = add_right_child_to(parent_index, right_child);

            left_child.m_index = left_index;
            right_child.m_index = right_index;

            auto parent_node = get_node_ref_from_index(parent_index);

            std::println("node {} is parent of {} (left) and {} (right).", parent_index, parent_node.m_left_index.value(), parent_node.m_right_index.value());

            // Push nodes into de stack
            if (left_list.size() > 1) {
                stack.push_back(left_child);
            }

            if (right_list.size() > 1) {
                stack.push_back(right_child);
            }

            auto& parent = get_node_ref_from_index(parent_index);
            parent.set_content(SFTreeNode::BranchNode{});
        }

        // For each node that is not in the tree.
        //     1. Sort the symbol list
        //     2. Create right and left child and slip the node's SymbolList
        //     3. Add the childs into the tree.

    }

    void ShannonFano::compress_preprocessed_portuguese_text(PreprocessedPortugueseText& text) {
        std::println("Text: {}", text.get_text());
        auto symb_list = SymbolList();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto ch_probability = PreprocessedPortugueseText::char_frequencies.at(ch);
            auto symb = Symbol {
                .m_symbol = ch,
                .m_propability = ch_probability,
            };

            symb_list.push_char_symbol(symb);
        }

        auto tree = ShannonFanoTree(symb_list);

        tree.generate_codes();

        std::println("{}", tree);
    }
}
