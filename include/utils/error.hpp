#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

struct Error
{
    std::string message;
    int         line;
    int         column;
    std::string filename;

    Error(const std::string& msg, int l, int c, const std::string& fname = "")
        : message(msg)
        , line(l)
        , column(c)
        , filename(fname)
    {
    }
};
#endif
