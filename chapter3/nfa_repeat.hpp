#pragma once

#include "nfa.hpp"

namespace nfa_operator {
// 生成NFA的克林闭包
// N1*
static NFA repeat(const NFA& nfa)
{
    auto newRules = nfa.getRules();

    // 添加从终止状态到初始状态在空输入下的状态转移
    for (const auto& state : nfa.getAcceptStates().getAcceptStateSet())
    {
        newRules.push_back(NFARule(state, std::nullopt, nfa.getInitialState()));
    }

    // 构造新的起始状态
    const auto newInitialState = "k" + nfa.getInitialState();

    // 在原终止状态集基础上，添加新的起始状态为终止状态
    auto finiteStateSet = nfa.getAcceptStates().getAcceptStateSet();
    finiteStateSet.insert(newInitialState);

    // 构造从新的起始状态到原起始状态在空输入下的状态转移
    newRules.push_back(NFARule(newInitialState, std::nullopt, nfa.getInitialState()));

    // 使用新的起始状态、新的状态转移以及原终止状态构造克林闭包NFA
    return NFA(newInitialState,
               newRules,
               NFAAcceptStates(finiteStateSet));
}
}// namespace nfa_operator