#include <iostream>

#include "ArgsProcessing.h"
#include "HuffmanCodec.h"

int main(int argc, char* argv[]) {
    ArgumentsProcessing arg_proc(argc, argv);
    ArgumentsProcessing::ParsingResult parsing_result = arg_proc.parsing_result;
    if (parsing_result == ArgumentsProcessing::ParsingResult::Error) {
        std::cerr << arg_proc.error_message << std::endl;
        ArgumentsProcessing::ShowHelp(false);
        return 1;
    } else if (parsing_result == ArgumentsProcessing::ParsingResult::Help) {
        ArgumentsProcessing::ShowHelp(true);
        return 0;
    }

    if (parsing_result == ArgumentsProcessing::ParsingResult::Encode) {
        std::cout << "Encoding..." << std::endl;
        try {
            std::ofstream out(arg_proc.archive_name, std::ios::binary);  // checked that the file exists
            Huffman::Coder coder(out);

            for (const auto& file : arg_proc.files) {
                coder.AddFile(file);
                std::cout << "Encoded file " << file << std::endl;
            }
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }

        std::cout << "Archive " << arg_proc.archive_name << " encoded successfully" << std::endl;

    } else {
        std::cout << "Decoding..." << std::endl;
        try {
            std::ifstream in(arg_proc.archive_name, std::ios::binary);
            Huffman::Decoder decoder(in);
            decoder.Decode();

        } catch (std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }

        std::cout << "Archive " << arg_proc.archive_name << " decoded successfully" << std::endl;
    }

    return 0;
}
