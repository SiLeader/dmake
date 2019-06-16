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
    bool isMatchToPattern(const std::string &target, std::string pattern) {
        boost::algorithm::replace_all(pattern, ".", "\\.");
        boost::algorithm::replace_all(pattern, "+", "\\+");
        boost::algorithm::replace_all(pattern, "*", "\\*");
        boost::algorithm::replace_all(pattern, "^", "\\^");
        boost::algorithm::replace_all(pattern, "|", "\\|");
        boost::algorithm::replace_all(pattern, "$", "\\$");
        boost::algorithm::replace_all(pattern, "\\", "\\\\");
        boost::algorithm::replace_all(pattern, "%", ".+");
        pattern = "^" + pattern + "$";

        return std::regex_match(target, std::regex(pattern));
    }

    void scheduleBuildImpl(std::shared_ptr<make::Phase> &phase,
                           const std::string &target,
                           const make::Makefile &makefile,
                           const make::DependGraph &dependGraph) {
        const auto &[graph, fileNames] = dependGraph;
        const auto targetIndex =
            std::find(std::begin(fileNames), std::end(fileNames), target) -
            std::begin(fileNames);

        std::vector<std::shared_ptr<make::Phase>> subPhases;

        for (const auto &edge : graph.out_edge_list(targetIndex)) {
            subPhases.emplace_back();
            const auto &file = fileNames[edge.get_target()];
            scheduleBuildImpl(subPhases.back(), file, makefile, dependGraph);
        }

        std::vector<make::Command> commands;
        if (makefile.hasRuleFor(target)) {
            for (const auto &command : makefile.rule(target).commands()) {
                // system(command.command().c_str());
                // std::cout << command.command() << std::endl;
                commands.emplace_back(command);
            }
        }
        phase = make::Phase::CreatePhase(commands, subPhases);
    }

    void runBuildImpl(const std::string &target, const make::Makefile &makefile,
                      const make::DependGraph &dependGraph) {
        const auto &[graph, fileNames] = dependGraph;
        const auto targetIndex =
            std::find(std::begin(fileNames), std::end(fileNames), target) -
            std::begin(fileNames);

        for (const auto &edge : graph.out_edge_list(targetIndex)) {
            const auto &file = fileNames[edge.get_target()];
            runBuildImpl(file, makefile, dependGraph);
        }

        if (makefile.hasRuleFor(target)) {
            for (const auto &command : makefile.rule(target).commands()) {
                system(command.command().c_str());
                // std::cout << command.command() << std::endl;
            }
        }
    }
} // namespace

namespace make {
    void runBuild(const std::string &target, const Makefile &makefile,
                  bool dryRun) {
        auto dg = make::createDependGraph(makefile.rules(), {});

        // runBuildImpl(target, makefile, dg);
        std::shared_ptr<make::Phase> phase;
        scheduleBuildImpl(phase, target, makefile, dg);
        phase->execute(dryRun);
    }
} // namespace make
