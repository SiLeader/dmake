//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_RULE_HPP
#define MAKE_RULE_HPP

#include <boost/algorithm/string.hpp>
#include <forward_list>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "../variables.hpp"

namespace make {
    namespace detail {
        inline std::regex getPatternRegex(std::string pattern) {
            boost::algorithm::replace_all(pattern, "\\", "\\\\");
            boost::algorithm::replace_all(pattern, ".", "\\.");
            boost::algorithm::replace_all(pattern, "+", "\\+");
            boost::algorithm::replace_all(pattern, "*", "\\*");
            boost::algorithm::replace_all(pattern, "^", "\\^");
            boost::algorithm::replace_all(pattern, "|", "\\|");
            boost::algorithm::replace_all(pattern, "$", "\\$");
            boost::algorithm::replace_all(pattern, "%", "(.+)");
            pattern = "^" + pattern + "$";

            return std::regex(pattern);
        }
        inline bool isMatchToPattern(const std::string &target, std::string pattern) {
            return std::regex_match(target, getPatternRegex(std::move(pattern)));
        }
    } // namespace detail

    class Command {
    private:
        bool _errorIgnored;
        bool _noEchoCommand;
        std::string _command;

    public:
        Command(bool errorIgnored, bool noEchoCommand, std::string command)
            : _errorIgnored(errorIgnored)
            , _noEchoCommand(noEchoCommand)
            , _command(std::move(command)) {}

    public:
        void assignVariables(
            const std::unordered_map<std::string, std::vector<std::string>> &variableMap) {
            _command = make::assignVariables(_command, variableMap);
        }

    public:
        bool isErrorIgnored() const noexcept { return _errorIgnored; }
        bool isNoEchoCommand() const noexcept { return _noEchoCommand; }
        const std::string &command() const { return _command; }

        std::string &command() { return _command; }
    };

    class Rule {
    private:
        std::vector<std::string> _targets, _dependencies;
        std::forward_list<Command> _commands;

    public:
        Rule()
            : Rule({}, {}, {}) {}
        Rule(std::vector<std::string> targets, std::vector<std::string> dependencies,
             std::forward_list<Command> commands)
            : _targets(std::move(targets))
            , _dependencies(std::move(dependencies))
            , _commands(std::move(commands)) {}

        Rule(const Rule &) = default;
        Rule(Rule &&) = default;
        Rule &operator=(const Rule &) = default;
        Rule &operator=(Rule &&) = default;

    public:
        void assignVariablesToCommand(
            const std::unordered_map<std::string, std::vector<std::string>> &variableMap) {
            for (auto &command : _commands) {
                command.assignVariables(variableMap);
            }
            for (auto &target : _targets) {
                target = make::assignVariables(target, variableMap);
            }
            for (auto &dependency : _dependencies) {
                dependency = make::assignVariables(dependency, variableMap);
            }
        }

    public:
        bool isPatternRule() const {
            for (const auto &target : targets()) {
                if (boost::contains(target, "%"))
                    return true;
            }
            for (const auto &dependency : dependencies()) {
                if (boost::contains(dependency, "%"))
                    return true;
            }
            return false;
        }

        bool isPatternRuleMatchTo(const std::string &target) const {
            if (!isPatternRule())
                return false;
            for (const auto &ruleTarget : targets()) {
                if (detail::isMatchToPattern(target, ruleTarget)) {
                    return true;
                }
            }
            return false;
        }

        std::string targetToPlaceholder(const std::string &target) const {
            for (const auto &ruleTarget : targets()) {
                if (boost::contains(ruleTarget, "%")) {
                    const auto regex = detail::getPatternRegex(ruleTarget);
                    std::smatch sm;
                    if (std::regex_match(target, sm, regex)) {
                        return sm[1].str();
                    }
                }
            }
            throw std::runtime_error("dmake bug: targetToPlaceholder(target) must be "
                                     "isPatternRuleMatchTo(target) == true");
        }

    public:
        Rule setPlaceholder(const std::string &placeholder) const {
            std::vector<std::string> targets(std::size(_targets)),
                dependencies(std::size(_dependencies));

            const auto converter = [&placeholder](const std::string &file) {
                return boost::algorithm::replace_all_copy(file, "%", placeholder);
            };

            std::transform(std::begin(_targets), std::end(_targets), std::begin(targets),
                           converter);
            std::transform(std::begin(_dependencies), std::end(_dependencies),
                           std::begin(dependencies), converter);

            return Rule(targets, dependencies, _commands);
        }

    public:
        const std::vector<std::string> &targets() const { return _targets; }

        const std::vector<std::string> &dependencies() const { return _dependencies; }

        const std::forward_list<Command> commands() const { return _commands; }
    };

} // namespace make

#endif // MAKE_RULE_HPP
