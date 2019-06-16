//
// Created by cerussite on 19/06/15.
//

#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <sstream>
#include <unordered_map>

#include "dependencies.hpp"
#include "types/rule.hpp"

namespace make {
    DependGraph createDependGraph(const std::vector<Rule> &rules,
                                  const std::vector<std::string> &files) {
        Graph graph;
        boost::property_map<Graph, boost::vertex_name_t>::type nameMap =
            boost::get(boost::vertex_name, graph);

        std::unordered_map<std::string, std::size_t> vertex;
        std::vector<std::string> vertexName;
        auto add_vertex = [&graph, &nameMap](const std::string &target) {
            auto vert = boost::add_vertex(graph);
            boost::put(nameMap, vert, target);
        };
        auto assignIfNotContains = [&vertex, &add_vertex,
                                    &vertexName](const std::string &test) {
            static std::size_t index = 0;
            if (vertex.find(test) == std::end(vertex)) {
                vertex[test] = index++;
                add_vertex(test);
                vertexName.emplace_back(test);
            }
        };

        for (const auto &rule : rules) {
            for (const auto &target : rule.targets()) {
                assignIfNotContains(target);
                for (const auto &dependency : rule.dependencies()) {

                    assignIfNotContains(dependency);

                    boost::add_edge(vertex[target], vertex[dependency], graph);
                }
            }
        }

        return {graph, vertexName};
    }

    void printGraph(std::ostream &os, const DependGraph &dg) {
        const auto &[graph, vname] = dg;
        boost::print_graph(graph, vname.data(), os);
    }
} // namespace make
