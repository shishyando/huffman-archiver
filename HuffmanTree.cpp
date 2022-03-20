#include "HuffmanTree.h"

namespace Huffman {
HuffmanTree::Node::Node(Symbol symbol) : symbol(symbol) {
}

HuffmanTree::HuffmanTree() {
    root_ = new Node(INCORRECT_SYMBOL);
}

HuffmanTree::HuffmanTree(Symbol symbol) {
    root_ = new Node(symbol);
}

HuffmanTree* HuffmanTree::Merge(HuffmanTree& left, HuffmanTree& right) {
    auto* new_tree = new HuffmanTree(INCORRECT_SYMBOL);
    new_tree->root_->left = left.root_;
    new_tree->root_->right = right.root_;
    return new_tree;
}

std::vector<std::pair<Symbol, size_t>> HuffmanTree::GetCodeLengthsForSymbols() const {
    std::vector<std::pair<Symbol, size_t>> codes_for_symbols;
    Dfs(root_, codes_for_symbols, 0);
    return codes_for_symbols;
}

void HuffmanTree::Dfs(Node* node, std::vector<std::pair<Symbol, size_t>>& code_lengths, size_t depth) {
    if (node == nullptr) {
        return;
    }
    if (node->left == nullptr && node->right == nullptr) {
        code_lengths.emplace_back(node->symbol, depth);
    }
    Dfs(node->left, code_lengths, depth + 1);
    Dfs(node->right, code_lengths, depth + 1);
}

void HuffmanTree::AddSymbol(Symbol symbol, Code code) {
    std::reverse(code.begin(), code.end());
    AddSymbol(root_, symbol, code);
}

void HuffmanTree::AddSymbol(Node* node, Symbol symbol, Code& code) {
    if (code.empty()) {
        if (node->symbol != INCORRECT_SYMBOL) {
            throw std::runtime_error("Error: Failed to add a symbol to the Huffman Tree");
        }
        node->symbol = symbol;
        return;
    }
    bool bit = code.back();
    code.pop_back();
    if (bit == 0) {
        if (node->left == nullptr) {
            node->left = new Node(INCORRECT_SYMBOL);
        }
        AddSymbol(node->left, symbol, code);
    } else {
        if (node->right == nullptr) {
            node->right = new Node(INCORRECT_SYMBOL);
        }
        AddSymbol(node->right, symbol, code);
    }
}

Symbol HuffmanTree::GetNextSymbol(BitReader& bin_in) {
    Symbol res = HuffmanTree::GetNextSymbol(root_, bin_in);
    if (res == INCORRECT_SYMBOL) {
        throw std::runtime_error("Error: The file is invalid, unable to find the symbol for encoded data");
    }
    return res;
}

Symbol HuffmanTree::GetNextSymbol(Node* node, BitReader& bin_in) {
    if (node == nullptr) {
        return INCORRECT_SYMBOL;
    }
    if (node->symbol != INCORRECT_SYMBOL) {
        return node->symbol;
    }
    auto bit = bin_in.Get();
    if (bit == 0) {
        return GetNextSymbol(node->left, bin_in);
    } else {
        return GetNextSymbol(node->right, bin_in);
    }
}

void HuffmanTree::Delete(HuffmanTree::Node* node) {
    if (node == nullptr) {
        return;
    }
    Delete(node->left);
    Delete(node->right);
    delete node;
}

HuffmanTree::~HuffmanTree() {
    Delete(root_);
}

}  // namespace Huffman