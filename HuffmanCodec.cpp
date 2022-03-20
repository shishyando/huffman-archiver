#include "HuffmanCodec.h"

#include <filesystem>
#include <iostream>
#include <queue>
#include <tuple>

#include "LeftistHeap.h"

namespace Huffman {
Coder::Coder(std::ofstream& out) : bin_out_(out) {
}

void Coder::Reset() {
    canonical_codes_.clear();
    symbols_ordered_by_codes_.clear();
    first_file_ = false;
}

void Coder::AddFile(const std::string& file_name) {
    if (!first_file_) {
        if (!canonical_codes_.contains(ONE_MORE_FILE)) {
            throw std::runtime_error("Error: could not encode file because ONE_MORE_FILE canonical code was not found");
        }
        bin_out_.Write(canonical_codes_[ONE_MORE_FILE]);
    }
    Reset();
    std::unordered_map<Symbol, size_t> symbol_freq;

    for (auto c : file_name) {
        ++symbol_freq[Symbol(c)];
    }
    ++symbol_freq[FILENAME_END];

    if (!std::filesystem::exists(file_name)) {
        throw std::runtime_error("Error: No such file" + file_name);
    }
    std::ifstream in(file_name, std::ios::binary);
    while (in.peek() != EOF) {
        char c = in.get();
        ++symbol_freq[Symbol(c)];
    }
    in.close();

    ++symbol_freq[ONE_MORE_FILE];
    ++symbol_freq[ARCHIVE_END];
    MakeCanonicalCodes(symbol_freq);
    Encode(file_name);
}

void Coder::MakeCanonicalCodes(std::unordered_map<Symbol, size_t>& symbol_freq) {
    using WeightedTree = std::pair<size_t, HuffmanTree*>;
    auto cmp = [](const WeightedTree& a, const WeightedTree& b) { return a.first < b.first; };
    LeftistHeap<WeightedTree, decltype(cmp)> queue;
    for (const auto& [symbol, freq] : symbol_freq) {
        auto* new_tree = new HuffmanTree(symbol);
        queue.Insert({freq, new_tree});
    }

    while (queue.Size() > 1) {
        auto a = queue.Extract();
        auto b = queue.Extract();
        auto new_tree = HuffmanTree::Merge(*a.second, *b.second);
        queue.Insert({a.first + b.first, new_tree});
    }

    auto code_lengths_per_symbol = queue.Top().second->GetCodeLengthsForSymbols();
    if (code_lengths_per_symbol.empty()) {
        throw std::runtime_error("Error: Failed to get canonical codes for symbols");
    }
    MakeCanonicalCodes(code_lengths_per_symbol);
}

void Coder::MakeCanonicalCodes(std::vector<std::pair<Symbol, size_t>>& code_length_per_symbol) {
    std::sort(code_length_per_symbol.begin(), code_length_per_symbol.end(), [&](const auto& a, const auto& b) {
        auto a_symbol = a.first.to_ullong();
        auto b_symbol = b.first.to_ullong();
        return std::tie(a.second, a_symbol) < std::tie(b.second, b_symbol);
    });
    uint64_t code = 0;
    for (size_t i = 0; i < code_length_per_symbol.size(); ++i) {
        size_t len = code_length_per_symbol[i].second;
        Symbol symbol = code_length_per_symbol[i].first;

        symbols_ordered_by_codes_.emplace_back(symbol);

        {
            uint64_t code_copy = code;
            for (size_t bit = 0; bit < len; ++bit) {
                canonical_codes_[symbol].emplace_back((code_copy & 1) == 1);
                code_copy >>= 1;
            }
            std::reverse(canonical_codes_[symbol].begin(), canonical_codes_[symbol].end());
        }
        if (i + 1 < code_length_per_symbol.size()) {
            size_t next_len = code_length_per_symbol[i + 1].second;
            code = (code + 1) << (next_len - len);
        }
    }
}

void Coder::Write(const Symbol& symbol) {
    bin_out_.Write(symbol.to_ullong(), BITS_IN_SYMBOL);
}

void Coder::Encode(const std::string& file_name) {
    bin_out_.Write(symbols_ordered_by_codes_.size(), BITS_IN_SYMBOL);  // SYMBOLS_COUNT, 9 bits

    for (const auto& symbol : symbols_ordered_by_codes_) {  // Symbols in canonical codes order, each has 9 bits
        Write(symbol);
    }

    {  // the amount of symbols with each code length
        std::vector<size_t> count_symbols_with_code_len;
        for (const auto& [symbol, code] : canonical_codes_) {
            size_t len = code.size();
            while (count_symbols_with_code_len.size() < len) {
                count_symbols_with_code_len.emplace_back(0);
            }
            ++count_symbols_with_code_len[len - 1];
        }
        for (size_t symbols_amount : count_symbols_with_code_len) {
            bin_out_.Write(symbols_amount, BITS_IN_SYMBOL);
        }
    }

    for (char c : file_name) {  // encode file name
        bin_out_.Write(canonical_codes_[Symbol(c)]);
    }

    bin_out_.Write(canonical_codes_[FILENAME_END]);

    if (!std::filesystem::exists(file_name)) {
        throw std::runtime_error("Error: No such file " + file_name);
    }
    std::ifstream in(file_name, std::ios::binary);

    while (in.peek() != EOF) {  // encode file body
        char c = in.get();
        bin_out_.Write(canonical_codes_[Symbol(c)]);
    }
    in.close();
}

Coder::~Coder() {
    bin_out_.Write(canonical_codes_[ARCHIVE_END]);
}

Decoder::Decoder(std::ifstream& in) : bin_in_(in) {
    symbols_count_ = 0;
}

void Decoder::Reset() {
    symbols_count_ = 0;
    symbols_.clear();
    canonical_codes_.clear();
}

size_t Decoder::ReadAmount() {
    size_t res = 0;
    auto code = bin_in_.Get(BITS_IN_SYMBOL);
    for (size_t bit = 0; bit < BITS_IN_SYMBOL; ++bit) {
        res |= code[bit] << bit;
    }
    return res;
}

void Decoder::Decode() {
    bool files_ended = false;
    while (!files_ended) {
        Reset();

        symbols_count_ = ReadAmount();

        for (size_t i = 0; i < symbols_count_; ++i) {  // get symbols in the order of canonical codes
            Code bits = bin_in_.Get(BITS_IN_SYMBOL);
            Symbol symbol;
            for (size_t bit = 0; bit < BITS_IN_SYMBOL; ++bit) {
                symbol[bit] = bits[bit];
            }
            symbols_.emplace_back(symbol);
        }

        {  // find canonical codes for symbols
            size_t ptr = 0;
            size_t value = 0;
            for (size_t len = 1; ptr < symbols_count_; ++len) {
                size_t cnt_symbols_with_len = ReadAmount();
                for (size_t iteration = 0; iteration < cnt_symbols_with_len; ++iteration) {
                    size_t value_copy = value;
                    Code code;
                    for (size_t bit = 0; bit < len; ++bit) {
                        code.emplace_back(value_copy % 2);
                        value_copy >>= 1;
                    }
                    std::reverse(code.begin(), code.end());
                    if (ptr == symbols_count_) {
                        throw std::runtime_error("Error: too many symbols with some code length");
                    }
                    canonical_codes_[code] = symbols_[ptr++];
                    ++value;
                }
                value <<= 1;
            }
        }

        HuffmanTree huffman_tree;
        for (auto& [code, symbol] : canonical_codes_) {  // add all symbols with their codes to the Huffman tree
            huffman_tree.AddSymbol(symbol, code);
        }

        files_ended = DecodeFile(huffman_tree);
    }
}

Symbol Decoder::GetNextSymbol(HuffmanTree& huffman_tree) {
    Symbol symbol = huffman_tree.GetNextSymbol(bin_in_);
    return symbol;
}

bool Decoder::DecodeFile(HuffmanTree& huffman_tree) {
    Symbol symbol = GetNextSymbol(huffman_tree);

    std::string file_name;

    while (symbol != FILENAME_END) {
        char c = static_cast<char>(symbol.to_ullong());
        file_name += c;
        symbol = GetNextSymbol(huffman_tree);
    }

    std::ofstream out(file_name, std::ios::binary);
    symbol = GetNextSymbol(huffman_tree);

    while (symbol != ONE_MORE_FILE && symbol != ARCHIVE_END) {
        char c = static_cast<char>(symbol.to_ullong());
        out.put(c);
        symbol = GetNextSymbol(huffman_tree);
    }
    out.close();
    std::cout << "Decoded file " << file_name << std::endl;
    return symbol == ARCHIVE_END;
}
}  // namespace Huffman