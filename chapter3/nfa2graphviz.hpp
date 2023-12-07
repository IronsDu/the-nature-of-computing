#pragma once

#include <format>
#include <string>

#include "nfa.hpp"

namespace nfa2graphviz {
// 根据NFA生成graphviz
static std::string nfa2graphviz(const NFA& nfa)
{
    auto initState = nfa.getTnitialState();

    std::vector<std::string> nodes;
    auto stateSet = nfa.getStateSet();
    for (const auto& state : stateSet)
    {
        if (state == initState)
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=circle, color=green]", state, state));
        }
        else if (!nfa.getAcceptStates().accept(state))
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=circle]", state, state));
        }
        else
        {
            nodes.push_back(std::format("{} [label=<{}>, shape=doublecircle, color=blue]", state, state));
        }
    }

    std::vector<std::string> edges;
    auto transformRelation = nfa.getTransformRelation();
    for (const auto& [startState, transform] : transformRelation)
    {
        for (const auto& [input, nextStateSet] : transform)
        {
            for (const auto& nextState : nextStateSet)
            {
                auto falg = input ? std::string{char(input.value())} : std::string("ε");
                edges.push_back(std::format("{}->{} [label=<{}>]", startState, nextState, falg));
            }
        }
    }

    std::string graphviz = "digraph G{\n";
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
}// namespace nfa2graphviz