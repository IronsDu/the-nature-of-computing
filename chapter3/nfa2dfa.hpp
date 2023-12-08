#pragma once

#include "dfa.hpp"
#include "nfa.hpp"

namespace nfa2dfa {

// 生成状态的排列组合
static std::vector<std::set<State>> generateCombList(std::vector<State> stateList)
{
    if (stateList.empty())
    {
        return {};
    }

    std::vector<std::set<State>> combState;
    combState.push_back(std::set<State>({stateList[0]}));

    stateList.erase(stateList.begin());
    auto subComb = generateCombList(stateList);

    for (auto& stateSet : subComb)
    {
        combState.push_back(stateSet);
        stateSet.insert(stateList[0]);
        combState.push_back(stateSet);
    }

    return combState;
}

// 生成状态的排列组合
static std::vector<std::set<State>> generateCombList(std::set<State> stateSet)
{
    std::vector<State> stateList;
    for (const auto& s : stateSet)
    {
        stateList.push_back(s);
    }
    return generateCombList(stateList);
}

static DFA convertNFA2DFA(const NFA& nfa)
{
    // 获取NFA的状态转移表
    const auto transformRelation = nfa.getTransformRelationOptEmpty();

    // 新的起始状态
    State newInitialState;
    {
        // 获取NFA的起始状态，以及获取此起始状态在空输入下所能达到的状态
        std::set<State> newInitialStateSet;
        auto initialState = nfa.getInitialState();
        auto initialStateTarget = nfa.getEmptyInputTargetState(initialState);

        newInitialStateSet.insert(initialState);
        newInitialStateSet.insert(initialStateTarget.begin(), initialStateTarget.end());
        std::cout << "=====" << std::endl;
        for (const auto& s : newInitialStateSet)
        {
            std::cout << s << std::endl;
        }
        std::cout << "====" << std::endl;
        newInitialState = convertCombStateToString(newInitialStateSet);
    }

    std::cout << "~~~~~~~~~~" << std::endl;
    // 从状态转移表中构造状态顺序表，用于后续的状态组合
    std::vector<State> stateList;
    for (const auto& [state, _] : transformRelation)
    {
        stateList.push_back(state);
        std::cout << state << std::endl;
    }

    std::cout << "~~~~~~~~~~" << std::endl;
    std::vector<DFARule> rules;
    // 计算状态的排列组合，计算每一个组合状态集在一些输入下将转移到的状态集
    // 状态集都将转换为字符串，作为新的状态机的状态，并以此构造新的转换规则
    auto stateCombList = generateCombList(stateList);
    for (const auto& stateComb : stateCombList)
    {
        if (stateComb.contains("klf5initialrf7initial") &&
            stateComb.contains("lf5initial") &&
            stateComb.contains("lf5initialrf7initial") &&
            stateComb.contains("rf7initial"))
        {
            std::cout << "1" << std::endl;
        }
        // 计算此状态组合在一些输入下所能达到的状态集合
        std::map<InputType, std::set<State>> combTransform;
        for (const auto& s : stateComb)
        {
            const auto& transform = transformRelation.at(s);
            for (const auto& [input, stateSet] : transform)
            {
                combTransform[input].insert(stateSet.begin(), stateSet.end());
            }
        }

        // 使用起始状态集何输入以及转移的目标状态集构造新的DFA中的规则
        auto newState = convertCombStateToString(stateComb);
        if (newState == "klf5initialrf7initiallf5initiallf5initialrf7initialrf7initial")
        {
            std::cout << "1" << std::endl;
        }
        for (const auto& [input, nextStateSet] : combTransform)
        {
            auto newNextState = convertCombStateToString(nextStateSet);
            rules.push_back(DFARule(newState, input, newNextState));
        }
    }


    // 构造新的终止状态集
    std::unordered_set<State> acceptedState;
    {
        auto combStateList = generateCombList(nfa.getAcceptStates().getAcceptStateSet());
        for (const auto& stateComb : combStateList)
        {
            acceptedState.insert(convertCombStateToString(stateComb));
        }
    }

    return DFA(newInitialState, rules, DFAAcceptStates(acceptedState)).getOpt();
}
}// namespace nfa2dfa