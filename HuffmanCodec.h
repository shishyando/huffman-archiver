#pragma once

#include <fstream>
#include <unordered_map>

#include "BitIO.h"
#include "HuffmanTree.h"

namespace Huffman {
class Coder {
public:
    explicit Coder(std::ofstream& out);
    void AddFile(const std::string& file_name);
    ~Coder();

private:
    void Reset();
    void MakeCanonicalCodes(std::unordered_map<Symbol, size_t>& symbol_freq);
    void MakeCanonicalCodes(std::vector<std::pair<Symbol, size_t>>& code_length_per_symbol);
    void Encode(const std::string& file_name);
    void Write(const Symbol& symbol);

private:
    BitWriter bin_out_;
    std::vector<Symbol> symbols_ordered_by_codes_;
    std::unordered_map<Symbol, Code> canonical_codes_;
    bool first_file_ = true;
    constexpr static const Symbol FILENAME_END = 256;
    constexpr static const Symbol ONE_MORE_FILE = 257;
    constexpr static const Symbol ARCHIVE_END = 258;
};

class Decoder {
public:
    explicit Decoder(std::ifstream& in);
    void Decode();
    Symbol GetNextSymbol(HuffmanTree& huffman_tree);

private:
    void Reset();
    size_t ReadAmount();
    bool DecodeFile(HuffmanTree& huffman_tree);

private:
    BitReader bin_in_;
    size_t symbols_count_;
    std::vector<Symbol> symbols_;
    std::unordered_map<Code, Symbol> canonical_codes_;
    constexpr static const Symbol FILENAME_END = 256;
    constexpr static const Symbol ONE_MORE_FILE = 257;
    constexpr static const Symbol ARCHIVE_END = 258;
};
}  // namespace Huffman
