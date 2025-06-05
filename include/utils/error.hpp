#ifndef ERROR_HPP
#define ERROR_HPP

#include "./format.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

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
        std::cerr << "\033[31merror: " << message << "\033[0m" << std::endl;
    }

    void printContext()
    {
        if (location) {
            int           line   = location->line;
            int           column = location->column;
            std::ifstream file(location->filename);

            std::string currentLine;
            int         currentLineNumber = 0;

            while (std::getline(file, currentLine)) {
                currentLineNumber++;

                if (currentLineNumber == line - 1) {
                    std::cerr << (line - 1) << " | " << currentLine << std::endl;
                }

                if (currentLineNumber == line) {
                    std::cerr << line << " | " << currentLine << std::endl;

                    std::cerr << std::string(column + std::to_string(line).length() + 3, ' ')
                              << "\033[31m^\033[0m " << std::endl;
                }

                if (currentLineNumber == line + 1) {
                    std::cerr << (line + 1) << " | " << currentLine << std::endl;
                    break;
                }
            }

            file.close();
        }
    }
};

#endif
