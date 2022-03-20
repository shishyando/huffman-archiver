#pragma once

#include <string>
#include <vector>

class ArgumentsProcessing {
public:
    enum class ParsingResult { Error, Help, Encode, Decode };

    static void ShowHelp(bool full);
    ArgumentsProcessing(int argc, char* argv[]);
    ArgumentsProcessing();

private:
    bool CheckFiles();
    bool CheckFile(const std::string& file_name);

public:
    std::vector<std::string> files;
    std::string archive_name;
    std::string error_message;
    ParsingResult parsing_result;
};
