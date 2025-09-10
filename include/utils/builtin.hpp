#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include <string>
#include <vector>

namespace BUILTIN {
inline const std::vector<std::string> BUILTIN_FUNC = {"println",
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

inline const std::vector<std::string> BUILTIN_CLASS        = {"Object"};
// inline const std::vector<std::string> BUILTIN_CLASS_METHOD = {"Object_constructor"};
// inline const std::vector<std::vector<std::pair<std::string, bool>>> BUILTIN_CLASS_METHOD_PARAMS = {
//     {{"str", false}}};
// inline const std::vector<std::pair<std::string, bool>> BUILTIN_CLASS_METHOD_RETURN_TYPE = {
//     {"Object", true}};
}   // namespace BUILTIN

#endif
