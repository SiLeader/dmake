//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_RULE_HPP
#define MAKE_RULE_HPP

#include <forward_list>
#include <iostream>
#include <string>
#include <vector>

#include "../variables.hpp"

namespace make {
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
            const std::unordered_map<std::string, std::vector<std::string>>
                &variableMap) {
            _command = make::assignVariables(_command, variableMap);
        }

    public:
        bool isErrorIgnored() const noexcept { return _errorIgnored; }
        bool isNoEchoCommand() const noexcept { return _noEchoCommand; }
        const std::string &command() const { return _command; }
    };

    class Rule {
    private:
        std::vector<std::string> _targets, _dependencies;
        std::forward_list<Command> _commands;

    public:
        Rule()
            : Rule({}, {}, {}) {}
        Rule(std::vector<std::string> targets,
             std::vector<std::string> dependencies,
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
            const std::unordered_map<std::string, std::vector<std::string>>
                &variableMap) {
            for (auto &command : _commands) {
                command.assignVariables(variableMap);
            }
        }

    public:
        const std::vector<std::string> &targets() const { return _targets; }

        const std::vector<std::string> &dependencies() const {
            return _dependencies;
        }

        const std::forward_list<Command> commands() const { return _commands; }
    };

} // namespace make

#endif // MAKE_RULE_HPP
