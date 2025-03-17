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

    std::unordered_map<char, float> PreprocessedPortugueseText::StaticModel::char_frequencies = {
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
                return a.m_probability < b.m_probability;
            });
    }

    bool SymbolList::is_sorted() {
        return std::ranges::is_sorted(m_list,
            [](decltype(m_list)::value_type a, decltype(m_list)::value_type b) {
                return a.m_probability < b.m_probability;
            });
    }

    std::pair<SymbolList, SymbolList> SFTreeNode::slip_symbol_list(SymbolList& symb_list) {
        float total_propability = 0.0f;

        for (auto symb: symb_list) {
            total_propability += symb.m_probability;
        }

        float half_propability = total_propability / 2.0f;

        std::size_t split_index = 0;
        float min_diff = std::numeric_limits<float>::max();

        auto current_total = 0.0f;
        for (auto [symb_index, symb]: std::views::enumerate(symb_list)) {
            current_total += symb.m_probability;
            float diff_to_half = std::abs(half_propability - current_total);

            if (diff_to_half < min_diff) {
                min_diff = diff_to_half;
                split_index = symb_index;
            }
        }

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
        
        return std::make_pair(left, right);
    }

    auto ShannonFano::generate_code_tree(SymbolList& symb_list) -> CodeTree<SFTreeNode> {
        auto sorted = symb_list;
        sorted.sort();

        auto tree = CodeTree<SFTreeNode>(sorted);
        tree.push_node(SFTreeNode(sorted));

        assert(tree.nodes_count() == 1);

        auto root = tree.get_node_ref_from_index(0);
        assert(root.index().value() == 0); // NOLINT(bugprone-unchecked-optional-access)
        auto stack = std::vector<SFTreeNode>{root};
        auto leaf_nodes_indexes = std::vector<std::size_t>();

        while (not stack.empty()) {
            auto node = stack.back();
            stack.pop_back();

            // Assert that the node doesnt hold a Symbol<char>
            assert(not node.has_content_of_type<Symbol<char>>());
            auto symb_list_opt = node.get_content<SymbolList>();
            auto symb_list = symb_list_opt.value(); // NOLINT(bugprone-unchecked-optional-access)
            // Assert that the SymbolList of the node is sorted;
            assert(symb_list.is_sorted());

            auto [left_list, right_list] = SFTreeNode::slip_symbol_list(symb_list);

            // Assert expected behaviour
            assert(left_list.size() != 0);
            assert(right_list.size() != 0);
            assert(left_list.is_sorted());
            assert(right_list.is_sorted());

            // If the SymbolList of the node has only one Symbol<char>
            // We re-assing its content with that Symbol<char>.
            auto left_content = SFTreeNodeContent(left_list);
            if (left_list.size() == 1) {
                left_content = left_list.front();
            }
            auto right_content = SFTreeNodeContent(right_list);
            if (right_list.size() == 1) {
                right_content = right_list.front();
            }

            auto left_child = SFTreeNode(left_content);
            auto right_child = SFTreeNode(right_content);
            //
            // Store the indexes of the nodes that have a Symbol as content
            if (left_child.has_content_of_type<Symbol<char>>()) {
                left_child.m_symbol = left_child.get_content<Symbol<char>>().value();
            }
            if (right_child.has_content_of_type<Symbol<char>>()) {
                right_child.m_symbol = right_child.get_content<Symbol<char>>().value();
            }

            // Add nodes to tree
            auto parent_index = node.index().value(); // NOLINT(bugprone-unchecked-optional-access)
            auto left_index = tree.add_left_child_to(parent_index, left_child);
            auto right_index = tree.add_right_child_to(parent_index, right_child);

            left_child.m_index = left_index;
            right_child.m_index = right_index;

            auto parent_node = tree.get_node_ref_from_index(parent_index);


            // Push the node into de stack if its content still is
            // a SymbolList
            if (left_child.has_content_of_type<SymbolList>()) {
                stack.push_back(left_child);
            }
            if (right_child.has_content_of_type<SymbolList>()) {
                stack.push_back(right_child);
            }

            // Store the indexes of the nodes that have a Symbol as content
            if (left_child.has_content_of_type<Symbol<char>>()) {
                leaf_nodes_indexes.push_back(left_index);
            }
            if (right_child.has_content_of_type<Symbol<char>>()) {
                leaf_nodes_indexes.push_back(right_index);
            }

            // Set the the parent node as a BranchNode.
            tree.get_node_ref_from_index(parent_index)
                .set_content(BranchNode{});
        }

        /*
        auto code = std::unordered_map<Symbol<char>, CodeWord>();

        // Get the code-words
        for (auto node_index: leaf_nodes_indexes) {
            auto node = tree.get_node_ref_from_index(node_index);
            auto node_code_word = CodeWord();
            auto current_node = node;
            while (true) {
                auto parent_index_opt = current_node.m_parent_index;
                // We break out of the loop in case we found the root node
                // (the node doesnt have a parent.
                if (not parent_index_opt.has_value()) {
                    break;
                }

                auto parent_index = parent_index_opt.value();
                auto parent = tree.get_node_ref_from_index(parent_index);

                // Check if the current node its the right os the left child.
                if (parent.m_left_index.value() == current_node.index().value()) { // NOLINT(bugprone-unchecked-optional-access)
                    node_code_word.push_left_bit(CodeTree<SFTreeNode>::left_branch_bit);
                }
                if (parent.m_right_index.value() == current_node.index().value()) { // NOLINT(bugprone-unchecked-optional-access)
                    node_code_word.push_left_bit(CodeTree<SFTreeNode>::right_branch_bit);
                }

                current_node = parent;
            }

            auto node_symb = node.get_content<Symbol<char>>().value(); // NOLINT(bugprone-unchecked-optional-access)
            code[node_symb] = node_code_word;
        }
        */

        // Print code-words
        /*
        for (auto [symb, code_word]: m_code) {
            std::print("Symbol({}): ", symb.m_symbol);

            auto last_index = (long long)(code_word.m_bits.size()) - 1;
            for (long long bit_index = last_index;
                    bit_index >= 0;
                    bit_index--)
            {
                assert(bit_index >= 0);
                if (std::size_t(bit_index) < code_word.length()) {
                    std::print("{}", int(code_word.m_bits.test(bit_index)));
                }
            }

            std::println(" (length={}),", code_word.length());
        }
        */

        return tree;
    }

    auto ShannonFano::encode_symbol_list(SymbolList& symb_list) -> std::unordered_map<Symbol<char>, CodeWord> {
        auto code_tree = ShannonFano::generate_code_tree(symb_list);
        return code_tree.get_code_map();
    }

    /*
    auto StaticCompressor::compress_preprocessed_portuguese_text(PreprocessedPortugueseText& text) -> std::vector<u8> {
        assert(text.as_string().size() < std::size_t(std::numeric_limits<uint32_t>::max)
                && "Input is too big.");

        //std::println("Text: {}", text.as_string());
        auto symb_list = SymbolList();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto ch_probability = PreprocessedPortugueseText::char_frequencies.at(ch);
            auto symb = Symbol {
                .m_symbol = ch,
                .m_probability = ch_probability,
            };

            symb_list.push_char_symbol(symb);
        }

        auto tree = ShannonFanoTree(symb_list);
        auto code = tree.generate_codes();
        auto symb_count = uint32_t(text.as_string().size());

        std::size_t total_bits{};

        // Buffer of compressed data
        auto outbuff = outbit::BitBuffer();
        // Write symb count in the first 4 bytes.
        outbuff.write(symb_count);
        for (char ch: text.as_string()) {
            auto symb = Symbol {
                .m_symbol = ch,
                .m_probability = 0.0f, // Doenst matter
            };

            auto code_word = code.at(symb);
            total_bits += code_word.length();
            // NOTE: We do this to make the decompression more efficient.
            code_word.reverse_valid_bits();
            auto bits_as_ullong = code_word.m_bits.to_ullong();

            outbuff.write_bits(bits_as_ullong, code_word.length());
        }

        auto bits_per_symb = float(total_bits) / float(text.as_string().size());
        std::println("bits per symb {}", bits_per_symb);
        std::println("razao de comp {}", 5.0f / bits_per_symb);

        return outbuff.buffer();
    }

    auto ShannonFano::decompress_preprocessed_portuguese_text(std::vector<u8>& data) -> PreprocessedPortugueseText {

        auto symb_list = SymbolList();
        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto ch_probability = PreprocessedPortugueseText::char_frequencies.at(ch);
            auto symb = Symbol {
                .m_symbol = ch,
                .m_probability = ch_probability,
            };

            symb_list.push_char_symbol(symb);
        }

        auto tree = ShannonFanoTree(symb_list);
        auto code = tree.generate_codes();
        auto inbuff = outbit::BitBuffer();
        inbuff.read_from_vector(data);
        // Write symb count in the first 4 bytes.
        auto symb_count = inbuff.read_as<uint32_t>();

        auto decompressed_text = std::string();

        // Get the root node
        for (uint32_t symb_index = 0; symb_index < symb_count; symb_index++) {
            auto current_node = tree.get_node_ref_from_index(0);
            auto code_word = CodeWord();
            auto symbol = std::optional<Symbol<char>>();

            while (true) {
                auto current_bit = inbuff.read_bits_as<bool>(1);

                if (current_bit) {
                    assert(ShannonFanoTree::right_branch_bit);
                    auto right_index = current_node.m_right_index.value();
                    current_node = tree.get_node_ref_from_index(right_index);
                } else {
                    assert(!ShannonFanoTree::left_branch_bit);
                    auto left_index = current_node.m_left_index.value();
                    current_node = tree.get_node_ref_from_index(left_index);
                }

                code_word.push_right_bit(current_bit);

                if (current_node.has_content_of_type<Symbol<char>>()) {
                    symbol = current_node.get_content<Symbol<char>>();
                    break;
                }
            }

            assert(code.at(symbol.value()).m_bits == code_word.m_bits);
            decompressed_text += symbol.value().m_symbol;
        }

        return {decompressed_text};
    }
    */
}
