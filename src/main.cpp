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

    std::string firstArg  = argv[1];
    std::string extension = ".wm";

    if (firstArg == "--dir") {
        if (argc < 3) {
            std::cerr << "Error: No directory specified after --dir\n";
            printUsage(argv[0]);
            return 1;
        }
        std::string dirPath = argv[2];
        processDirectory(dirPath, extension);
    }
    else if (firstArg == "--files") {
        if (argc < 3) {
            std::cerr << "Error: No files specified after --files\n";
            printUsage(argv[0]);
            return 1;
        }

        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--ext") {
                i++;
                continue;
            }
            if (arg.substr(0, 2) == "--") {
                continue;
            }
            processFile(arg);
        }
    }
    else {
        processFile(firstArg);
    }

    return 0;
}