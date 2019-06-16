//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_INPUT_HPP
#define MAKE_INPUT_HPP

#include "types/makefile.hpp"
#include <string>

namespace make {
    std::string loadMakefile(const std::string &file);
    Makefile lexer(const std::string &makefile);
} // namespace make

#endif // MAKE_INPUT_HPP
