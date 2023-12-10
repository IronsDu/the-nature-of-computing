#pragma once

#include "dfa.hpp"
#include "nfa.hpp"

namespace nfa2dfa {

static DFA convertNFA2DFA(const NFA& nfa)
{
    const std::vector<InputType> inputSet = nfa.getNoneEmptyInputSet();
    const auto determinationTransformRelation = nfa.getDeterminationTransformRelation();
    const auto initialStateEClosure = nfa.getEClosure(nfa.getInitialState());

    auto newInitialState = convertCombinationStateToString(initialStateEClosure);

    // 已经处理过的状态组合(此状态是原状态组合之后的状态)
    std::set<State> visitedStateSet;
    // 待处理的状态组合(其中的状态为原始状态)
    std::vector<std::set<State>> pendingStateSet;
    pendingStateSet.push_back(initialStateEClosure);

    // 新的规则列表
    std::vector<DFARule> newRules;
    // 新的终结状态
    std::unordered_set<State> newFinalState;

    if (nfa.getAcceptStates().accept(initialStateEClosure))
    {
        newFinalState.insert(newInitialState);
    }

    std::set<State> finalStateEClosure;
    {
        // 由于NFA里可能存在某些状态接受空输入可以达到结束状态，这些关系没有优化
        // 因此我们可以把这些状态也当作“终止”状态，后续子集构造中，只要遇到包含这些“终止”状态的组合，都可以看作新的DFA的终止状态
        const auto stateList = nfa.getStateSet();
        for (const auto& state : stateList)
        {
            if (nfa.getAcceptStates().accept(nfa.getEClosure(state)))
            {
                finalStateEClosure.insert(state);
            }
        }
    }

    NFAAcceptStates originAcceptedStateEClosure(finalStateEClosure);

    while (!pendingStateSet.empty())
    {
        auto currentStartStateSet = pendingStateSet.back();
        const auto currentStartStateStr = convertCombinationStateToString(currentStartStateSet);
        pendingStateSet.pop_back();
        visitedStateSet.insert(currentStartStateStr);

        for (const auto& input : inputSet)
        {
            // 计算当前开始状态集下输入input所能到达的状态集
            std::set<State> nextStateSet;
            for (const auto& state : currentStartStateSet)
            {
                if (!determinationTransformRelation.contains(state))
                {
                    continue;
                }
                const auto& transform = determinationTransformRelation.at(state);
                if (!transform.contains(input))
                {
                    continue;
                }
                const auto& originNextStateSet = transform.at(input);
                for (const auto& state : originNextStateSet)
                {
                    nextStateSet.insert(state);
                }
            }
            if (nextStateSet.empty())
            {
                continue;
            }

            auto newNextStateStr = convertCombinationStateToString(nextStateSet);

            if (originAcceptedStateEClosure.accept(nextStateSet))
            {
                newFinalState.insert(newNextStateStr);
            }
            if (!visitedStateSet.contains(newNextStateStr))
            {
                pendingStateSet.push_back(std::move(nextStateSet));
            }

            newRules.push_back(DFARule(currentStartStateStr, input, newNextStateStr));
        }
    }

    return DFA(newInitialState, std::move(newRules), DFAAcceptStates(std::move(newFinalState))).trim();
}

}// namespace nfa2dfa