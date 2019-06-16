//
// Created by cerussite on 19/06/15.
//

#include "types/variable.hpp"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    constexpr std::string_view PAREN_REGEX_STRING = R"(\$\((\w+)\))";
    constexpr std::string_view BRACE_REGEX_STRING = R"(\$\{(\w+)\})";

    std::vector<std::string> getVariableNames(const std::string &c) {
        static const std::regex PAREN{PAREN_REGEX_STRING.data(),
                                      PAREN_REGEX_STRING.size()};
        static const std::regex BRACE{BRACE_REGEX_STRING.data(),
                                      BRACE_REGEX_STRING.size()};

        std::size_t offset = 0;
        const auto first = std::begin(c), last = std::end(c);

        std::unordered_set<std::string> variables;

        std::smatch sm;
        while (std::regex_search(first + offset, last, sm, PAREN)) {
            variables.emplace(sm[0].str());
            offset += sm.position() + sm.size();
        }
        while (std::regex_search(first + offset, last, sm, BRACE)) {
            variables.emplace(sm[0].str());
            offset += sm.position() + sm.size();
        }

        return std::vector<std::string>(std::begin(variables),
                                        std::end(variables));
    }
} // namespace

namespace make {

    std::optional<std::string> getVariableName(const std::string &vn) {
        static const std::regex PAREN{PAREN_REGEX_STRING.data(),
                                      PAREN_REGEX_STRING.size()};
        static const std::regex BRACE{BRACE_REGEX_STRING.data(),
                                      BRACE_REGEX_STRING.size()};

        std::smatch sm;
        if (std::regex_match(vn, sm, PAREN)) {
            return sm[1].str();
        }
        if (std::regex_match(vn, sm, BRACE)) {
            return sm[1].str();
        }
        return std::nullopt;
    }
} // namespace make

namespace {
    std::vector<std::string>
    resolveVar(const std::vector<make::Variable> &vars) {
        std::vector<std::string> result;

        for (const auto &var : vars) {
            if (var.type() == make::Variable::Type::Append) {
                result.insert(std::end(result), std::begin(var.value()),
                              std::end(var.value()));
            } else {
                result = var.value();
            }
        }
        return result;
    }

    std::vector<std::string> resolveVariable(
        const std::string &value, const make::Variable::Type type,
        const std::unordered_map<std::string, std::vector<std::string>>
            &resolved,
        const std::unordered_map<std::string, std::vector<make::Variable>>
            &vars) {
        const auto varName = make::getVariableName(value);
        if (!varName.has_value()) {
            return {value};
        }

        const std::string &vn = varName.value();
        if (type == make::Variable::Type::Simple) {
            if (resolved.find(vn) != std::end(resolved)) {
                return resolved.at(vn);
            }
            return {""};
        }

        if (vars.find(vn) == std::end(vars)) {
            std::cerr << "undefined variable: " << vn << std::endl;
            std::exit(EXIT_FAILURE);
        }

        const auto &referenceVariables = vars.at(vn);
        return resolveVar(referenceVariables);
    }
} // namespace

namespace make {
    std::unordered_map<std::string, std::vector<std::string>>
    resolveVariables(const std::vector<Variable> &variables) {
        std::unordered_map<std::string, std::vector<Variable>> vars;
        for (const auto &variable : variables) {
            vars[variable.name()].emplace_back(variable);
        }

        std::vector<Variable *> unresolved;
        std::unordered_map<std::string, std::vector<std::string>> resolved;

        for (const auto &variable : variables) {
            std::vector<std::string> values;

            for (const auto &value : variable.value()) {
                const auto resolvedValue =
                    resolveVariable(value, variable.type(), resolved, vars);
                values.insert(std::end(values), std::begin(resolvedValue),
                              std::end(resolvedValue));
            }

            if (variable.type() == Variable::Type::Append) {
                auto &rv = resolved[variable.name()];
                rv.insert(std::end(rv), std::begin(values), std::end(values));
            } else {
                resolved[variable.name()] = std::move(values);
            }
        }

        return resolved;
    }

    std::string assignVariables(
        std::string command,
        const std::unordered_map<std::string, std::vector<std::string>>
            &variables) {
        const auto variableExpressions = getVariableNames(command);

        for (const auto &variableExpression : variableExpressions) {
            const auto variable = getVariableName(variableExpression);
            if (!variable)
                continue;
            if (variables.find(*variable) == std::end(variables))
                continue;

            const auto &value = variables.at(*variable);

            boost::algorithm::replace_all(command, variableExpression,
                                          boost::algorithm::join(value, " "));
        }

        return command;
    }
} // namespace make
