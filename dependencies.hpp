//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_DEPENDENCIES_HPP
#define MAKE_DEPENDENCIES_HPP

#include "types/rule.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <vector>

namespace make {
    using VertexProperty = boost::property<boost::vertex_name_t, std::string>;
    using Graph = boost::adjacency_list<boost::listS, boost::vecS,
                                        boost::directedS, VertexProperty>;
    using DependGraph = std::tuple<Graph, std::vector<std::string>>;

    DependGraph createDependGraph(const std::vector<Rule> &rules,
                                  const std::vector<std::string> &files);

    void printGraph(std::ostream &os, const DependGraph &dg);
} // namespace make

#endif // MAKE_DEPENDENCIES_HPP
