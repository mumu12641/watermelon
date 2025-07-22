#include "../../include/utils/process.hpp"

#include "../../include/ir/ir.hpp"
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


void processSourceFile(const std::string& source, const std::string& filename)
{
    std::string s =
        "Processing " + (filename.empty() ? "source code" : "'" + filename + "'") + ":\n";
    cout_yellow(s);

    cout_pink("  [1/4] Lexical analysis... ");
    Lexer lexer(source, filename);
    auto [tokens, lexerError] = lexer.tokenize();

    if (lexerError) {
        cout_red("Failed");
        std::cout << std::endl;
        lexerError->print();
        return;
    }
    cout_green("Passed");
    std::cout << std::endl;

    cout_pink("  [2/4] Syntax analysis...  ");
    Parser parser(tokens);
    auto [program, parserError] = parser.parse();

    if (parserError) {
        cout_red("Failed");
        std::cout << std::endl;
        parserError->print();
        return;
    }
    cout_green("Passed");
    // std::cout << program->dump() << std::endl;
    std::cout << std::endl;

    cout_pink("  [3/4] Semantic analysis... ");
    SemanticAnalyzer semanticAnalyzer(std::move(program));
    auto [resolveProgram, semanticError] = semanticAnalyzer.analyze();

    if (semanticError) {
        cout_red("Failed");
        std::cout << std::endl;
        semanticError->print();
        return;
    }
    cout_green("Passed");
    std::cout << resolveProgram->dump() << std::endl;
    std::cout << std::endl;

    cout_pink("  [4/4] LLVM IR generating... ");
    IRGen irGen(std::move(resolveProgram),
                std::move(semanticAnalyzer.getClassTable()),
                std::move(semanticAnalyzer.getFunctionTable()));
    auto  llvmIR = irGen.generateIR();
    cout_green("Passed");
    std::cout << std::endl;
    cout_blue("✓ Compilation successful");
    std::cout << std::endl;
    // llvmIR->print(llvm::outs(), nullptr);
    std::cout << std::endl;

    std::filesystem::path inputPath(filename);
    std::string           outputFilename =
        "/home/pbb/code/watermelon/examples/" + inputPath.stem().string() + ".ll";
    std::error_code      EC;
    llvm::raw_fd_ostream outFile(outputFilename, EC);

    if (EC) {
        cout_red("Error opening output file: " + EC.message());
        std::cout << std::endl;
    }
    else {
        llvmIR->print(outFile, nullptr);
        outFile.close();
        cout_green("LLVM IR has been written to: " + outputFilename);
        std::cout << std::endl;
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

void processDirectory(const std::string& dirPath, const std::string& extension)
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
    cout_green("O K Z P W B F D P I X Z R M M\n\n");
}