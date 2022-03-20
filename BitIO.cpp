#include "BitIO.h"

#include <fstream>

const size_t BITS_IN_CHAR = 8;

BitReader::BitReader(std::ifstream& in) : in_(in) {
}

bool BitReader::Get() {
    if (buff_.empty()) {
        if (in_.peek() == EOF) {
            throw std::runtime_error("Error: unexpected end of file");
        }

        char c = in_.get();
        for (size_t i = 0; i < BITS_IN_CHAR; ++i) {
            buff_.emplace_back(c & (1 << i));
        }
    }
    auto bit = buff_.back();
    buff_.pop_back();
    return bit;
}

std::vector<bool> BitReader::Get(size_t size) {
    std::vector<bool> res;
    while (res.size() < size) {
        res.emplace_back(Get());
    }
    return res;
}

void BitReader::Close() {
    in_.close();
}

BitReader::~BitReader() {
    Close();
}

BitWriter::BitWriter(std::ofstream& out) : out_(out) {
}

void BitWriter::Write(uint64_t val, size_t len) {
    std::vector<bool> bits;
    for (size_t i = 0; i < len; ++i) {
        bits.emplace_back(val % 2);
        val >>= 1;
    }
    Write(bits);
}

void BitWriter::Write(const std::vector<bool>& bits) {
    for (auto bit : bits) {
        buff_.emplace_back(bit);
        if (buff_.size() == BITS_IN_CHAR) {
            Write();
        }
    }
}

void BitWriter::Write() {
    char buff_char = 0;
    for (size_t i = 0; i < BITS_IN_CHAR; ++i) {
        buff_char |= buff_[i] << (BITS_IN_CHAR - i - 1);
    }
    out_.put(buff_char);
    buff_.clear();
}

void BitWriter::Close() {
    if (buff_.empty()) {
        out_.close();
        return;
    }
    while (buff_.size() < BITS_IN_CHAR) {
        buff_.emplace_back(0);
    }
    Write();
    out_.close();
}

BitWriter::~BitWriter() {
    Close();
}
