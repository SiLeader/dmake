//
// Created by cerussite on 19/06/16.
//

#ifndef MAKE_ERROR_HPP
#define MAKE_ERROR_HPP

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace make {
    [[noreturn]] inline void errorExit(std::string_view msg, int code) {
        std::cerr << "error: " << msg << std::endl;

        std::exit(code);
    }
} // namespace make

#endif // MAKE_ERROR_HPP
