#include "../../include/lexer/lexer.hpp"
#include "../../include/lexer/token.hpp"
#include "../../include/parser/parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void printContext(const std::string& filename, int line, int column)
{
    std::ifstream file(filename);

    std::string currentLine;
    int         currentLineNumber = 0;

    while (std::getline(file, currentLine)) {
        currentLineNumber++;

        if (currentLineNumber == line - 1) {
            std::cout << (line - 1) << " | " << currentLine << std::endl;
        }

        if (currentLineNumber == line) {
            std::cout << line << " | " << currentLine << std::endl;

            std::cout << std::string(column + std::to_string(line).length() + 3, ' ')
                      << "\033[31m^\033[0m " << std::endl;
        }

        if (currentLineNumber == line + 1) {
            std::cout << (line + 1) << " | " << currentLine << std::endl;
            break;
        }
    }

    file.close();
}
void printTokens(const std::vector<Token>& tokens)
{
    std::cout << "=== Tokens ===\n";
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
    std::cout << "==============\n\n";
}

void processSourceFile(const std::string& source, const std::string& filename = "")
{

    Lexer lexer(source, filename);
    auto [tokens, lexerError] = lexer.tokenize();
    if (lexerError) {
        std::cerr << (lexerError->filename.empty() ? "" : lexerError->filename + ":")
                  << lexerError->line << ":" << lexerError->column
                  << ": \033[31merror: " << lexerError->message << "\033[0m" << std::endl;
        printContext(filename, lexerError->line, lexerError->column);
        return;
    }

    Parser parser(tokens);
    auto   result = parser.parse();

    if (std::holds_alternative<ParseError>(result)) {
        auto error = std::get<ParseError>(result);
        std::cerr << (error.filename.empty() ? "" : error.filename + ":") << error.line << ":"
                  << error.column << ": \033[31merror: " << error.message << "\033[0m" << std::endl;
        printContext(filename, error.line, error.column);
    }
    else {
        auto program = std::move(std::get<std::unique_ptr<Program>>(result));
        if (program) {
            std::cout << "=== AST ===\n";
            std::cout << program->dump() << std::endl;
            std::cout << "===========\n";
        }
        else {
            std::cout << "Failed to parse program.\n";
        }
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