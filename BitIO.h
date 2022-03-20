#pragma once

#include <bitset>
#include <fstream>
#include <vector>

class BitReader {
public:
    explicit BitReader(std::ifstream& in);
    bool Get();
    std::vector<bool> Get(size_t size);
    void Close();
    ~BitReader();

private:
    std::ifstream& in_;
    std::vector<bool> buff_;  // buff is reversed (Big-endian)
};

class BitWriter {
public:
    explicit BitWriter(std::ofstream& out);
    void Write(size_t val, size_t len);
    void Write(const std::vector<bool>& bits);
    void Close();
    ~BitWriter();

private:
    void Write();
    std::vector<bool> buff_;
    std::ofstream& out_;
};