//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_BUILD_HPP
#define MAKE_BUILD_HPP

#include "../types/makefile.hpp"
#include "../types/rule.hpp"
#include <memory>
#include <vector>

namespace make {
    class Phase {
    private:
        std::vector<Command> _commands;
        std::vector<std::shared_ptr<Phase>> _next;

    public:
        Phase(std::vector<Command> commands, std::vector<std::shared_ptr<Phase>> next)
            : _commands(std::move(commands))
            , _next(std::move(next)) {}

    public:
        static std::shared_ptr<Phase> CreatePhase(std::vector<Command> commands,
                                                  std::vector<std::shared_ptr<Phase>> next) {
            return std::make_shared<Phase>(std::move(commands), std::move(next));
        }

    public:
        bool execute(bool dryRun = false) const {
            for (const auto &p : _next) {
                if (!p->execute(dryRun))
                    return false;
            }

            for (std::size_t i = 0; i < std::size(_commands); ++i) {
                const auto &command = _commands[i];

                if (!command.isNoEchoCommand()) {
                    std::cout << command.command() << std::endl;
                }

                if (dryRun)
                    continue;

                if (system(command.command().c_str()) != 0) {
                    if (!command.isErrorIgnored()) {
                        return false;
                    }
                }
            }

            return true;
        }
    };

    void runBuild(const std::string &target, const Makefile &makefile, bool dryRun);
} // namespace make

#endif // MAKE_BUILD_HPP
