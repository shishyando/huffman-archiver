#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "BitIO.h"

namespace Huffman {
const size_t BITS_IN_SYMBOL = 9;
using Symbol = std::bitset<BITS_IN_SYMBOL>;
using Code = std::vector<bool>;

class HuffmanTree {
public:
    HuffmanTree();
    explicit HuffmanTree(Symbol symbol);
    static HuffmanTree* Merge(HuffmanTree& left, HuffmanTree& right);
    [[nodiscard]] std::vector<std::pair<Symbol, size_t>> GetCodeLengthsForSymbols() const;
    void AddSymbol(Symbol symbol, Code code);
    Symbol GetNextSymbol(BitReader& bin_in);
    ~HuffmanTree();

private:
    struct Node {
        explicit Node(Symbol symbol);
        Symbol symbol = 0;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    constexpr static const Symbol INCORRECT_SYMBOL = 0b111'111'111;
    static void Dfs(Node* node, std::vector<std::pair<Symbol, size_t>>& code_lengths, size_t depth);
    void AddSymbol(Node* node, Symbol symbol, Code& code);
    static Symbol GetNextSymbol(Node* node, BitReader& bin_in);
    static void Delete(Node* node);

private:
    Node* root_ = nullptr;
};
}  // namespace Huffman