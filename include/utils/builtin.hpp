#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include <string>
#include <vector>

namespace BUILTIN {
inline const std::vector<std::string> BUILTIN_FUNC = {
    // io funcions
    "println",
    "print_int",
    "print_str",
    "print_float",
    "print_bool",
    "input",

    // public utils function
    "len",
    "int_to_str",
    "float_to_str",
    "bool_to_str",
    "str_to_int",
    "str_to_float",
    "str_to_bool",

    // builtin utils function
    "_concat_strs",
    "clone",

    // array mem function
    "_builtin_malloc",
    "_builtin_int_array_insert_impl",
    "_builtin_int_array_at_impl"};

inline const std::vector<std::string> BUILTIN_CLASS = {"Object", "String", "Range"};
// const std::
}   // namespace BUILTIN

namespace OBJECT_LAYOUT {
const int SIZE_OFFSET       = 1;
const int VTABLE_OFFSET     = 0;
const int BUILTIN_FIELD_NUM = 2;
const int VALID_OFFSET      = 2;


const int BUILTIN_METHOD_NUM = 3;
}   // namespace OBJECT_LAYOUT

#endif
