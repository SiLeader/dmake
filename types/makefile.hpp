//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_MAKEFILE_HPP
#define MAKE_MAKEFILE_HPP

#include "../variables.hpp"
#include "include.hpp"
#include "rule.hpp"
#include "variable.hpp"
#include <unordered_map>
#include <vector>

namespace make {
    class Makefile {
    private:
        std::vector<Variable> _variables;
        std::vector<Include> _includes;
        std::vector<Rule> _rules;
        std::unordered_map<std::string, Rule> _ruleMap;
        std::unordered_map<std::string, std::vector<std::string>> _variableMap;

    public:
        Makefile()
            : Makefile({}, {}, {}) {}
        Makefile(std::vector<Variable> variables, std::vector<Include> includes,
                 std::vector<Rule> rules)
            : _variables(std::move(variables))
            , _includes(std::move(includes))
            , _rules(std::move(rules))
            , _variableMap(resolveVariables(_variables)) {
            for (auto &rule : _rules) {
                rule.assignVariablesToCommand(_variableMap);
                for (const auto &target : rule.targets()) {
                    _ruleMap[target] = rule;
                }
            }
        }

        Makefile(const Makefile &) = default;
        Makefile(Makefile &&) noexcept = default;
        Makefile &operator=(const Makefile &) = default;
        Makefile &operator=(Makefile &&) noexcept = default;

    public:
        const std::vector<Variable> &variables() const { return _variables; }

        const std::vector<Include> &includes() const { return _includes; }

        const std::vector<Rule> &rules() const { return _rules; }

        const Rule &rule(const std::string &target) const {
            return _ruleMap.at(target);
        }
        bool hasRuleFor(const std::string &target) const {
            return _ruleMap.find(target) != std::end(_ruleMap);
        }

        const std::vector<std::string> &
        variable(const std::string &name) const {
            return _variableMap.at(name);
        }
        bool hasVariable(const std::string &name) const {
            return _variableMap.find(name) != std::end(_variableMap);
        }
    };

} // namespace make

#endif // MAKE_MAKEFILE_HPP
