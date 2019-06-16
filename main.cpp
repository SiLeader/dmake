#include <iostream>

#include "argparse/argparse.hpp"
#include "build/build.hpp"
#include "dependencies.hpp"
#include "input.hpp"
#include "variables.hpp"
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>

int main(int argc, char **argv) {
    argparse::ArgumentParser parser("make", "build tool", "License: Apache License 2.0");
    parser.addArgument({"--makefile", "--file", "-f"}, "Makefile path");
    parser.addArgument({"--graph"}, "show dependency graph", argparse::ArgumentType::StoreTrue);
    parser.addArgument({"--dot"}, "generate dot file and dot file path");
    parser.addArgument({"target"}, "target name");
    parser.addArgument({"--dry-run"}, "dry run", argparse::ArgumentType::StoreTrue);

    auto args = parser.parseArgs(argc, argv, true);

    auto makeString = make::loadMakefile("Makefile");
    auto makefile = make::lexer(makeString);

    if (args.has("graph")) {
        auto dg = make::createDependGraph(makefile.rules(), {});
        make::printGraph(std::cout, dg);
        return 0;

    } else if (args.has("dot")) {
        auto dg = make::createDependGraph(makefile.rules(), {});
        const auto &[graph, names] = dg;
        std::ofstream fout(args.get<std::string>("dot"));
        boost::write_graphviz(fout, graph, boost::make_label_writer(names.data()));

    } else {
        std::string defaultTarget(makefile.defaultTarget());

        const auto target = args.safeGet<std::string>("target", defaultTarget);
        make::runBuild(target, makefile, args.has("dry-run"));
    }
    return 0;
}
