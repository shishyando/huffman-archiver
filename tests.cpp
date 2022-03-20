#include <iostream>
#include <map>

#include "ArgsProcessing.h"
#include "BitIO.h"
#include "catch.hpp"
#include "HuffmanTree.h"
#include "LeftistHeap.h"

TEST_CASE("Heap_test") {
    {
        using Person = std::pair<std::string, size_t>;
        auto cmp = [](const Person& a, const Person& b) { return a.second < b.second; };
        LeftistHeap<Person, decltype(cmp)> queue;
        REQUIRE(queue.Size() == 0);
        REQUIRE(queue.Empty());
        Person me("Shish", 1337);
        queue.Insert(me);
        REQUIRE(queue.Size() == 1);
        REQUIRE(!queue.Empty());
        REQUIRE(queue.Top() == me);
        REQUIRE(queue.Extract() == me);
        REQUIRE(queue.Empty());
    }
    {
        struct Info {
            size_t a;
            size_t b;

            Info(size_t a, size_t b) : a(a), b(b) {
            }

            bool operator<(const Info& other) const {
                return a + b < other.a + other.b;
            }

            bool operator==(const Info& other) const {
                return a == other.a && b == other.b;
            }
        };

        std::vector<Info> v = {{5, 5}, {2, 2}, {4, 1}, {5, 5}, {0, 1}};

        LeftistHeap<Info> queue(v);

        std::vector<Info> sorted_v;
        while (!queue.Empty()) {
            sorted_v.emplace_back(queue.Extract());
        }

        std::sort(v.begin(), v.end());

        REQUIRE(sorted_v == v);
    }
    std::cout << "Heap tests passed" << std::endl;
}

TEST_CASE("Command line arguments processing (errors)") {
    // decoding
    {  // no arguments passed
        std::vector<std::string> v_args = {"current_directory/archiver.exe"};
        int argc = 1;
        char* argv[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = v_args[i].data();
        }
        ArgumentsProcessing arg_proc(argc, argv);
        REQUIRE(arg_proc.parsing_result == ArgumentsProcessing::ParsingResult::Error);
        REQUIRE(arg_proc.error_message == "Error: No arguments passed");
    }
    {  // No archive name given to decode
        std::vector<std::string> v_args = {"current_directory/archiver.exe", "-d"};
        int argc = 2;
        char* argv[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = v_args[i].data();
        }
        ArgumentsProcessing arg_proc(argc, argv);
        REQUIRE(arg_proc.parsing_result == ArgumentsProcessing::ParsingResult::Error);
        REQUIRE(arg_proc.error_message == "Error: No archive name given to decode");
    }
    {  // too many files given to decode
        std::vector<std::string> v_args = {"current_directory/archiver.exe", "-d", "archive1", "archive_name2.qwerty"};
        int argc = 4;
        char* argv[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = v_args[i].data();
        }
        ArgumentsProcessing arg_proc(argc, argv);
        REQUIRE(arg_proc.parsing_result == ArgumentsProcessing::ParsingResult::Error);
        REQUIRE(arg_proc.error_message == "Error: Too many arguments given");
    }
    // encoding
    {  // not enough arguments passed
        std::vector<std::string> v_args = {"current_directory/archiver.exe", "-c", "ar"};
        int argc = 2;
        char* argv[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = v_args[i].data();
        }
        ArgumentsProcessing arg_proc(argc, argv);
        REQUIRE(arg_proc.parsing_result == ArgumentsProcessing::ParsingResult::Error);
        REQUIRE(arg_proc.error_message == "Error: Not enough arguments passed");
    }
    // invalid first opt
    {
        std::vector<std::string> v_args = {"current_directory/archiver.exe", "-e", "ar", "file", "file2"};
        int argc = 2;
        char* argv[argc];
        for (int i = 0; i < argc; ++i) {
            argv[i] = v_args[i].data();
        }
        ArgumentsProcessing arg_proc(argc, argv);
        REQUIRE(arg_proc.parsing_result == ArgumentsProcessing::ParsingResult::Error);
        REQUIRE(arg_proc.error_message == "Error: Incorrect first argument");
    }
    std::cout << "Command line arguments processing tests passed" << std::endl;
}

TEST_CASE("BitReader") {
    std::string test_file = "binary_IO_test";
    REQUIRE(CHAR_BIT == 8);
    {  // correct read
        // this test requires that standard type char contains exactly 8 bits
        std::ofstream out(test_file, std::ios::binary);
        char a = 0b00010101;
        char b = 0b00110100;
        char c = 0b00111011;
        out.put(a);
        out.put(b);
        out.put(c);
        out.close();

        std::ifstream in(test_file, std::ios::binary);
        BitReader bin_in(in);
        std::vector<bool> correct_bits_a = {0, 0, 0, 1, 0, 1, 0, 1};
        std::vector<bool> correct_bits_b = {0, 0, 1, 1, 0, 1, 0, 0};
        std::vector<bool> correct_bits_c = {0, 0, 1, 1, 1, 0, 1, 1};
        for (size_t i = 0; i < 8; ++i) {
            REQUIRE(bin_in.Get() == correct_bits_a[i]);
        }
        REQUIRE(bin_in.Get(8) == correct_bits_b);
        REQUIRE(bin_in.Get(3) == std::vector<bool>{0, 0, 1});
        REQUIRE(bin_in.Get(1) == std::vector<bool>{1});
        REQUIRE(bin_in.Get(4) == std::vector<bool>{1, 0, 1, 1});
    }
    {  // read EOF
        std::ofstream out(test_file, std::ios::binary);
        std::string test_str = "qw\ne";
        out << test_str;
        out.close();

        std::ifstream in(test_file, std::ios::binary);
        BitReader bin_in(in);
        std::vector<bool> bits = bin_in.Get(test_str.size() * CHAR_BIT);
        REQUIRE(bits == std::vector<bool>{0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1,
                                          0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1});
        REQUIRE_THROWS(bin_in.Get());
    }
    std::cout << "BitReader test passed" << std::endl;
}

TEST_CASE("BitWriter") {
    std::string test_file = "binary_IO_test";
    std::vector<bool> bits = {0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1};

    // this test requires that standard type char contains exactly 8 bits
    REQUIRE(CHAR_BIT == 8);

    // subtest 1
    {  // writing
        std::ofstream out(test_file, std::ios::binary);
        BitWriter bin_out(out);
        bin_out.Write(bits); // bits.size() = 26
        bin_out.Close();
    }
    {  // checking
        std::ifstream in(test_file, std::ios::binary);
        BitReader bin_in(in);
        std::vector<bool> read_bits;
        while (true) {  // reading until the EOF
            try {
                bool bit = bin_in.Get();
                read_bits.emplace_back(bit);
            } catch (std::runtime_error& e) {
                break;
            }
        }

        REQUIRE(read_bits != bits);
        while (bits.size() % 8 != 0) {
            bits.emplace_back(0);  // adding zeroes until size % 8 == 0
        }
        REQUIRE(bits.size() == 32);  // new size
        REQUIRE(read_bits == bits);
    }

    // subtest 2
    {  // writing
        std::ofstream out(test_file, std::ios::binary);
        BitWriter bin_out(out);
        bin_out.Write(10, 5);  // 01010
        bin_out.Write(15, 3);  // 111
        bin_out.Write({0});
    }
    {  // reading
        std::ifstream in(test_file, std::ios::binary);
        BitReader bin_in(in);
        bits = bin_in.Get(9);
        REQUIRE(bits == std::vector<bool>{0, 1, 0, 1, 0, 1, 1, 1, 0});
        REQUIRE_NOTHROW(bits = bin_in.Get(7));
        REQUIRE_THROWS_AS(bin_in.Get(), std::runtime_error);
    }
    std::cout << "BitWriter tests passed" << std::endl;
}

TEST_CASE("Huffman tree") {
    std::string file_name = "huffman_tree_test";
    std::map<char, std::string> string_codes = {
        {'a', "101"},
        {'b', "110"},
        {'c', "11111"},
        {'d', "11100"},
        {'e', "10010"},
        {'f', "11101"},
        {'g', "1000"},
        {'h', "0"},
    };

    Huffman::HuffmanTree huffman_tree;

    for (const auto& [chr, str] : string_codes) {
        std::vector<bool> code;
        for (char bit : str) {
            code.emplace_back(bit == '1');
        }
        huffman_tree.AddSymbol(Huffman::Symbol(chr), code);
    }

    std::string data = "defabacabaghghdef";
    std::vector<bool> data_bin;
    for (char c : data) {
        for (char bit : string_codes[c]) {
            data_bin.emplace_back(bit == '1');
        }
    }

    std::ofstream out(file_name, std::ios::binary);
    BitWriter bin_out(out);
    bin_out.Write(data_bin);
    bin_out.Close();

    auto code_lengths = huffman_tree.GetCodeLengthsForSymbols();
    for (const auto& [symbol, code_len] : code_lengths) {
        REQUIRE(string_codes[static_cast<char>(symbol.to_ullong())].size() == code_len);
    }

    std::ifstream in(file_name, std::ios::binary);
    BitReader bin_in(in);
    std::string decoded_data;
    while (decoded_data.size() < data.size()) {
        Huffman::Symbol new_symbol;
        REQUIRE_NOTHROW(new_symbol = huffman_tree.GetNextSymbol(bin_in));
        decoded_data += static_cast<char>(new_symbol.to_ullong());
    }
    bool tail_bit; // encoded data has 63 bits, which means that the BitWriter will add one tail bit to the file
    REQUIRE_NOTHROW(tail_bit = bin_in.Get());
    REQUIRE_THROWS(bin_in.Get());
    REQUIRE(tail_bit == 0);
    bin_in.Close();
    REQUIRE(decoded_data == data);
    std::cout << "Huffman tree tests passed" << std::endl;
}