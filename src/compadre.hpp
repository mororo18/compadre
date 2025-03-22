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

    template<typename Model>
    concept StaticModel = requires(char symb) {
        { Model::probability_of(symb) } -> std::same_as<float>;
    };

    template<typename Model>
    concept AdaptativeModel = requires(char symb) {
        { Model::new_probability_of(symb) } -> std::same_as<float>;
    };

    class PreprocessedPortugueseText {
        private:
            std::string m_text;
        public:
            PreprocessedPortugueseText(const std::string&);
            inline const std::string& as_string() { return m_text; }
            static const std::array<char, 27> char_list;

            class StaticModel {
                public:
                    static std::unordered_map<char, float> char_frequencies;

                    inline
                    static auto probability_of(char symb) -> float {
                        return char_list[symb];
                    }
            };
    };

    template<typename T>
    struct Symbol {
        private:
            std::optional<T> m_symbol;
            std::optional<float> m_probability;
        public:
            using inner_type = T;
            Symbol() = default;
            Symbol(T symb)
                : m_symbol(symb)
            {
            }

            Symbol(T symb, float probability)
                : m_symbol(symb), m_probability(probability)
            {
            }

            void set_probability(float probability) {
                m_probability = probability;
            }

            std::optional<float> probability() {
                return m_probability;
            }

            std::optional<T> inner() {
                return m_symbol;
            }

            [[nodiscard]]
            std::optional<T> inner() const {
                return m_symbol;
            }

            bool is_unknown() {
                return !m_symbol.has_value();
            }

            [[nodiscard]]
            bool is_unknown() const {
                return !m_symbol.has_value();
            }

            bool has_probability() {
                return m_probability.has_value();
            }

        bool operator==(const Symbol& other) const {
            bool same_symbol = m_symbol.has_value() && !other.is_unknown();

            if (!is_unknown() && !other.is_unknown()) {
                same_symbol = m_symbol.value() == other.inner().value();
            }

            bool both_unknwon = is_unknown() && other.is_unknown();

            return same_symbol || both_unknwon;
        }
    };

    class CodeWord {
        public:
            std::bitset<32> m_bits;
            std::size_t m_bit_count;

            CodeWord() = default;
            inline void push_right_bit(Bit bit) {
                // Bounds checking
                m_bits.test(m_bit_count);
                m_bits <<= 1;
                m_bits[0] = bit;
                m_bit_count++;
            }

            inline void push_left_bit(Bit bit) {
                // Bounds checking
                m_bits.test(m_bit_count);
                m_bits[m_bit_count] = bit;
                m_bit_count++;
            }

            void reverse_valid_bits() {
                for (std::size_t i = 0; i < m_bit_count / 2; ++i) {
                    bool bit1 = m_bits[i];
                    bool bit2 = m_bits[m_bit_count - 1 - i];

                    m_bits[i] = bit2;
                    m_bits[m_bit_count - 1 - i] = bit1;
                }
            }

            inline std::size_t length() { return m_bit_count; }
    };

    template<typename SpecializedSymbol>
    concept ValidSymbol =
    std::same_as<SpecializedSymbol, Symbol<typename SpecializedSymbol::inner_type>>;

    template<ValidSymbol SpecializedSymbol>
    class Code {
        using T = SpecializedSymbol::inner_type;
        std::vector<std::pair<Symbol<T>, CodeWord>> m_code;

        public:
        auto get(Symbol<T> symb) -> std::optional<CodeWord> {
            for (auto& [symbol, cword] : m_code) {
                if (symb == symbol) {
                    return std::make_optional(cword);
                }
            }

            return std::nullopt;
        }

        void set(Symbol<T> symb, CodeWord code_word) {
            for (auto& [symbol, cword] : m_code) {
                if (symb == symbol) {
                    cword = code_word;
                    return;
                }
            }

            m_code.push_back(std::make_pair(symb, code_word));
        }
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


    template<typename Content>
    class CodeTreeNode {

        public:
            using content_type = Content;

            std::optional<Content> m_content;
            std::optional<Symbol<char>> m_symbol;
            std::optional<std::size_t> m_index;
            std::optional<std::size_t> m_left_index;
            std::optional<std::size_t> m_right_index;
            std::optional<std::size_t> m_parent_index;

            [[nodiscard]]
            inline bool is_empty() const { return !m_content.has_value(); }
            inline bool is_empty() { return !m_content.has_value(); }
            inline bool has_symbol() { return m_symbol.has_value(); }
            inline bool is_leaf() {
                return !m_left_index.has_value()
                    && !m_right_index.has_value();
            }

            CodeTreeNode(Content content) : m_content(std::move(content))
            {
            }

            inline void set_content(Content new_content) {
                m_content = std::move(new_content);
            }

            inline void clear_content() {
                m_content = std::nullopt;
            }

            [[nodiscard]]
            inline std::optional<std::size_t> index() const { return m_index; }
            inline std::optional<std::size_t> index() { return m_index; }

            [[nodiscard]]
            inline std::optional<Content> get_content() const { return m_content; }
            inline std::optional<Content> get_content() { return m_content; }
    };

    class BranchNode {};
    using SFTreeNodeContent = std::variant<Symbol<char>, SymbolList, BranchNode>;
    class SFTreeNode : public CodeTreeNode<SFTreeNodeContent> {
        public:

            SFTreeNode(SFTreeNodeContent content) : CodeTreeNode(content)
            {
            }

            static
            std::pair<SymbolList, SymbolList> slip_symbol_list(SymbolList& symb_list);

            template<typename ContentVariant>
            inline bool has_content_of_type() {
                return !is_empty() && std::holds_alternative<ContentVariant>(m_content.value());
            }

            template<typename ContentVariant>
            [[nodiscard]]
            inline bool has_content_of_type() const {
                return !is_empty() && std::holds_alternative<ContentVariant>(m_content.value());
            }

            template<typename ContentVariant>
            [[nodiscard]]
            inline std::optional<ContentVariant> get_content() const {
                if (is_empty()) {
                    return std::nullopt;
                }

                if (has_content_of_type<ContentVariant>()) {
                    return std::get<ContentVariant>(m_content.value());
                }

                return std::nullopt;
            }

            template<typename ContentVariant>
            inline std::optional<ContentVariant> get_content() {
                if (is_empty()) {
                    return std::nullopt;
                }

                if (has_content_of_type<ContentVariant>()) {
                    return std::get<ContentVariant>(m_content.value());
                }

                return std::nullopt;
            }

    };
}

namespace compadre {
    // A ValidTreeNode is a class that inherits CodeTreeNode<T>
    template <typename DerivedTreeNode>
    concept ValidTreeNode =
        std::is_base_of_v<CodeTreeNode<typename DerivedTreeNode::content_type>, DerivedTreeNode>;


    template <ValidTreeNode CodeTreeNode>
    class CodeTree {
        private:
            std::vector<CodeTreeNode> m_tree;
            SymbolList m_symb_list;
            //std::unordered_map<Symbol<char>, CodeWord> m_code;
            Code<Symbol<char>> m_code;
        public:
            static const Bit left_branch_bit = false;
            static const Bit right_branch_bit = true;
            CodeTree(SymbolList& symb_list);
            std::size_t push_node(const CodeTreeNode& node);
            std::size_t add_left_child_to(std::size_t parent_index, const CodeTreeNode& child);
            std::size_t add_right_child_to(std::size_t parent_index, const CodeTreeNode& child);
            auto get_code_map() -> Code<Symbol<char>>;
            inline CodeTreeNode& get_node_ref_from_index(std::size_t index) {
                assert(index < m_tree.size());

                return m_tree.at(index);
            }

            auto get_index_of_leaves() -> std::vector<std::size_t>;

            //inline auto symbol_from_codeword(CodeWord codeword) {
            //}

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

    template<ValidTreeNode CodeTreeNode>
    CodeTree<CodeTreeNode>::CodeTree(SymbolList& symb_list)
        : m_symb_list(symb_list)
    {
    }


    template <ValidTreeNode CodeTreeNode>
    std::size_t CodeTree<CodeTreeNode>::push_node(const CodeTreeNode& node) {
        assert(not node.is_empty());

        for (auto [node_index, tree_node]: std::views::enumerate(m_tree)) {
            assert(not tree_node.is_empty());
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

    template <ValidTreeNode CodeTreeNode>
    std::size_t CodeTree<CodeTreeNode>::add_right_child_to(std::size_t parent_index, const CodeTreeNode& child) {
        auto child_index = push_node(child); 

        auto& child_ref = get_node_ref_from_index(child_index);
        child_ref.m_parent_index = parent_index;

        auto& parent = get_node_ref_from_index(parent_index);
        parent.m_right_index = child_index;

        return child_index;
    }

    template <ValidTreeNode CodeTreeNode>
    std::size_t CodeTree<CodeTreeNode>::add_left_child_to(std::size_t parent_index, const CodeTreeNode& child) {
        auto child_index = push_node(child); 

        auto& child_ref = get_node_ref_from_index(child_index);
        child_ref.m_parent_index = parent_index;

        auto& parent = get_node_ref_from_index(parent_index);
        parent.m_left_index = child_index;

        return child_index;
    }

    template <ValidTreeNode CodeTreeNode>
    auto CodeTree<CodeTreeNode>::get_index_of_leaves() -> std::vector<std::size_t> {
        auto indexes = std::vector<std::size_t>();

        for (auto [node_index, tree_node]: std::views::enumerate(m_tree)) {
            if (tree_node.is_leaf()) {
                indexes.push_back(node_index);
            }
        }

        return indexes;
    }

    template <ValidTreeNode CodeTreeNode>
    auto CodeTree<CodeTreeNode>::get_code_map() -> Code<Symbol<char>> {

        auto code = Code<Symbol<char>>();
        auto leaf_nodes_indexes = get_index_of_leaves();

        for (auto node_index: leaf_nodes_indexes) {
            auto node = get_node_ref_from_index(node_index);
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
                auto parent = get_node_ref_from_index(parent_index);

                // Check if the current node its the right os the left child.
                if (parent.m_left_index.value() == current_node.index().value()) { // NOLINT(bugprone-unchecked-optional-access)
                    node_code_word.push_left_bit(this->left_branch_bit);
                }
                if (parent.m_right_index.value() == current_node.index().value()) { // NOLINT(bugprone-unchecked-optional-access)
                    node_code_word.push_left_bit(this->right_branch_bit);
                }

                current_node = parent;
            }

            assert(node.has_symbol());
            auto node_symb = node.m_symbol.value(); 
            //auto node_symb = node.m_symbol.value(); // NOLINT(bugprone-unchecked-optional-access)
            code.set(node_symb, node_code_word);
        }

        return code;
    }


    template <typename Algo>
    concept CodingAlgorithm = requires(SymbolList& symb_list) {
        {
            Algo::encode_symbol_list(symb_list)
        } -> std::same_as<Code<Symbol<char>>>;
    };

    class ShannonFano {
        public:
            static auto encode_symbol_list(SymbolList& symb_list) -> Code<Symbol<char>>;
            static auto generate_code_tree(SymbolList& symb_list) -> CodeTree<SFTreeNode>;
    };

    class Huffman {
        public:
            static auto encode_symbol_list(SymbolList& symb_list) -> Code<Symbol<char>>;
    };

    template <typename T>
    concept ProbabilityModel = StaticModel<T> || AdaptativeModel<T>;


    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    class Compressor
    {
        private:
            outbit::BitBuffer m_bitbuffer;

            template <StaticModel SModel>
            auto static_compression(PreprocessedPortugueseText& msg, SymbolList& symb_list) -> std::vector<u8>;

            template <StaticModel SModel>
            auto static_decompression(std::vector<u8>& data, SymbolList& symb_list) -> PreprocessedPortugueseText;
        public:
            auto compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) -> std::vector<u8>;
            auto decompress_preprocessed_portuguese_text(std::vector<u8>&) -> PreprocessedPortugueseText;

    };

    // TODO: usar um tipo generico iterável no lugar de PreprocessedPortugueseText
    // para a msg
    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    template <StaticModel SModel>
    auto Compressor<Model, CodingAlgo>::static_compression(PreprocessedPortugueseText& msg, SymbolList& symb_list) -> std::vector<u8> {

        for (auto& symb: symb_list) {
            symb.set_probability(SModel::probability_of(symb.inner().value()));
        }

        auto code = CodingAlgo::encode_symbol_list(symb_list);
        auto msg_lenght = uint32_t(msg.as_string().size());
        std::size_t total_bits{};

        // Buffer of compressed data
        auto outbuff = outbit::BitBuffer();
        // Write symb count in the first 4 bytes.
        outbuff.write(msg_lenght);
        for (char ch: msg.as_string()) {
            auto symb = Symbol(ch);

            auto code_word = code.get(symb).value();
            total_bits += code_word.length();
            // NOTE: We do this to make the decompression more efficient.
            code_word.reverse_valid_bits();
            auto bits_as_ullong = code_word.m_bits.to_ullong();

            outbuff.write_bits(bits_as_ullong, code_word.length());
        }

        auto bits_per_symb = float(total_bits) / float(msg.as_string().size());
        //std::println("bits per symb {}", bits_per_symb);
        //std::println("razao de comp {}", 5.0f / bits_per_symb);

        return outbuff.buffer();
    }

    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    auto Compressor<Model, CodingAlgo>::compress_preprocessed_portuguese_text(PreprocessedPortugueseText& text) -> std::vector<u8> {
        assert(text.as_string().size() < std::size_t(std::numeric_limits<uint32_t>::max)
                && "Input is too big.");

        auto symb_list = SymbolList();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto symb = Symbol(ch);
            symb_list.push_char_symbol(symb);
        }

        return this->static_compression<Model>(text, symb_list);
    }

    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    template <StaticModel SModel>
    auto Compressor<Model, CodingAlgo>::static_decompression(std::vector<u8>& data, SymbolList& symb_list) -> PreprocessedPortugueseText {
        for (auto& symbol: symb_list) {
            symbol.set_probability(SModel::probability_of(symbol.inner().value()));
        }

        auto tree = CodingAlgo::generate_code_tree(symb_list);
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
                    assert(decltype(tree)::right_branch_bit);
                    auto right_index = current_node.m_right_index.value();
                    current_node = tree.get_node_ref_from_index(right_index);
                } else {
                    assert(!decltype(tree)::left_branch_bit);
                    auto left_index = current_node.m_left_index.value();
                    current_node = tree.get_node_ref_from_index(left_index);
                }

                code_word.push_right_bit(current_bit);

                if (current_node.template has_content_of_type<Symbol<char>>()) {
                    symbol = current_node.template get_content<Symbol<char>>();
                    break;
                }
            }

            //assert(code.at(symbol.value()).m_bits == code_word.m_bits);
            decompressed_text += symbol.value().inner().value();
        }

        return {decompressed_text};
    }

    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    auto Compressor<Model, CodingAlgo>::decompress_preprocessed_portuguese_text(std::vector<u8>& data) -> PreprocessedPortugueseText {

        auto symb_list = SymbolList();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto symb = Symbol(ch);
            symb_list.push_char_symbol(symb);
        }

        return this->static_decompression<Model>(data, symb_list);
    }
}

template <>
struct std::formatter<compadre::CodeTree<compadre::SFTreeNode>> : std::formatter<std::string_view> {
    auto format(const compadre::CodeTree<compadre::SFTreeNode>& tree, std::format_context& ctx) const {
        std::string temp;

        std::format_to(std::back_inserter(temp),
                "Tree (size={}): ", tree.nodes_count());

        for (auto& node: tree) {
            if (node.has_content_of_type<compadre::Symbol<char>>()) {
                auto node_symbol = node.get_content<compadre::Symbol<char>>().value().inner().value();
                std::format_to(std::back_inserter(temp),
                        " (node={}, symb='{}')", node.index().value(), node_symbol);
            }

            if (node.has_content_of_type<compadre::BranchNode>()) {
                std::format_to(std::back_inserter(temp),
                        " (node={}, 'branch')", node.index().value());
            }
        }
        return std::formatter<string_view>::format(temp, ctx);
    }
};

