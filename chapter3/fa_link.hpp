#pragma once

#include "dfa.hpp"
#include "nfa.hpp"

namespace fa_link {
// 顺序连捏两个状态机生成新的状态机
static NFA link(const NFA& front, const NFA& tail)
{
    const static std::string frontStatePrefix = "f";
    const static std::string tailStatePrefix = "t";

    std::vector<NFARule> newRules;

    // 根据状态前缀何状态转移表生成新的状态转移表
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

    // 对前面的NFA施加前缀并添加新的关系
    generateNewRule(frontStatePrefix, front.getTransformRelation());

    // 对侯面的NFA施加前缀并添加新的关系
    generateNewRule(tailStatePrefix, tail.getTransformRelation());

    // 生成从前面的NFA的终止状态转移接受空输入到后面NFA的起始状态的状态转移
    for (const auto& state : front.getAcceptStates().getAcceptStateSet())
    {
        newRules.push_back(NFARule(frontStatePrefix + state, std::nullopt, tailStatePrefix + tail.getTnitialState()));
    }

    // 根据后面NFA的终止状态施加前缀，得到新的终止状态
    std::set<State> newFiniteStateSet;
    for (const auto& state : front.getAcceptStates().getAcceptStateSet())
    {
        newFiniteStateSet.insert(tailStatePrefix + state);
    }

    return NFA(frontStatePrefix + front.getTnitialState(), newRules, NFAAcceptStates(newFiniteStateSet));
}
}// namespace fa_link