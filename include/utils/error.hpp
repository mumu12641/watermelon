#ifndef ERROR_HPP
#define ERROR_HPP

#include "./format.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace Color {
const std::string RESET  = "\033[0m";
const std::string RED    = "\033[1;31m";
const std::string GREEN  = "\033[1;32m";
const std::string YELLOW = "\033[1;33m";
const std::string BLUE   = "\033[1;34m";
const std::string PINK   = "\033[1;35m";
const std::string CYAN   = "\033[1;36m";
}   // namespace Color

inline void cout_red(const std::string& s)
{
    std::cout << Color::RED << s << Color::RESET;
}

inline void cout_green(const std::string& s)
{
    std::cout << Color::GREEN << s << Color::RESET;
}

inline void cout_yellow(const std::string& s)
{
    std::cout << Color::YELLOW << s << Color::RESET;
}

inline void cout_blue(const std::string& s)
{
    std::cout << Color::BLUE << s << Color::RESET;
}

inline void cout_pink(const std::string& s)
{
    std::cout << Color::PINK << s << Color::RESET;
}

inline void cout_cyan(const std::string& s)
{
    std::cout << Color::CYAN << s << Color::RESET;
}

struct Location
{
    int         line     = 0;
    int         column   = 0;
    std::string filename = "";

    Location(int l, int c, const std::string& fname = "")
        : line(l)
        , column(c)
        , filename(fname)
    {
    }
    Location()
        : line(0)
        , column(0)
        , filename("")
    {
    }

    std::string to_string() { return Format("{0}:{1}:{2}", filename, line, column); }
};

struct Error
{
    std::string             message;
    std::optional<Location> location;

    Error(const std::string& msg, int l, int c, const std::string& fname = "")
        : message(msg)
        , location(Location(l, c, fname))
    {
    }
    Error(const std::string& msg, const Location& l)
        : message(msg)
        , location(l)
    {
    }

    Error(const std::string& msg)
        : message(msg)
    {
    }
    void print() const
    {
        if (location) {
            std::cerr << (location->filename.empty() ? "" : location->filename + ":")
                      << location->line << ":" << location->column << ": ";
        }
        cout_red(Format("error: {}\n", message));

        if (location) {
            int line   = location->line;
            int column = location->column;

            if (location->filename.empty() || !std::filesystem::exists(location->filename)) {
                return;
            }

            std::ifstream file(location->filename);
            if (!file.is_open()) {
                return;
            }

            std::string currentLine;
            int         currentLineNumber = 0;
            std::string lineNumberStr;

            while (std::getline(file, currentLine) && currentLineNumber < line + 1) {
                currentLineNumber++;

                if (currentLineNumber >= line - 1 && currentLineNumber <= line + 1) {
                    lineNumberStr = std::to_string(currentLineNumber);
                    std::cout << lineNumberStr << " | " << currentLine << std::endl;

                    if (currentLineNumber == line) {
                        int padding = column + lineNumberStr.length() + 3;   // +3 for " | "
                        std::cout << std::string(padding, ' ');
                        cout_red("^\n");
                    }
                }
            }

            file.close();
        }
    }
};


#endif
