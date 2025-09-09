#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include <string>
#include <vector>

namespace BUILTIN {
inline const std::vector<std::string> BUILTIN_FUNC = {
    "println",
    "print_int",
    "print_str",
    "print_float",
    "print_bool",
    "input",
    "_concat_strs",
    "int_to_str",
    "float_to_str",
    "bool_to_str",
    "str_to_int",
    "str_to_float",
    "str_to_bool",
};
}

#endif
