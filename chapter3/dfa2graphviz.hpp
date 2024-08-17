#pragma once

#include <format>
#include <string>

#include "dfa.hpp"

namespace dfa2graphviz {
// 根据DFA生成graphviz
static std::string dfa2graphviz(const DFA& dfa)
{
    auto initState = dfa.getInitialState();

    std::vector<std::string> nodes;
    auto stateSet = dfa.getStateSet();
    for (const auto& state : stateSet)
    {
        if (state == initState && dfa.getAcceptStates().accept(state))
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=doublecircle, color=green]", state, state));
        }
        else if (state == initState)
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=circle, color=green]", state, state));
        }
        else if (dfa.getAcceptStates().accept(state))
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=doublecircle, color=blue]", state, state));
        }
        else
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=circle]", state, state));
        }
    }

    std::vector<std::string> edges;
    const auto& rules = dfa.getRules();
    const auto& transformRelation = dfa.getTransformRelation();
    for (const auto& [startState, transform] : transformRelation)
    {
        for (const auto& [input, nextState] : transform)
        {
            edges.push_back(std::format("{}->{} [label=<{}>]", startState, nextState, input));

        }
    }

    std::string graphviz = "digraph G{\n";
    graphviz += "rankdir = LR\n";

    for (const auto& node : nodes)
    {
        graphviz += node;
        graphviz += "\n";
    }
    for (const auto& edge : edges)
    {
        graphviz += edge;
        graphviz += "\n";
    }
    graphviz += "}";

    return graphviz;
}

}// namespace dfa2graphviz