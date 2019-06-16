//
// Created by cerussite on 19/06/15.
//

#include "build.hpp"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <regex>

#include "../dependencies.hpp"
#include "../variables.hpp"

namespace {
    void scheduleBuildImpl(std::shared_ptr<make::Phase> &phase, const std::string &target,
                           const make::Makefile &makefile, const make::DependGraph &dependGraph) {
        const auto &[graph, fileNames] = dependGraph;

        std::vector<std::shared_ptr<make::Phase>> subPhases;

        std::vector<make::Command> commands;
        std::optional<make::Rule> rule;
        if (makefile.hasRuleFor(target)) {
            rule = makefile.rule(target);
        } else if (makefile.hasPatternRuleFor(target)) {
            rule = makefile.patternRule(target);
        }

        if (rule.has_value()) {
            for (auto commandInfo : (*rule).commands()) {
                auto &command = commandInfo.command();
                {
                    const auto &dependencies = rule->dependencies();
                    if (std::size(dependencies))
                        boost::algorithm::replace_all(command, "$<", dependencies[0]);
                    boost::algorithm::replace_all(command, "$^",
                                                  boost::algorithm::join(dependencies, " "));
                }
                {
                    const auto &targets = rule->targets();
                    boost::algorithm::replace_all(command, "$@", targets[0]);
                }
                commands.emplace_back(commandInfo);
            }

            for (const auto &dep : rule->dependencies()) {
                subPhases.emplace_back();
                scheduleBuildImpl(subPhases.back(), dep, makefile, dependGraph);
            }
        }

        phase = make::Phase::CreatePhase(commands, subPhases);
    }
} // namespace

namespace make {
    void runBuild(const std::string &target, const Makefile &makefile, bool dryRun) {
        auto dg = make::createDependGraph(makefile.rules(), {});

        std::shared_ptr<make::Phase> phase;
        scheduleBuildImpl(phase, target, makefile, dg);
        phase->execute(dryRun);
    }
} // namespace make
