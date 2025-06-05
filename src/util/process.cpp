#include "../../include/lexer/lexer.hpp"
#include "../../include/lexer/token.hpp"
#include "../../include/parser/parser.hpp"
#include "../../include/semantic/semantic.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void processSourceFile(const std::string& source, const std::string& filename = "")
{

    Lexer lexer(source, filename);
    auto [tokens, lexerError] = lexer.tokenize();
    if (lexerError) {
        lexerError->print();
        lexerError->printContext();
        return;
    }

    Parser parser(tokens);
    auto [program, parserError] = parser.parse();

    if (parserError) {
        parserError->print();
        parserError->printContext();
        return;
    }

    SemanticAnalyzer semanticAnalyzer(std::move(program));
    auto [resolveProgram, semanticError] = semanticAnalyzer.analyze();
    if (semanticError) {
        semanticError->print();
        semanticError->printContext();
        return;
    }
}


std::string readFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void processFile(const std::string& filepath)
{
    std::string source = readFile(filepath);
    if (source.empty()) {
        return;
    }

    processSourceFile(source, filepath);
}

void processDirectory(const std::string& dirPath, const std::string& extension = ".wm")
{
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                processFile(entry.path().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

void printUsage(const char* programName)
{
    std::cerr << "Usage:\n"
              << "  " << programName << " <source_file>    Process a single file\n"
              << "  " << programName << " --dir <directory>    Process all files in a directory\n"
              << "  " << programName
              << " --files <file1> <file2> ...    Process multiple specific files\n";
}