#include "../include/lexer/lexer.hpp"
#include "../include/lexer/token.hpp"
#include "../include/parser/parser.hpp"
#include "../include/utils/process.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    cout_pink("🎉Welcome to watermelon compiler!!\n");
    printLogo();
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    std::string              firstArg  = argv[1];
    std::string              extension = ".wm";
    std::vector<std::string> stdLibFiles;
    std::vector<std::string> userFiles;
    std::string              stdLibPath = getStdLibPath();
    collectStdLibFiles(stdLibPath, extension, stdLibFiles);
    
    if (firstArg == "--dir") {
        if (argc < 3) {
            std::cerr << "Error: No directory specified after --dir\n";
            printUsage(argv[0]);
            return 1;
        }
        std::string dirPath = argv[2];
        collectDirectoryFiles(dirPath, extension, userFiles);
        processFiles(stdLibFiles, userFiles);
    }
    else if (firstArg == "--files") {
        if (argc < 3) {
            std::cerr << "Error: No files specified after --files\n";
            printUsage(argv[0]);
            return 1;
        }
        for (int i = 2; i < argc; i++) {
            userFiles.push_back(argv[i]);
        }
        processFiles(stdLibFiles, userFiles);
    }
    else {
        userFiles.push_back(firstArg);
        processFiles(stdLibFiles, userFiles);
    }
    return 0;
}