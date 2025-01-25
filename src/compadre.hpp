#pragma once

#include <outbit/BitBuffer.hpp>
#include <print>
#include <string>
#include <unordered_map>
#include <utility>
#include <format>

namespace compadre {

    using u8 = outbit::u8;
    using Bit = bool;


    std::string preprocess_portuguese_text(const std::string& text);

    class PreprocessedPortugueseText {
        private:
            std::string m_text;
        public:
            PreprocessedPortugueseText(const std::string&);
            inline const std::string& get_text() { return m_text; }
            static std::unordered_map<char, float> char_frequencies;
            static const std::array<char, 27> char_list;
    };

    class CompressionAlgorithm {
        protected:
            outbit::BitBuffer m_bitbuffer;
        public:
            virtual void compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) = 0;
    };

    template<typename T>
    struct Symbol {
        T m_symbol;
        float m_probability;

        bool operator==(const Symbol& other) const {
            return m_symbol == other.m_symbol;
        }
    };

    class CodeWord {
        public:
            std::bitset<32> m_bits;
            std::size_t m_bit_count;

            CodeWord() = default;
            inline void push_bit(Bit bit) {
                // Bounds checking
                m_bits.test(m_bit_count);
                m_bits[m_bit_count] = bit;
                m_bit_count++;
            }

            inline std::size_t length() { return m_bit_count; }
    };

    class SymbolList {
        public:
            SymbolList() = default;
            void sort();
            bool is_sorted();
            void push_char_symbol(Symbol<char> symb);
            
            [[nodiscard]]
            inline auto cbegin() const noexcept { return m_list.cbegin(); }
            inline auto begin() noexcept { return m_list.begin(); }

            [[nodiscard]]
            inline auto cend() const noexcept { return m_list.cend(); }
            inline auto end() noexcept { return m_list.end(); }

            inline std::size_t size() { return m_list.size(); }
            inline Symbol<char> front() { return *m_list.begin(); }
        private:
            std::vector<Symbol<char>> m_list;
    };

    class SFTreeNode {
        public:
            class EmptyNode {};
            class BranchNode {};
            using NodeContent = std::variant<Symbol<char>, SymbolList, BranchNode, EmptyNode>;

            NodeContent m_content;
            std::optional<std::size_t> m_index;
            std::optional<std::size_t> m_left_index;
            std::optional<std::size_t> m_right_index;
            std::optional<std::size_t> m_parent_index;
            SFTreeNode(NodeContent content = EmptyNode{});

            static
            std::pair<SymbolList, SymbolList> slip_symbol_list(SymbolList& symb_list);

            template<typename ContentVariant>
            inline bool has_content_of_type() {
                return std::holds_alternative<ContentVariant>(m_content);
            }

            template<typename ContentVariant>
            [[nodiscard]]
            inline bool has_content_of_type() const {
                return std::holds_alternative<ContentVariant>(m_content);
            }

            [[nodiscard]]
            inline bool is_empty() const { return has_content_of_type<EmptyNode>(); }
            inline bool is_empty() { return has_content_of_type<EmptyNode>(); }

            template<typename ContentVariant>
            [[nodiscard]]
            inline std::optional<ContentVariant> get_content() const {
                if (has_content_of_type<ContentVariant>()) {
                    return std::get<ContentVariant>(m_content);
                }

                return std::nullopt;
            }

            template<typename ContentVariant>
            inline std::optional<ContentVariant> get_content() {
                if (has_content_of_type<ContentVariant>()) {
                    return std::get<ContentVariant>(m_content);
                }

                return std::nullopt;
            }

            inline void set_content(NodeContent new_content) {
                m_content = std::move(new_content);
            }

            [[nodiscard]]
            inline std::optional<std::size_t> index() const { return m_index; }
            inline std::optional<std::size_t> index() { return m_index; }
    };
}

// NOTE: This specialization needs to be declared after the
// ShannonFanoTree class.
template <>
struct std::hash<compadre::Symbol<char>> {
    size_t operator()(const compadre::Symbol<char> &s) const {
        return std::hash<char>()(s.m_symbol);
    }
};

namespace compadre {

    class ShannonFanoTree {
        private:
            std::vector<SFTreeNode> m_tree;
            SymbolList m_symb_list;
            std::unordered_map<Symbol<char>, CodeWord> m_code;
        public:
            static Bit left_branch_bit;
            static Bit right_branch_bit;
            ShannonFanoTree(SymbolList& symb_list);
            std::size_t push_node(const SFTreeNode& node);
            std::size_t add_left_child_to(std::size_t parent_index, const SFTreeNode& child);
            std::size_t add_right_child_to(std::size_t parent_index, const SFTreeNode& child);
            void generate_codes();
            inline SFTreeNode& get_node_ref_from_index(std::size_t index) {
                assert(index < m_tree.size());

                return m_tree.at(index);
            }

            [[nodiscard]]
            inline std::size_t nodes_count() const { return m_tree.size(); }
            inline std::size_t nodes_count() { return m_tree.size(); }

            [[nodiscard]]
            inline auto begin() const noexcept { return m_tree.cbegin(); }
            inline auto begin() noexcept { return m_tree.begin(); }

            [[nodiscard]]
            inline auto end() const noexcept { return m_tree.cend(); }
            inline auto end() noexcept { return m_tree.end(); }
    };


    class ShannonFano: public CompressionAlgorithm {
        private:
            int a;
        public:
            void compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) override;
    };
}

template <>
struct std::formatter<compadre::ShannonFanoTree> : std::formatter<std::string_view> {
    auto format(const compadre::ShannonFanoTree& tree, std::format_context& ctx) const {
        std::string temp;

        std::format_to(std::back_inserter(temp),
                "Tree (size={}): ", tree.nodes_count());

        for (auto& node: tree) {
            if (node.has_content_of_type<compadre::Symbol<char>>()) {
                auto node_symbol = node.get_content<compadre::Symbol<char>>().value().m_symbol;
                std::format_to(std::back_inserter(temp),
                        " (node={}, symb='{}')", node.index().value(), node_symbol);
            }

            if (node.has_content_of_type<compadre::SFTreeNode::BranchNode>()) {
                std::format_to(std::back_inserter(temp),
                        " (node={}, 'branch')", node.index().value());
            }
        }
        return std::formatter<string_view>::format(temp, ctx);
    }
};

