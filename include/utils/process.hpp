#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "../lexer/token.hpp"

#include <string>
#include <vector>

void printTokens(const std::vector<Token>& tokens);

void processSourceFile(const std::string& source, const std::string& filename = "");

std::string readFile(const std::string& filepath);

void processFile(const std::string& filepath);

void processDirectory(const std::string& dirPath, const std::string& extension = ".wm");

void printUsage(const char* programName);
#endif