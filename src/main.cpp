#include "../include/lexer/lexer.hpp"
#include "../include/lexer/token.hpp"
#include "../include/parser/parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


void printTokens(const std::vector<Token>& tokens)
{
    std::cout << "=== Tokens ===\n";
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
    std::cout << "==============\n\n";
}

void testLexer(const std::string& source)
{
    std::cout << "Testing Lexer...\n";
    Lexer lexer(source);
    auto  tokens = lexer.tokenize();
    printTokens(tokens);
}

void testParser(const std::string& source)
{
    std::cout << "Testing Parser...\n";
    Lexer lexer(source);
    auto  tokens = lexer.tokenize();

    try {
        Parser parser(tokens);
        auto   program = parser.parse();

        if (program) {
            std::cout << "=== AST ===\n";
            std::cout << program->toString() << std::endl;
            std::cout << "===========\n";
        }
        else {
            std::cout << "Failed to parse program.\n";
        }
    }
    catch (const ParseError& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
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

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file> [--tokens-only]\n";
        return 1;
    }

    std::string filepath   = argv[1];
    bool        tokensOnly = (argc > 2 && std::string(argv[2]) == "--tokens-only");

    std::string source = readFile(filepath);
    if (source.empty()) {
        return 1;
    }

    if (tokensOnly) {
        testLexer(source);
    }
    else {
        testLexer(source);
        testParser(source);
    }

    return 0;
}