#pragma once

#include <outbit/BitBuffer.hpp>
#include <print>
#include <string>
#include <unordered_map>

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
        public:
            SymbolList() = default;
            void sort();
            void push_char_symbol(Symbol<char> symb);
        private:
            std::vector<Symbol<char>> m_list;
    };

    using u8 = outbit::u8;
    using Bit = bool;

    class SFTreeNode {
        class EmptyNode {};
        using NodeContent = std::variant<Bit, SymbolList, EmptyNode>;
        public:
            NodeContent m_content;
            std::optional<std::size_t> m_left_index;
            std::optional<std::size_t> m_right_index;
            std::optional<std::size_t> m_index;
            SFTreeNode(NodeContent content = EmptyNode{});

            template<typename T>
            [[nodiscard]]
            bool content_of_type() const;

            template<typename T>
            bool content_of_type();

            [[nodiscard]]
            inline bool is_empty() const { return content_of_type<EmptyNode>(); }
            inline bool is_empty() { return content_of_type<EmptyNode>(); }
    };

    class ShannonFanoTree {
        private:
            std::vector<SFTreeNode> m_tree;
        public:
            std::size_t push_node(const SFTreeNode& node);
            void add_left_child_to(SFTreeNode& node, const SFTreeNode& child);
            void add_right_child_to(SFTreeNode& node, const SFTreeNode& child);
    };

    class ShannonFano: public CompressionAlgorithm {
        private:
            int a;
        public:
            void compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) override;
    };

}
