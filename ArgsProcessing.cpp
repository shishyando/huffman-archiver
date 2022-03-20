#include "ArgsProcessing.h"

#include <filesystem>
#include <iostream>

void ArgumentsProcessing::ShowHelp(bool full = true) {
    if (full) {
        std::cout << "archiver -c archive_name file1 [file2 ...]" << std::endl
                  << std::endl
                  << ""
                     "\tcreates an archive with name *archive_name* in the current directory"
                  << std::endl
                  << ""
                     "\tthe archive contains all listed files *file1, file2, ...*"
                  << std::endl
                  << ""
                     "\tthe files should be in the same directory as the archiver"
                  << std::endl
                  << std::endl
                  << ""
                     ""
                     "archiver -d archive_name"
                  << std::endl
                  << std::endl
                  << ""
                     "\textracts the files from *archive_name* in the current directory"
                  << std::endl
                  << ""
                     "\tthe files' names stay unchanged"
                  << std::endl
                  << std::endl
                  << ""
                     ""
                     "archiver -h"
                  << std::endl
                  << std::endl
                  << ""
                     "\t shows this message"
                  << std::endl;

    } else {
        std::cout << "archiver -h" << std::endl
                  << ""
                     "\t shows all features of the archiver"
                  << std::endl;
    }
}

ArgumentsProcessing::ArgumentsProcessing(int argc, char* argv[]) {
    if (argc == 1) {
        parsing_result = ParsingResult::Error;
        error_message = "Error: No arguments passed";
        return;
    }
    std::string opt = std::string(argv[1]);
    if (opt == "-h") {
        parsing_result = ParsingResult::Help;
    } else if (opt == "-d") {
        if (argc == 2) {
            parsing_result = ParsingResult::Error;
            error_message = "Error: No archive name given to decode";
            return;
        } else if (argc > 3) {
            parsing_result = ParsingResult::Error;
            error_message = "Error: Too many arguments given";
            return;
        }

        parsing_result = ParsingResult::Decode;
        archive_name = argv[2];
        if (!CheckFile(archive_name)) {
            return;
        }
    } else if (opt == "-c") {
        if (argc <= 3) {
            parsing_result = ParsingResult::Error;
            error_message = "Error: Not enough arguments passed";
            return;
        }

        parsing_result = ParsingResult::Encode;
        archive_name = argv[2];

        for (int i = 3; i < argc; ++i) {
            files.emplace_back(argv[i]);
        }

        if (!CheckFiles()) {
            return;
        }
    } else {
        parsing_result = ParsingResult::Error;
        error_message = "Error: Incorrect first argument";
    }
}

ArgumentsProcessing::ArgumentsProcessing() {
    parsing_result = ParsingResult::Help;
    ShowHelp();
}

bool ArgumentsProcessing::CheckFiles() {
    return std::ranges::all_of(files, [&](const std::string& file) { return CheckFile(file); });
}

bool ArgumentsProcessing::CheckFile(const std::string& file) {
    if (!std::filesystem::exists(file)) {
        parsing_result = ParsingResult::Error;
        error_message = "Error: File " + file + " does not exist in the current directory";
        return false;
    }
    return true;
}
