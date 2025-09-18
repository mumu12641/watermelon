#include "../../include/utils/process.hpp"

#include "../../include/ir/ir.hpp"
#include "../../include/lexer/lexer.hpp"
#include "../../include/lexer/token.hpp"
#include "../../include/parser/parser.hpp"
#include "../../include/semantic/semantic.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string getLibPath(std::string name)
{
    const char* homeDir = getenv("HOME");
    if (homeDir != nullptr) {
        std::string homePath = std::string(homeDir) + "/.watermelon/" + name;
        if (std::filesystem::exists(homePath)) {
            return homePath;
        }
    }
    return "";
}


void collectLibFiles(const std::string& stdLibPath, const std::string& extension,
                     std::vector<std::string>& stdLibFiles)
{
    if (!std::filesystem::exists(stdLibPath)) {
        cout_yellow("Warning: Standard library path does not exist: " + stdLibPath + "\n");
        return;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(stdLibPath)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            stdLibFiles.push_back(entry.path().string());
        }
    }

    std::sort(stdLibFiles.begin(), stdLibFiles.end());
}

void collectDirectoryFiles(const std::string& dirPath, const std::string& extension,
                           std::vector<std::string>& files)
{
    if (!std::filesystem::exists(dirPath)) {
        cout_red("Error: Directory does not exist: " + dirPath + "\n");
        return;
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files.push_back(entry.path().string());
        }
    }
    std::sort(files.begin(), files.end());
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

void processFiles(const std::vector<std::string>& stdLibFiles,
                  const std::vector<std::string>& userFiles)
{
    std::vector<std::string> filepaths;
    filepaths.reserve(stdLibFiles.size() + userFiles.size());
    filepaths.insert(filepaths.end(), stdLibFiles.begin(), stdLibFiles.end());
    filepaths.insert(filepaths.end(), userFiles.begin(), userFiles.end());

    std::vector<Token> tokens;
    cout_pink("  [1/6] Lexical analysis... ");
    for (int i = 0; i < filepaths.size(); i++) {
        auto        filename = filepaths[i];
        std::string source   = readFile(filename);
        Lexer       lexer(source, filename);
        auto [currTokens, lexerError] = lexer.tokenize();
        if (i != filepaths.size() - 1) {
            currTokens.pop_back();
        }
        if (lexerError) {
            cout_red("Failed");
            std::cout << std::endl;
            lexerError->print();
            return;
        }
        tokens.insert(tokens.end(), currTokens.begin(), currTokens.end());
    }
    cout_green("Passed");
    std::cout << std::endl;

    cout_pink("  [2/6] Syntax analysis...  ");
    Parser parser(tokens);
    auto [program, parserError] = parser.parse();
    if (parserError) {
        cout_red("Failed");
        std::cout << std::endl;
        parserError->print();
        return;
    }
    cout_green("Passed");
    std::cout << std::endl;

    cout_pink("  [3/6] Semantic analysis... ");
    SemanticAnalyzer semanticAnalyzer(std::move(program));
    auto [resolveProgram, semanticError] = semanticAnalyzer.analyze();
    if (semanticError) {
        cout_red("Failed");
        std::cout << std::endl;
        semanticError->print();
        return;
    }
    cout_green("Passed");
    std::cout << std::endl;

    // std::cout << resolveProgram->dump() << std::endl;

    cout_pink("  [4/6] LLVM IR generating... ");
    IRGen irGen(std::move(resolveProgram),
                std::move(semanticAnalyzer.getClassTable()),
                std::move(semanticAnalyzer.getFunctionTable()));
    auto  llvmIR = irGen.generateIR();
    cout_green("Passed");
    std::cout << std::endl;
    std::string          outputFilename = "./output.ll";
    std::error_code      EC;
    llvm::raw_fd_ostream outFile(outputFilename, EC);
    llvmIR->print(outFile, nullptr);
    outFile.close();

    cout_pink("  [5/6] Optimizing LLVM IR... ");
    std::string              optLibPath = getLibPath("opt");
    std::vector<std::string> optSoFiles;
    collectLibFiles(optLibPath, ".so", optSoFiles);
    std::string outputOptFilename = "./output_opt.ll";

    std::string optCmd = "opt -S ./output.ll -o " + outputOptFilename;
    for (const auto& soFile : optSoFiles) {
        optCmd += " -load-pass-plugin=" + soFile;
    }
    optCmd += " -passes=mem2reg-pass,constant-prop-pass,dce-pass";
    int optResult = system(optCmd.c_str());
    if (optResult != 0) {
        cout_red("Failed");
        std::cout << std::endl;
        cout_red("Error executing opt command: " + optCmd);
        std::cout << std::endl;
        return;
    }
    cout_green("Passed");
    std::cout << std::endl;

    cout_pink("  [6/6] Compiling LLVM IR to executable... ");
    std::vector<std::string> stdLibLLFiles;
    collectLibFiles(getLibPath("std"), ".ll", stdLibLLFiles);
    collectLibFiles(getLibPath("gc"), ".ll", stdLibLLFiles);
    std::string clangCmd = "clang++ -w -o ./output";
    for (const auto& llFile : stdLibLLFiles) {
        clangCmd += " " + llFile;
    }
    clangCmd += " " + outputOptFilename;
    int compileResult = system(clangCmd.c_str());
    if (compileResult != 0) {
        cout_red("Failed");
        std::cout << std::endl;
        cout_red("Error executing clang command: " + clangCmd);
        std::cout << std::endl;
        return;
    }
    cout_green("Passed");
    std::cout << std::endl;
    cout_blue("✓ Executable has been created: ./output");
    std::cout << std::endl;
}

void printUsage(const char* programName)
{

    std::string usage = "Usage:\n"
                        "  " +
                        std::string(programName) +
                        " <source_file>    Process a single file\n"
                        "  " +
                        std::string(programName) +
                        " --dir <directory>    Process all files in a directory\n"
                        "  " +
                        std::string(programName) +
                        " --files <file1> <file2> ...    Process multiple specific files\n";
    cout_yellow(usage);
}

void printLogo()
{
    // cout_green("A Q C                       A\n");
    cout_green("F G U A Z Y N Q J P W E E U K\n");
    cout_green("D M");
    cout_cyan(" D ");
    cout_red("C");
    cout_green("                     E\n");
    cout_green("D Z   ");
    cout_red("O L");
    cout_green("                   X\n");
    cout_green("Q");
    cout_cyan(" O ");
    cout_red("P R Q O");
    cout_green("                 I\n");
    cout_green("I   ");
    cout_red("C I O Z R");
    cout_green("               J\n");
    cout_green("Z   ");
    cout_red("M C P ");
    std::cout << "N";
    cout_red(" G M");
    cout_green("             G\n");
    cout_green("X   ");
    cout_red("O Q S A T K O");
    cout_green("           J\n");
    cout_green("U   ");
    cout_red("Y S A D M I Q S");
    cout_green("         H\n");
    cout_green("W   ");
    cout_red("K G T O W D K U N");
    cout_green("       A\n");
    cout_green("J");
    cout_cyan(" P ");
    cout_red("U G P B F D ");
    std::cout << "H";
    cout_red(" J ");
    std::cout << "O";
    cout_red(" S");
    cout_green("     R\n");
    cout_green("V H   ");
    cout_red("J Y V D Q C J P R B");
    cout_green("   G\n");
    cout_green("F G U   ");
    cout_red("Z Y N Q J P W E E U");
    cout_green(" K\n");
    cout_green("V N E Q N   ");
    cout_red("J C Q J W M");
    cout_cyan("   E");
    cout_green(" D\n");
    cout_green("S B A Y U ");
    cout_cyan("I           B");
    cout_green(" E J T\n");
    cout_green("O K Z P W B F D P I X Z R M M\n");
}