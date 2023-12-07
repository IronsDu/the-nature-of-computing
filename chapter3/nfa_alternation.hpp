#pragma once

#include "nfa.hpp"

namespace nfa_operator {
// 并两个状态机生成新的状态机
// N1 | N2
static NFA alternation(const NFA& left, const NFA& right)
{
    const static std::string leftStatePrefix = "l";
    const static std::string rightStatePrefix = "r";

    std::vector<NFARule> newRules;

    // 根据状态前缀和状态转移表生成新的状态转移表
    using TransformRelationType = std::map<State, std::map<std::optional<InputType>, std::set<State>>>;
    const auto generateNewRule = [&newRules](const std::string& prefix, const TransformRelationType& transfromRelation) {
        for (const auto& [startState, transform] : transfromRelation)
        {
            for (const auto& [input, stateSet] : transform)
            {
                for (const auto& nextState : stateSet)
                {
                    newRules.push_back(NFARule(prefix + startState, input, prefix + nextState));
                }
            }
        }
    };

    // 对左边的NFA施加前缀并添加新的关系
    generateNewRule(leftStatePrefix, left.getTransformRelation());

    // 对右边的NFA施加前缀并添加新的关系
    generateNewRule(rightStatePrefix, right.getTransformRelation());


    // 添加前缀，然后合并为一个终止状态集
    std::set<State> newFiniteStateSet;
    for (const auto& state : left.getAcceptStates().getAcceptStateSet())
    {
        newFiniteStateSet.insert(leftStatePrefix + state);
    }
    for (const auto& state : right.getAcceptStates().getAcceptStateSet())
    {
        newFiniteStateSet.insert(rightStatePrefix + state);
    }

    // 构造新的起始状态，添加从新的起始状态接受空输入下转移到两个NFA起始状态的状态转移
    const auto newInitialState = leftStatePrefix + left.getInitialState() + rightStatePrefix + right.getInitialState();
    newRules.push_back(NFARule(newInitialState, std::nullopt, leftStatePrefix + left.getInitialState()));
    newRules.push_back(NFARule(newInitialState, std::nullopt, rightStatePrefix + right.getInitialState()));

    return NFA(newInitialState,
               newRules,
               NFAAcceptStates(newFiniteStateSet));
}
}// namespace nfa_operator