//
// Created by cerussite on 19/06/15.
//

#include <boost/algorithm/searching/boyer_moore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <forward_list>
#include <fstream>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <bits/unordered_set.h>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "error.hpp"
#include "input.hpp"
#include "types/makefile.hpp"

namespace {
    make::Makefile loadInlineMakefile(const std::string &file,
                                      bool errorIgnored) {
        static std::unordered_map<std::string, make::Makefile> makefileCache;

        if (makefileCache.find(file) == std::end(makefileCache)) {
            if (fs::exists(fs::absolute(fs::path(file)))) {
                makefileCache[file] = make::lexer(make::loadMakefile(file));
            } else {
                if (errorIgnored) {
                    makefileCache[file] = make::Makefile();
                } else {
                    make::errorExit("Makefile not found: " + file, 3);
                }
            }
        }
        return makefileCache[file];
    }

    std::vector<std::string> parseSuffixes(const std::string &ss) {
        std::vector<std::string> suffixes(
            std::count(std::begin(ss), std::end(ss), '.'));

        auto suffixItr = std::begin(suffixes);

        const auto first = std::cbegin(ss), last = std::cend(ss);

        auto offset = ss.find('.');
        while (offset != std::string::npos) {
            auto endOffset = ss.find('.', offset + 1);

            *suffixItr = ss.substr(offset, endOffset - offset);

            offset = endOffset;
        }

        return suffixes;
    }
} // namespace

namespace make {
    std::string loadMakefile(const std::string &file) {
        using namespace std::literals::string_literals;
        using boost::algorithm::replace_regex;

        std::ifstream fin(file);

        std::string content{std::istreambuf_iterator<char>(fin),
                            std::istreambuf_iterator<char>()};
        replace_regex(content, boost::regex(R"(\\s*$)"), ""s);
        return content;
    }

    Makefile lexer(const std::string &makefile) {
        using namespace std::literals::string_literals;
        using boost::contains;
        using boost::starts_with;
        using boost::algorithm::replace_all_regex;
        using boost::algorithm::split;
        using boost::algorithm::split_regex;
        using boost::algorithm::trim_copy;

        std::unordered_set<std::string> suffixes;

        const auto isRegisteredSuffixes =
            [&suffixes](const std::vector<std::string> &targetSuffixes) {
                for (const auto &targetSuffix : targetSuffixes) {
                    if (suffixes.find(targetSuffix) == std::end(suffixes)) {
                        return false;
                    }
                }
                return true;
            };

        std::vector<Variable> variables;
        std::vector<Include> includes;
        std::vector<Rule> rules;

        std::forward_list<Command> commands;
        std::optional<Rule> currentRule;

        const auto appendRuleIfExists = [&currentRule, &rules, &commands] {
            if (currentRule.has_value()) {
                commands.reverse();
                rules.emplace_back(currentRule->targets(),
                                   currentRule->dependencies(), commands);
                currentRule.reset();
                commands.clear();
            }
        };

        std::istringstream iss(makefile);
        while (iss) {
            std::string line;
            std::getline(iss, line);

            {
                auto sharp = line.find('#');
                if (sharp != std::string::npos) {
                    line = line.substr(sharp);
                }
            }
            boost::algorithm::trim_right(line);

            if (std::empty(line)) {
                continue;
            }

            if (line[0] == '\t') {
                if (!currentRule.has_value()) {
                    errorExit(
                        "command line section must be in the target section",
                        1);
                }

                bool errorIgnored =
                    std::regex_match(line, std::regex(R"(^\s*@*-@*)"));
                bool noEchoCommand =
                    std::regex_match(line, std::regex(R"(^\s*-*@-*)"));

                std::smatch sm;
                std::regex_match(line, sm, std::regex(R"(^\s*[-@]*(.+))"));

                commands.emplace_front(errorIgnored, noEchoCommand,
                                       trim_copy(sm[1].str()));
                continue;

            } else {
                appendRuleIfExists();
            }

            if (line.find('=') != std::string::npos) {
                auto type = Variable::Type::Recursive;
                std::vector<std::string> operands;

                if (contains(line, "+=")) {
                    type = Variable::Type::Append;
                    split_regex(operands, line, boost::regex(R"(\+=)"));
                } else if (contains(line, ":=")) {
                    type = Variable::Type::Simple;
                    split_regex(operands, line, boost::regex(":="));
                } else if (contains(line, "?=")) {
                    type = Variable::Type::Optional;
                    split_regex(operands, line, boost::regex(R"(\?=)"));
                } else {
                    split(operands, line, boost::is_any_of("="));
                }
                std::vector<std::string> values;
                split_regex(values, trim_copy(operands[1]),
                            boost::regex(R"(\s+)"));

                variables.emplace_back(type, trim_copy(operands[0]),
                                       std::move(values));

            } else if (line.find(':') != std::string::npos) {
                std::vector<std::string> td;
                split(td, line, boost::is_any_of(":"));

                std::vector<std::string> targets;
                split_regex(targets, td[0], boost::regex(R"(\s+)"));
                std::vector<std::string> dependencies;
                split_regex(dependencies, td[1], boost::regex(R"(\s+)"));

                targets.erase(std::remove_if(std::begin(targets),
                                             std::end(targets),
                                             std::empty<std::string>),
                              std::end(targets));

                dependencies.erase(std::remove_if(std::begin(dependencies),
                                                  std::end(dependencies),
                                                  std::empty<std::string>),
                                   std::end(dependencies));
                if (std::size(targets) == 1) {
                    auto &target = targets[0];

                    if (target[0] == '.') {
                        if (target == ".SUFFIXES") {
                            suffixes.insert(std::begin(dependencies),
                                            std::end(dependencies));

                        } else {
                            auto targetSuffixes = parseSuffixes(target);
                            if (isRegisteredSuffixes(targetSuffixes)) {
                                dependencies.clear();
                                dependencies.emplace_back("%" +
                                                          targetSuffixes[0]);
                                if (std::size(targetSuffixes) == 1) {
                                    target = "%";
                                } else {
                                    target = "%" + targetSuffixes[1];
                                }
                            }
                        }
                    }
                }

                currentRule = Rule(targets, dependencies, {});

            } else if (contains(line, "include")) {
                auto trimmed = trim_copy(line);
                std::vector<std::string> spl;
                split_regex(spl, trimmed, boost::regex(R"(\s+)"));

                auto errorIgnored = starts_with(spl[0], "-");

                std::for_each(
                    std::begin(spl) + 1, std::end(spl),
                    [&](const std::string &file) {
                        auto makefile = loadInlineMakefile(file, errorIgnored);
                        variables.insert(std::end(variables),
                                         std::begin(makefile.variables()),
                                         std::end(makefile.variables()));
                        rules.insert(std::end(rules),
                                     std::begin(makefile.rules()),
                                     std::end(makefile.rules()));
                    });

            } else {
                errorExit("unrecognized line\n  " + line, 2);
            }
        }
        appendRuleIfExists();

        return Makefile(std::move(variables), std::move(includes),
                        std::move(rules));
    }
} // namespace make
