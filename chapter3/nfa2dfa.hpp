#pragma once

#include "dfa.hpp"
#include "nfa.hpp"

namespace nfa2dfa {

// 生成状态的排列组合
static std::vector<std::vector<State>> generateCombinationList(const std::vector<State>& stateList)
{
    std::vector<std::vector<State>> stateSetList;
    for (const auto& state : stateList)
    {
        const auto oldSize = stateSetList.size();
        for (size_t i = 0; i < oldSize; i++)
        {
            auto tmp = stateSetList[i];
            tmp.push_back(state);
            stateSetList.emplace_back(std::move(tmp));
        }
        stateSetList.push_back({state});
    }
    return stateSetList;
}

// 生成状态的排列组合
static std::vector<std::vector<State>> generateCombinationList(const std::set<State>& stateSet)
{
    std::vector<State> stateList;
    for (const auto& state : stateSet)
    {
        stateList.push_back(state);
    }
    return generateCombinationList(stateList);
}

static DFA convertNFA2DFA(const NFA& nfa)
{
    // 新的起始状态
    State newInitialState;
    {
        // 获取NFA的起始状态，以及获取此起始状态在空输入下所能达到的状态
        std::set<State> newInitialStateSet;
        auto initialState = nfa.getInitialState();
        auto initialStateTarget = nfa.getEClosure(initialState);

        newInitialStateSet.insert(initialState);
        newInitialStateSet.insert(initialStateTarget.begin(), initialStateTarget.end());

        newInitialState = convertCombinationStateToString(newInitialStateSet);
    }

    // 新的终止状态集
    std::unordered_set<State> newAcceptedState;

    // 新的规则列表
    std::vector<DFARule> newRules;

    {
        const auto stateSet = nfa.getStateSet();

        const auto combinationStateList = generateCombinationList(stateSet);

        // 构造新的终止状态集
        for (const auto& combinationState : combinationStateList)
        {
            bool hasAcceptedState = false;
            for (const auto& state : combinationState)
            {
                if (nfa.getAcceptStates().accept(state))
                {
                    hasAcceptedState = true;
                    break;
                }
            }
            if (hasAcceptedState)
            {
                newAcceptedState.insert(convertCombinationStateToString(combinationState));
            }
        }

        // 获取NFA的状态转移表
        const auto transformRelation = nfa.getDeterminationTransformRelation();

        // 计算每一个组合状态集在一些输入下将转移到的状态集
        // 状态集都将转换为字符串，作为新的状态机的状态，并以此构造新的转换规则
        for (const auto& combinationState : combinationStateList)
        {
            // 计算此状态组合在一些输入下所能达到的状态集合
            std::map<InputType, std::set<State>> combinationTransform;
            for (const auto& state : combinationState)
            {
                if (!transformRelation.contains(state))
                {
                    continue;
                }
                const auto& transform = transformRelation.at(state);
                for (const auto& [input, stateSet] : transform)
                {
                    combinationTransform[input].insert(stateSet.begin(), stateSet.end());
                }
            }

            // 使用起始状态集何输入以及转移的目标状态集构造新的DFA中的规则
            auto newStartState = convertCombinationStateToString(combinationState);
            for (const auto& [input, nextStateSet] : combinationTransform)
            {
                auto newNextState = convertCombinationStateToString(nextStateSet);
                newRules.push_back(DFARule(newStartState, input, std::move(newNextState)));
            }
        }
    }

    return DFA(newInitialState, std::move(newRules), DFAAcceptStates(std::move(newAcceptedState))).trim();
}
}// namespace nfa2dfa