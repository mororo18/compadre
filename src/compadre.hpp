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
        { Model::occurencies_of(symb) } -> std::same_as<uint32_t>;
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
                    static auto occurencies_of(char symb) -> uint32_t {
                        return uint32_t(char_list[symb] * 1000.0);
                    }
            };
    };

    template<typename InnerType, typename Attribute>
    struct Symbol {
        private:
            std::optional<InnerType> m_symbol;
            std::optional<Attribute> m_attribute;
        public:
            using inner_type = InnerType;
            using attribute_type = Attribute;

            Symbol() = default;
            Symbol(InnerType symb)
                : m_symbol(symb)
            {
            }
            Symbol(InnerType symb, Attribute att)
                : m_symbol(symb), m_attribute(att)
            {
            }

            void set_attribute(Attribute att) {
                m_attribute = att;
            }

            std::optional<Attribute> attribute() {
                return m_attribute;
            }

            [[nodiscard]]
            std::optional<Attribute> attribute() const {
                return m_attribute;
            }

            bool has_attribute() {
                return m_attribute.has_value();
            }

            std::optional<InnerType> inner() {
                return m_symbol;
            }

            [[nodiscard]]
            std::optional<InnerType> inner() const {
                return m_symbol;
            }

            bool is_unknown() {
                return !m_symbol.has_value();
            }

            [[nodiscard]]
            bool is_unknown() const {
                return !m_symbol.has_value();
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

    template<typename SpecializedSymbol>
    concept ValidSymbol =
    std::same_as<SpecializedSymbol,
        Symbol<typename SpecializedSymbol::inner_type, typename SpecializedSymbol::attribute_type>>;


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

    template<ValidSymbol SpecializedSymbol>
    class Code {
        std::vector<std::pair<SpecializedSymbol, CodeWord>> m_code;

        public:
        auto get(SpecializedSymbol symb) -> std::optional<CodeWord> {
            for (auto& [symbol, cword] : m_code) {
                if (symb == symbol) {
                    return std::make_optional(cword);
                }
            }

            return std::nullopt;
        }

        void set(SpecializedSymbol symb, CodeWord code_word) {
            for (auto& [symbol, cword] : m_code) {
                if (symb == symbol) {
                    cword = code_word;
                    return;
                }
            }

            m_code.push_back(std::make_pair(symb, code_word));
        }
    };

    template<ValidSymbol SpecializedSymbol>
    class SymbolList {
        using symbol_type = SpecializedSymbol;
        public:
            SymbolList() = default;
            void sort_by_attribute();
            bool is_sorted();
            void push(SpecializedSymbol symb);
            void remove(const SpecializedSymbol& symb);
            void remove_at(std::size_t index);
            bool contains(SpecializedSymbol symb);
            
            [[nodiscard]]
            inline auto cbegin() const noexcept { return m_list.cbegin(); }
            inline auto begin() noexcept { return m_list.begin(); }

            [[nodiscard]]
            inline auto cend() const noexcept { return m_list.cend(); }
            inline auto end() noexcept { return m_list.end(); }

            inline std::size_t size() { return m_list.size(); }
            inline SpecializedSymbol front() { return *m_list.begin(); }
        private:
            std::vector<SpecializedSymbol> m_list;
    };

    template<ValidSymbol SpecializedSymbol>
    bool SymbolList<SpecializedSymbol>::contains(SpecializedSymbol symb) {
        for (auto [index, symbol]: std::views::enumerate(m_list)) {
            if (symbol == symb) {
                return true;
            }
        }

        return false;
    }

    template<ValidSymbol SpecializedSymbol>
    void SymbolList<SpecializedSymbol>::push(SpecializedSymbol symb) {
        m_list.push_back(symb);
    }

    template<ValidSymbol SpecializedSymbol>
    void SymbolList<SpecializedSymbol>::remove(const SpecializedSymbol& symb) {
        std::optional<std::size_t> found_index = std::nullopt;
        for (auto [index, symbol]: std::views::enumerate(m_list)) {
            if (symbol == symb) {
                found_index = index;
                break;
            }
        }

        if (found_index.has_value()) {
            // it is not the last one
            if (found_index.value() < m_list.size()-1) {
                m_list.at(found_index.value()) = m_list.back();
            }

            m_list.pop_back();
        }
    }

    template<ValidSymbol SpecializedSymbol>
    void SymbolList<SpecializedSymbol>::remove_at(std::size_t index) {
        // Bounds checking
        void(m_list.at(index));
        m_list.erase(m_list.begin() + index);
    }

    // TODO: write test
    template<ValidSymbol SpecializedSymbol>
    void SymbolList<SpecializedSymbol>::sort_by_attribute() {
        std::ranges::sort(m_list,
            [](const SpecializedSymbol a, const SpecializedSymbol b) {
                auto a_attr = a.attribute().value();
                auto b_attr = b.attribute().value();
                return a_attr < b_attr;
            });
    }

    template<ValidSymbol SpecializedSymbol>
    bool SymbolList<SpecializedSymbol>::is_sorted() {
        return std::ranges::is_sorted(m_list,
            [](SpecializedSymbol a, SpecializedSymbol b) {
                return a.attribute().value() < b.attribute().value();
            });
    }

    template<typename Content, ValidSymbol SpecializedSymbol>
    class CodeTreeNode {

        public:
            using content_type = Content;
            using symbol_type = SpecializedSymbol;

            std::optional<Content> m_content;
            std::optional<SpecializedSymbol> m_symbol;
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

            CodeTreeNode(Content content, SpecializedSymbol symbol)
                : m_content(std::move(content)), m_symbol(std::move(symbol))
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
            inline std::optional<SpecializedSymbol> symbol() const { return m_symbol; }
            inline std::optional<SpecializedSymbol> symbol() { return m_symbol; }

            [[nodiscard]]
            inline std::optional<Content> get_content() const { return m_content; }
            inline std::optional<Content> get_content() { return m_content; }
    };

    class BranchNode {};
    using SFSymbol = Symbol<char, uint32_t>;
    using SFTreeNodeContent = std::variant<SFSymbol, SymbolList<SFSymbol>, BranchNode>;
    class SFTreeNode : public CodeTreeNode<SFTreeNodeContent, SFSymbol> {
        public:

            SFTreeNode(SFTreeNodeContent content) : CodeTreeNode(content)
            {
            }

            static
            std::pair<SymbolList<SFSymbol>, SymbolList<SFSymbol>> slip_symbol_list(SymbolList<SFSymbol>& symb_list);

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
        std::is_base_of_v<CodeTreeNode<typename DerivedTreeNode::content_type, typename DerivedTreeNode::symbol_type>, DerivedTreeNode>;


    template <ValidTreeNode CodeTreeNode>
    class CodeTree {
        using symbol_type =  CodeTreeNode::symbol_type;
        private:
            std::vector<CodeTreeNode> m_tree;
            Code<symbol_type> m_code;
        public:
            static const Bit left_branch_bit = false;
            static const Bit right_branch_bit = true;
            CodeTree() = default;
            CodeTree(const CodeTreeNode& root);
            std::size_t push_node(const CodeTreeNode& node);
            std::size_t add_left_child_to(std::size_t parent_index, const CodeTreeNode& child);
            std::size_t add_right_child_to(std::size_t parent_index, const CodeTreeNode& child);
            auto get_code_map() -> Code<symbol_type>;
            inline CodeTreeNode& get_node_ref_from_index(std::size_t index) {
                assert(index < m_tree.size());

                return m_tree.at(index);
            }

            auto get_index_of_leaves() -> std::vector<std::size_t>;

            inline
            auto root() -> CodeTreeNode {
                return m_tree.at(0);
            }

            [[nodiscard]]
            auto root() const -> CodeTreeNode {
                return m_tree.at(0);
            }

            static
            auto merge(const CodeTree& left, const CodeTree& right) -> CodeTree;

            static
            auto greater_than(const CodeTree& left, const CodeTree& right) -> bool;



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
    CodeTree<CodeTreeNode>::CodeTree(const CodeTreeNode& root)
    {
        push_node(root);
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
    auto CodeTree<CodeTreeNode>::get_code_map() -> Code<symbol_type> {

        auto code = Code<symbol_type>();
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
    struct SymbolType {
        using type = typename Algo::symbol_type;
    };

    template <typename Algo>
    struct SymbolListType {
        using type = typename Algo::symbol_list_type;
    };

    template <typename Algo, typename SymbolList = SymbolListType<Algo>::type>
    concept CodingAlgorithm = requires(SymbolList& symb_list) {
        {
            Algo::encode_symbol_list(symb_list)
        } -> std::same_as<Code<typename SymbolType<Algo>::type>>;
        typename Algo::symbol_type;
        typename Algo::symbol_type::inner_type;
        typename Algo::symbol_type::attribute_type;

        typename Algo::symbol_list_type;
    } && std::same_as<
            Symbol<typename Algo::symbol_type::inner_type, typename Algo::symbol_type::attribute_type>,
            typename Algo::symbol_type
    > && std::same_as<SymbolList, typename Algo::symbol_list_type>;

    template<typename Model>
    concept AdaptativeModel =
        requires(
            Model model,
            typename Model::symbol_type symb,
            SymbolList<typename Model::symbol_type> symb_list
        )
    {
        { model.occurencies_of(symb) } -> std::same_as<SymbolList<typename Model::symbol_type>>;
        { Model(symb_list) } -> std::same_as<Model>;
    };

    template<ValidSymbol Symbol>
    class Context {
        private:
            SymbolList<Symbol> m_inner;
            SymbolList<Symbol> m_symbols;
            std::size_t m_size;
        public:

            Context(SymbolList<Symbol>& ctx_symbols)
                : m_inner(ctx_symbols)
            {
                m_size = ctx_symbols.size();
            }

            std::size_t size() {
                return m_size;
            }
    };

    template<ValidSymbol Symbol, std::size_t MaxK>
    class PPM {
        std::array<std::vector<Context<Symbol>>, MaxK + 1> m_contexts_lists;
        SymbolList<Symbol> m_eq_prob_list;
        SymbolList<Symbol> m_symbols;


        public:
            using symbol_type = Symbol;

            PPM(SymbolList<Symbol>& symb_list)
                // : m_symbols(symb_list)
            {
                //m_symbols = SymbolList<Symbol>();
                for (auto& symb: symb_list) {
                    auto symbol = Symbol(symb.inner().value(), 1);
                    m_symbols.push(symbol);
                }

                m_eq_prob_list = m_symbols;
            }

            auto find_symbol_ctx(Symbol& symbol) -> std::optional<Context<Symbol>> {
                for (auto [ctx_size, ctx_list]: std::views::enumerate(m_contexts_lists) | std::views::reverse) {
                    std::println("ctx size {}", ctx_size);
                        
                    
                    for (auto& ctx: ctx_list) {
                    }
                }

                return std::nullopt;
            }

            auto occurencies_of(Symbol& symbol) -> SymbolList<Symbol> {
                auto ret = SymbolList<Symbol>();
                // TODO: Comecar pelo K = -1
                // x procura pelo symbolo nos contextos em ordem decrescente de tamanho
                auto symb_ctx = find_symbol_ctx(symbol);

                if (!symb_ctx.has_value()) {
                    assert(m_eq_prob_list.contains(symbol));
                    ret = m_eq_prob_list;
                    //m_eq_prob_list.remove(symbol);


                    // Add symbol to Ctx K=0
                }

                // ctx = bb , symb  = c
                // k = 2            k = 1
                // bb -> c          b -> c

                // x atualiza a tabela
                // x atualiza o contexto atual
                // x retorna a lista de simbolos com os contadores previos à atualização
                return ret;
            }

            void assert_contexts() {
                for (std::size_t k = 0; k < MaxK; ++k) {
                    for (const auto& context : m_contexts_lists[k]) {
                        assert(context.size() == k);
                    }
                }
            }
    };

        
    class ShannonFano {
        public:
            using symbol_type = SFSymbol;
            using symbol_list_type = SymbolList<SFTreeNode::symbol_type>;
            static auto encode_symbol_list(SymbolList<symbol_type>& symb_list) -> Code<symbol_type>;
            static auto generate_code_tree(SymbolList<symbol_type>& symb_list) -> CodeTree<SFTreeNode>;
    };

    using HuffmanSymbol = Symbol<char, uint32_t>;
    class HuffmanNode : public CodeTreeNode<uint32_t, HuffmanSymbol> {
        public:
            using symbol_type = HuffmanSymbol;
            HuffmanNode(uint32_t counter)
                : CodeTreeNode(counter)
            {
            }

            HuffmanNode(uint32_t counter, symbol_type symbol)
                : CodeTreeNode(counter, symbol)
            {
            }
    };

    // TODO: Write test!!
    template<>
    inline
    auto CodeTree<HuffmanNode>::merge(const CodeTree& left, const CodeTree& right) -> CodeTree
    {
        /*
        auto print_node = [](HuffmanNode node) {
            auto node_symb = node.symbol().has_value()
                ?
                node.symbol().value().is_unknown()
                    ?
                    std::string("rho")
                    :
                    std::string(1, node.symbol().value().inner().value())
                :
                "Branch";
            std::println("({}\t{}) pai={}, index={}, left={}, right={}",
                    node_symb, node.get_content().value(),
                    node.m_parent_index.has_value() ? std::to_string(node.m_parent_index.value()) : std::string("None"),
                    node.m_index.has_value() ? std::to_string(node.m_index.value()) : std::string("None"),
                    node.m_left_index.has_value() ? std::to_string(node.m_left_index.value()) : std::string("None"),
                    node.m_right_index.has_value() ? std::to_string(node.m_right_index.value()) : std::string("None")
            );
        };
        */

        auto merged = CodeTree();
        auto left_branch = left;
        auto right_branch = right;

        auto root_counter = left.root().get_content().value()
                            + right.root().get_content().value();
        auto new_root = HuffmanNode(root_counter);
        auto root_index = merged.push_node(new_root);
        auto merged_count = merged.nodes_count();

        // Left branch
        //std::println("Left branch");
        for (auto node: left_branch) {
            //std::print("Antes\t");
            //print_node(node);

            if (node.m_parent_index.has_value()) {
                // Not the root
                assert(node.m_index.value() != 0);
                node.m_parent_index = merged_count + node.m_parent_index.value();
            }

            if (node.m_left_index.has_value()) {
                node.m_left_index = merged_count + node.m_left_index.value();
            }

            if (node.m_right_index.has_value()) {
                node.m_right_index = merged_count + node.m_right_index.value();
            }

            if (node.m_index.value() == 0) {
                node.m_parent_index = root_index;
            }

            //std::print("Dps\t");
            //print_node(node);

            if (node.m_index.value() == 0) {
                merged.add_left_child_to(root_index, node);
            } else {
                merged.push_node(node);
            }
        }

        // Update the total number of nodes after the
        // partial merge
        merged_count = merged.nodes_count();

        // Right branch
        //std::println("Right branch");
        for (auto node: right_branch) {
            //std::print("Antes\t");
            //print_node(node);

            if (node.m_parent_index.has_value()) {
                // Not the root
                assert(node.m_index.value() != 0);
                node.m_parent_index = merged_count + node.m_parent_index.value();
            }

            if (node.m_left_index.has_value()) {
                node.m_left_index = merged_count + node.m_left_index.value();
            }

            if (node.m_right_index.has_value()) {
                node.m_right_index = merged_count + node.m_right_index.value();
            }

            if (node.m_index.value() == 0) {
                node.m_parent_index = root_index;
            }

            //std::print("Dps\t");
            //print_node(node);

            if (node.m_index.value() == 0) {
                merged.add_right_child_to(root_index, node);
            } else {
                merged.push_node(node);
            }
        }

        //std::println("Result!!");
        /*
        for (auto node: merged) {
            print_node(node);
        }
        */

        return merged;
    }

    template<>
    inline
    auto CodeTree<HuffmanNode>::greater_than(const CodeTree& a_tree, const CodeTree& b_tree) -> bool {
        auto a = a_tree.root();
        auto b = b_tree.root();
        auto a_counter = a.get_content().value();
        auto b_counter = b.get_content().value();

        auto a_has_symbol = a.symbol().has_value();
        auto b_has_symbol = b.symbol().has_value();
        auto both_have_symbol = a_has_symbol && b_has_symbol;

        if (a_counter != b_counter) {
            return a_counter > b_counter;
        }

        if (both_have_symbol) {

            auto a_unknown_symbol = a.symbol().value().is_unknown();
            auto b_unknown_symbol = b.symbol().value().is_unknown();

            if (a_unknown_symbol || b_unknown_symbol) {
                return a_unknown_symbol > b_unknown_symbol;
            }

            auto a_symbol_raw = a.symbol().value().inner().value();
            auto b_symbol_raw = b.symbol().value().inner().value();

            // Ordem alfabetica (lexicografica)
            return b_symbol_raw > a_symbol_raw;
        }

        return a_has_symbol > b_has_symbol;
    }

    class Huffman {
        public:
            using symbol_type = HuffmanNode::symbol_type;
            using symbol_list_type = SymbolList<HuffmanNode::symbol_type>;
            static auto encode_symbol_list(SymbolList<symbol_type>& symb_list) -> Code<symbol_type>;
            static auto generate_code_tree(SymbolList<symbol_type>& symb_list) -> CodeTree<HuffmanNode>;
    };

    template <typename T>
    concept ProbabilityModel = AdaptativeModel<T> || StaticModel<T>;


    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
        //requires CodingAlgorithm<CodingAlgo, typename CodingAlgo::symbol_list_type>
    class Compressor
    {
        private:
            outbit::BitBuffer m_bitbuffer;

            template <StaticModel SModel>
            auto static_compression(PreprocessedPortugueseText& msg, SymbolListType<CodingAlgo>::type& symb_list) -> std::vector<u8>;

            template <StaticModel SModel>
            auto static_decompression(std::vector<u8>& data, SymbolListType<CodingAlgo>::type& symb_list) -> PreprocessedPortugueseText;

            template <AdaptativeModel AModel>
            auto adaptative_compression(PreprocessedPortugueseText& msg, SymbolListType<CodingAlgo>::type& symb_list) -> std::vector<u8>;
        public:
            auto compress_preprocessed_portuguese_text(PreprocessedPortugueseText&) -> std::vector<u8>;
            auto decompress_preprocessed_portuguese_text(std::vector<u8>&) -> PreprocessedPortugueseText;

    };

    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    template <AdaptativeModel AModel>
    auto Compressor<Model, CodingAlgo>::adaptative_compression(PreprocessedPortugueseText& msg, SymbolListType<CodingAlgo>::type& symb_list) -> std::vector<u8> {
        auto prob_model = AModel(symb_list);

        // Buffer of compressed data
        auto outbuff = outbit::BitBuffer();
        // TODO: Isso precisa ser feitor posteriormente
        // Write symb count in the first 4 bytes.
        // outbuff.write(msg_lenght);
        std::size_t total_bits{};

        //std::println("adaptativoo");
        for (char ch: msg.as_string()) {
            auto symb = typename SymbolType<CodingAlgo>::type(ch);

            auto symb_list_to_encode = prob_model.occurencies_of(symb);
            auto code = CodingAlgo::encode_symbol_list(symb_list_to_encode); // Code

            auto code_word = code.get(symb).value(); // CodeWord
            total_bits += code_word.length();
            // NOTE: We do this to make the decompression easy.
            code_word.reverse_valid_bits();
            auto bits_as_ullong = code_word.m_bits.to_ullong();

            outbuff.write_bits(bits_as_ullong, code_word.length());

        }

        return outbuff.buffer();
    }

    // TODO: usar um tipo generico iterável no lugar de PreprocessedPortugueseText
    // para a msg
    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    template <StaticModel SModel>
    auto Compressor<Model, CodingAlgo>::static_compression(PreprocessedPortugueseText& msg, SymbolListType<CodingAlgo>::type& symb_list) -> std::vector<u8> {

        for (auto& symb: symb_list) {
            symb.set_attribute(SModel::occurencies_of(symb.inner().value()));
        }

        auto code = CodingAlgo::encode_symbol_list(symb_list);
        auto msg_lenght = uint32_t(msg.as_string().size());
        std::size_t total_bits{};

        // Buffer of compressed data
        auto outbuff = outbit::BitBuffer();
        // Write symb count in the first 4 bytes.
        outbuff.write(msg_lenght);
        for (char ch: msg.as_string()) {
            auto symb = typename SymbolType<CodingAlgo>::type(ch);

            auto code_word = code.get(symb).value();
            total_bits += code_word.length();
            // NOTE: We do this to make the decompression more efficient.
            code_word.reverse_valid_bits();
            auto bits_as_ullong = code_word.m_bits.to_ullong();

            outbuff.write_bits(bits_as_ullong, code_word.length());
        }

        // auto bits_per_symb = float(total_bits) / float(msg.as_string().size());
        //std::println("bits per symb {}", bits_per_symb);
        //std::println("razao de comp {}", 5.0f / bits_per_symb);

        return outbuff.buffer();
    }

    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    auto Compressor<Model, CodingAlgo>::compress_preprocessed_portuguese_text(PreprocessedPortugueseText& text) -> std::vector<u8> {
        assert(text.as_string().size() < std::size_t(std::numeric_limits<uint32_t>::max)
                && "Input is too big.");

        auto symb_list = typename CodingAlgo::symbol_list_type();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto symb = typename CodingAlgo::symbol_type(ch);
            symb_list.push(symb);
        }

        if constexpr (StaticModel<Model>) {
            return this->static_compression<Model>(text, symb_list);
        } if constexpr (AdaptativeModel<Model>) {
            return this->adaptative_compression<Model>(text, symb_list);
        }

        static_assert(ProbabilityModel<Model>);

    }

    template <ProbabilityModel Model, CodingAlgorithm CodingAlgo>
    template <StaticModel SModel>
    auto Compressor<Model, CodingAlgo>::static_decompression(std::vector<u8>& data, SymbolListType<CodingAlgo>::type& symb_list) -> PreprocessedPortugueseText {
        for (auto& symbol: symb_list) {
            symbol.set_attribute(SModel::occurencies_of(symbol.inner().value()));
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
            auto symbol = std::optional<typename CodingAlgo::symbol_type>();

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

                if (current_node.symbol().has_value()) {
                    symbol = current_node.symbol().value();
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

        auto symb_list = typename SymbolListType<CodingAlgo>::type();

        for (auto ch: PreprocessedPortugueseText::char_list) {
            auto symb = typename SymbolType<CodingAlgo>::type(ch);
            symb_list.push(symb);
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
            if (node.has_content_of_type<compadre::SFSymbol>()) {
                auto node_symbol = node.get_content<compadre::SFSymbol>().value().inner().value();
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

