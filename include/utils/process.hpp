#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "../lexer/token.hpp"

#include <algorithm>
#include <string>
#include <vector>


void        collectLibFiles(const std::string& stdLibPath, const std::string& extension,
                               std::vector<std::string>& stdLibFiles);
void        collectDirectoryFiles(const std::string& dirPath, const std::string& extension,
                                  std::vector<std::string>& files);
std::string getLibPath(std::string name);
std::string readFile(const std::string& filepath);
void        processFiles(const std::vector<std::string>& stdLibFiles,
                         const std::vector<std::string>& userFiles);
void        printUsage(const char* programName);
void        printLogo();

#endif