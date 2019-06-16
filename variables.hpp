//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_VARIABLES_HPP
#define MAKE_VARIABLES_HPP

#include "types/variable.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace make {
    std::optional<std::string> getVariableName(const std::string &vn);
    std::unordered_map<std::string, std::vector<std::string>>
    resolveVariables(const std::vector<Variable> &variables);
    std::string assignVariables(
        std::string command,
        const std::unordered_map<std::string, std::vector<std::string>>
            &variables);
} // namespace make

#endif // MAKE_VARIABLES_HPP
