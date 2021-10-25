//
// dd_converter
//
// Copyright (c) 2021 Jun Kawahara and Toshiki Saitoh
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#ifndef TDZDD_CONVERTER_HPP
#define TDZDD_CONVERTER_HPP


#include <iostream>
#include <sstream>
#include <map>
#include <vector>

//#include "tdzdd/DdEval.hpp"
#include "tdzdd/DdSpec.hpp"
//#include "tdzdd/DdSpecOp.hpp"
#include "tdzdd/DdStructure.hpp"

namespace dd_converter {

void outputAsGraphillionText(std::ostream& ost,
                             const tdzdd::DdStructure<2>& dd)
{
    std::vector<std::vector<tdzdd::NodeId> > level_nodes;
    std::map<tdzdd::NodeId, int64_t> node2id;
    std::vector<tdzdd::NodeId> stack;
    tdzdd::NodeId root = dd.root();
    tdzdd::NodeId bot(0);
    tdzdd::NodeId top(1);

    stack.push_back(root);

    level_nodes.resize(root.row() + 1);

    node2id[bot] = -1;
    node2id[top] = -2;

    // visit all ZDD nodes
    while (!stack.empty()) {
        tdzdd::NodeId node = stack.back();
        stack.pop_back();

        assert(1 <= node.row() && node.row() <= root.row());
        level_nodes[node.row()].push_back(node);

        tdzdd::NodeId c0 = dd.child(node, 0);
        if (node2id.find(c0) == node2id.end()) { // not found
            stack.push_back(c0);
            node2id[c0] = 0;
        }

        tdzdd::NodeId c1 = dd.child(node, 1);
        if (node2id.find(c1) == node2id.end()) { // not found
            stack.push_back(c1);
            node2id[c1] = 0;
        }
    }

    // give IDs to nodes and make node2id
    int64_t id = 1;
    for (int level = 1; level <= root.row(); ++level) {
        for (size_t i = 0; i < level_nodes[level].size(); ++i) {
            tdzdd::NodeId node = level_nodes[level][i];
            node2id[node] = id;
            ++id;
        }
    }

    for (int level = 1; level <= root.row(); ++level) {
        for (size_t i = 0; i < level_nodes[level].size(); ++i) {
            tdzdd::NodeId node = level_nodes[level][i];
            ost << node2id[node] << " " << node.row() << " ";
            int64_t n0 = node2id[dd.child(node, 0)];
            if (n0 >= 0) {
                ost << n0;
            } else {
                ost << (n0 == -1 ? "B" : "T");
            }
            ost << " ";
            int64_t n1 = node2id[dd.child(node, 1)];
            if (n1 >= 0) {
                ost << n1;
            } else {
                ost << (n1 == -1 ? "B" : "T");
            }
            ost << std::endl;
        }
    }
    ost << "." << std::endl;
}


struct GraphillionToTdZddNode {
    int64_t id;
    int elem;
    int64_t lo;
    int64_t hi;
};

class GraphillionToTdZdd
    : public tdzdd::DdSpec<GraphillionToTdZdd, int64_t, 2> {
    const std::map<int64_t, GraphillionToTdZddNode>& nodes_;
    int64_t root_node_;
    int max_elem_;

public:
    GraphillionToTdZdd(const std::map<int64_t, GraphillionToTdZddNode>& nodes,
                       int64_t root_node, int max_elem) :
        nodes_(nodes), root_node_(root_node), max_elem_(max_elem) { }

    static int64_t stringToPositiveInt(const std::string& s) {
        for (size_t i = 0; i < s.length(); ++i) {
            if (!(isspace(s[i]) || isdigit(s[i]))) {
                return -3; // -3 means a format error
            }
        }
        // string -> int convert for old C++
        std::istringstream iss(s);
        int64_t v;
        iss >> v;
        return v;
    }

    static int64_t stringToId(const std::string& s) {
        if (s.length() == 1 && s[0] == 'B') {
            return -1;
        } else if (s.length() == 1 && s[0] == 'T') {
            return -2;
        } else {
            return stringToPositiveInt(s);
        }
    }

    int getRoot(int64_t& node) const {
        node = root_node_;
        return max_elem_;
    }

    int getChild(int64_t& node, int level, int take) const {
        int64_t id;
        if (take == 0) {
            id = nodes_.at(node).lo;
        } else {
            id = nodes_.at(node).hi;
        }
        if (id == -1) {
            return 0;
        } else if (id == -2) {
            return -1;
        }
        node = id;
        return nodes_.at(id).elem;
    }
};

tdzdd::DdStructure<2> graphillionToTdZdd(std::istream& ist)
{
    std::map<int64_t, GraphillionToTdZddNode> nodes;
    int64_t root_node = -1;
    int max_elem = 0;
    std::string line;
    int64_t line_number = 0;
    while (ist && std::getline(ist, line)) {
        if (line[0] == '.') {
            break;
        }
        ++line_number;
        GraphillionToTdZddNode node;
        std::istringstream linest(line);
        std::string v;
        linest >> v;
        if (linest.fail()) {
            goto graphillionToTdZdderror;
        }

        node.id = GraphillionToTdZdd::stringToPositiveInt(v);
        if (node.id < 0) {
            goto graphillionToTdZdderror;
        }
        root_node = node.id;

        linest >> v;
        if (linest.fail()) {
            goto graphillionToTdZdderror;
        }
        node.elem = GraphillionToTdZdd::stringToPositiveInt(v);
        if (node.elem < 0) {
            goto graphillionToTdZdderror;
        }
        if (node.elem > max_elem) {
            max_elem = node.elem;
        }

        linest >> v;
        if (linest.fail()) {
            goto graphillionToTdZdderror;
        }
        node.lo = GraphillionToTdZdd::stringToId(v);
        if (node.lo <= -3) { // -1 and -2 mean 0 and 1 terminal, respectively
            goto graphillionToTdZdderror;
        }

        linest >> v;
        if (linest.fail()) {
            goto graphillionToTdZdderror;
        }
        node.hi = GraphillionToTdZdd::stringToId(v);
        if (node.hi <= -3) { // -1 and -2 mean 0 and 1 terminal, respectively
            goto graphillionToTdZdderror;
        }
        nodes[node.id] = node;

        continue;

        // error processing
    graphillionToTdZdderror:
        std::cerr << "format error at line " << line_number << std::endl;
        exit(1);
    }
    GraphillionToTdZdd gtot(nodes, root_node, max_elem);
    return tdzdd::DdStructure<2>(gtot);
}

} // namespace dd_converter

#endif // TDZDD_CONVERTER_HPP
