#pragma once

#include <outbit/BitBuffer.hpp>
#include <print>
#include <string>
#include <unordered_map>
#include <utility>
#include <format>

namespace compadre {

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
        float m_propability;
    };

    class SymbolList {
        using vec_type = std::vector<Symbol<char>>;
        using iterator = vec_type::iterator;
        using const_iterator = vec_type::const_iterator;
        public:
            SymbolList() = default;
            void sort();
            bool is_sorted();
            void push_char_symbol(Symbol<char> symb);
            
            inline iterator begin() noexcept { return m_list.begin(); }
            [[nodiscard]]
            inline const_iterator cbegin() const noexcept { return m_list.cbegin(); }
            inline iterator end() noexcept { return m_list.end(); }
            [[nodiscard]]
            inline const_iterator cend() const noexcept { return m_list.cend(); }

            inline std::size_t size() { return m_list.size(); }
            inline Symbol<char> front() { return *m_list.begin(); }
        private:
            std::vector<Symbol<char>> m_list;
    };

    using u8 = outbit::u8;

    class SFTreeNode {
        public:
            class EmptyNode {};
            class BranchNode {};
            using NodeContent = std::variant<Symbol<char>, SymbolList, BranchNode, EmptyNode>;

            NodeContent m_content;
            std::optional<std::size_t> m_left_index;
            std::optional<std::size_t> m_right_index;
            std::optional<std::size_t> m_index;
            SFTreeNode(NodeContent content = EmptyNode{});

            static
            std::pair<SymbolList, SymbolList> slip_symbol_list(SymbolList& symb_list);

            template<typename T>
            bool has_content_of_type() {
                return std::holds_alternative<T>(m_content);
            }

            template<typename T>
            [[nodiscard]]
            bool has_content_of_type() const {
                return std::holds_alternative<T>(m_content);
            }

            [[nodiscard]]
            inline bool is_empty() const { return has_content_of_type<EmptyNode>(); }
            inline bool is_empty() { return has_content_of_type<EmptyNode>(); }

            template<typename T>
            [[nodiscard]]
            inline std::optional<T> get_content() const {
                if (has_content_of_type<T>()) {
                    return std::get<T>(m_content);
                }

                return std::nullopt;
            }

            template<typename T>
            inline std::optional<T> get_content() {
                if (has_content_of_type<T>()) {
                    return std::get<T>(m_content);
                }

                return std::nullopt;
            }

            inline void set_content(NodeContent new_content) {
                m_content = std::move(new_content);
            }

            inline std::size_t index() { return m_index.value(); }
    };

    class ShannonFanoTree {
        //private:
        public:
            std::vector<SFTreeNode> m_tree;
            ShannonFanoTree(SymbolList& symb_list);
            std::size_t push_node(const SFTreeNode& node);
            std::size_t add_left_child_to(std::size_t parent_index, const SFTreeNode& child);
            std::size_t add_right_child_to(std::size_t parent_index, const SFTreeNode& child);
            void generate_codes();
            inline SFTreeNode& get_node_ref_from_index(std::size_t index) {
                assert(index < m_tree.size());

                return m_tree.at(index);
            }

            void print();

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
                "Tree (size={}): ", tree.m_tree.size());

        for (auto& node: tree.m_tree) {
            if (node.has_content_of_type<compadre::Symbol<char>>()) {
                auto node_symbol = node.get_content<compadre::Symbol<char>>().value().m_symbol;
                std::format_to(std::back_inserter(temp),
                    " (node={}, symb='{}')", node.m_index.value(), node_symbol);
            }

            if (node.has_content_of_type<compadre::SFTreeNode::BranchNode>()) {
                std::format_to(std::back_inserter(temp),
                    " (node={}, 'branch')", node.m_index.value());
            }
        }
        return std::formatter<string_view>::format(temp, ctx);
    }
};
