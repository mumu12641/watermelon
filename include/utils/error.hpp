#ifndef ERROR_HPP
#define ERROR_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <optional>

struct Location
{
    int         line;
    int         column;
    std::string filename;

    Location(int l, int c, const std::string& fname = "")
        : line(l)
        , column(c)
        , filename(fname)
    {
    }
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
