#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <map>

#include "fa_common.hpp"

// 有限状态机（状态转移）规则
// 定义某个状态下接收到某个输入时转移到哪个状态
class DFARule
{
public:
    DFARule(State startState, InputType character, State nextState)
        : _startState(std::move(startState)),
          _input(character),
          _nextState(std::move(nextState))
    {
    }

    const State& startState() const
    {
        return _startState;
    }

    const InputType& input() const
    {
        return _input;
    }

    const auto& nextState() const
    {
        return _nextState;
    }

    // 判断某开始状态和输入是否匹配此规则，匹配则表示接受
    bool accept(const State& currentState, InputType input) const
    {
        return _startState == currentState && _input == input;
    }

private:
    // 开始状态
    const State _startState;
    // 接受的输入是什么
    const InputType _input;
    // 下一个状态
    const State _nextState;
};

// 定义终止状态，即可接受的状态有哪些
class DFAAcceptStates
{
public:
    DFAAcceptStates(std::unordered_set<State> acceptStateSet)
        : _acceptStateSet(std::move(acceptStateSet))
    {}
    DFAAcceptStates(const DFAAcceptStates& right)
        : _acceptStateSet(right._acceptStateSet)
    {
    }
    DFAAcceptStates(DFAAcceptStates&& rvalue)
        : _acceptStateSet(std::move(rvalue._acceptStateSet))
    {
    }

    bool accept(const State& state) const
    {
        return _acceptStateSet.contains(state);
    }

    const auto& getStateSet() const
    {
        return _acceptStateSet;
    }

private:
    const std::unordered_set<State> _acceptStateSet;
};

// 有限状态机，由状态转移规则和可接受的中终止状态组成
class DFA
{
public:
    DFA(State initialState, std::vector<DFARule> rules, DFAAcceptStates acceptStates)
        : _initialState(std::move(initialState)),
          _rules(std::move(rules)),
          _acceptStates(std::move(acceptStates)),
          _transformRelation(generateTransformRelation())
    {}

    DFA(DFA&& rvalue)
        : _initialState(std::move(rvalue._initialState)),
          _rules(std::move(rvalue._rules)),
          _acceptStates(std::move(rvalue._acceptStates)),
          _transformRelation(generateTransformRelation())
    {
    }

    const auto& getInitialState() const
    {
        return _initialState;
    }

    const auto& getAcceptStates() const
    {
        return _acceptStates;
    }

    const auto& getRules() const
    {
        return _rules;
    }

    // 裁剪掉无法到达的状态及其状态转移规则
    auto trim() const
    {
        // 待处理状态
        std::vector<State> pedingState = {_initialState};
        // 记录当前访问过的状态节点，这些节点也是能够到达的节点，这些节点之外的节点都是无法到达的.
        std::set<State> visitedState;

        while (!pedingState.empty())
        {
            auto state = pedingState.back();
            visitedState.insert(state);
            pedingState.pop_back();

            const auto it = _transformRelation.find(state);
            if (it == _transformRelation.end())
            {
                continue;
            }

            for (const auto& [input, nextState] : it->second)
            {
                if (!visitedState.contains(nextState))
                {
                    pedingState.push_back(nextState);
                }
            }
        }

        auto transformRelation = _transformRelation;
        // 从状态转移中删除无法访问的节点的状态转移
        for (auto it = transformRelation.begin(); it != transformRelation.end();)
        {
            if (!visitedState.contains(it->first))
            {
                // 如果此节点没有在之前被访问过，则可以删除从它开始的这条状态转移关系
                it = transformRelation.erase(it);
            }
            else
            {
                it++;
            }
        }

        std::unordered_set<State> newFinalStateSet;
        if (_acceptStates.accept(_initialState))
        {
            newFinalStateSet.insert(_initialState);
        }

        // 根据最新的状态转移表构建转移规则列表
        std::vector<DFARule> rules;
        for (const auto& [state, transfrom] : transformRelation)
        {
            for (const auto& [input, nextState] : transfrom)
            {
                rules.push_back(DFARule(state, input, nextState));
                if (_acceptStates.accept(nextState))
                {
                    newFinalStateSet.insert(nextState);
                }
            }
        }

        return DFA(_initialState, std::move(rules), DFAAcceptStates(newFinalStateSet));
    }

    // 获取最小化的DFA
    auto minimize() const
    {
        const auto inputSet = getInputSet();

        std::vector<std::set<State>> groupList;
        std::vector<std::set<State>> newGroupList;

        {
            // 收集所有转换关系中的开始状态集合，这些状态才是允许合并的
            std::set<State> stateSet;
            for (const auto& [state, _] : _transformRelation)
            {
                stateSet.insert(state);
            }
            groupList.push_back(stateSet);
        }

        // 根据每一个输入划分状态分组
        for (const auto& input : inputSet)
        {
            for (const auto& group : groupList)
            {
                // 如果此分组的个数不超过1，则不需要进行尝试拆分
                if (group.size() <= 1)
                {
                    newGroupList.push_back(group);
                    continue;
                }

                std::map<State, std::set<State>> eatchSet;
                std::set<State> cantEatchState;

                for (const auto& state : group)
                {
                    const auto it = _transformRelation.find(state);
                    if (it == _transformRelation.end())
                    {
                        // 这里应抛异常，按理不会存在这个分支情况
                        cantEatchState.insert(state);
                        continue;
                    }
                    const auto nextStateIt = it->second.find(input);
                    if (nextStateIt == it->second.end())
                    {
                        // 如果是正规DFA，则也不应该出现此情况，但我们这里不做严格要求
                        cantEatchState.insert(state);
                        continue;
                    }
                    // 记录能到达某个状态的起始状态集
                    eatchSet[nextStateIt->second].insert(state);
                }

                for (const auto& [_, startStateSet] : eatchSet)
                {
                    newGroupList.push_back(startStateSet);
                }
                if (!cantEatchState.empty())
                {
                    newGroupList.push_back(cantEatchState);
                }
            }

            groupList = newGroupList;
            newGroupList.clear();
        }

        // 根据分组构建替换表
        std::map<State, State> insteadOf;
        for (auto& group : groupList)
        {
            for (const auto& state : group)
            {
                insteadOf[state] = convertCombinationStateToString(group);
            }
        }

        // 根据最新的状态转移表构建新的转移规则列表
        std::vector<DFARule> rules;
        for (const auto& rule : _rules)
        {
            auto startState = rule.startState();
            if (const auto it = insteadOf.find(startState); it != insteadOf.end())
            {
                startState = it->second;
            }

            auto nextState = rule.nextState();
            if (const auto it = insteadOf.find(nextState); it != insteadOf.end())
            {
                nextState = it->second;
            }

            rules.push_back(DFARule(startState, rule.input(), nextState));
        }

        // 获取新的起始状态
        auto newInitialState = _initialState;
        if (auto it = insteadOf.find(newInitialState); it != insteadOf.end())
        {
            newInitialState = it->second;
        }

        // 构建新的终止状态（因为某些终止状态也是一些关系的起始状态，也存在被合并的可能，所以可能需要替换）
        std::unordered_set<State> newFinalStateSet;
        for (const auto& oldFinalState : _acceptStates.getStateSet())
        {
            if (const auto it = insteadOf.find(oldFinalState); it != insteadOf.end())
            {
                newFinalStateSet.insert(it->second);
            }
            else
            {
                newFinalStateSet.insert(oldFinalState);
            }
        }

        return DFA(newInitialState, std::move(rules), DFAAcceptStates(newFinalStateSet));
    }

    std::set<InputType> getInputSet() const
    {
        std::set<InputType> inputSet;
        for (const auto& rule : _rules)
        {
            inputSet.insert(rule.input());
        }
        return inputSet;
    }

    // 访问状态机中的所有状态
    std::set<State> getStateSet() const
    {
        std::set<State> stateSet;
        for (const auto& rule : _rules)
        {
            stateSet.insert(rule.startState());
            stateSet.insert(rule.nextState());
        }
        return stateSet;
    }

    // 检查此有限状态机的定义是否有效
    bool valid() const
    {
        // 记录状态集合
        std::unordered_set<State> stateSet;
        // 记录符号集合
        std::unordered_set<InputType> inputSet;

        // 1. 从规则中收集状态集合、符号集合
        // 2. 检测是否存在重复的状态转移规则
        // 即存在相同的[开始状态、输入]的组合。如果存在重复则存在不确定性、二义性。（后话：NFA则允许重复的组合）
        std::unordered_map<State, std::unordered_map<InputType, State>> stateRelation;
        for (const auto& rule : _rules)
        {
            auto& transform = stateRelation[rule.startState()];
            if (transform.contains(rule.input()))
            {
                // 存在重复的状态转移 [startState, input]
                return false;
            }
            transform[rule.input()] = rule.nextState();

            stateSet.insert(rule.startState());
            stateSet.insert(rule.nextState());
            inputSet.insert(rule.input());
        }

        if (!stateSet.contains(_initialState))
        {
            // 如果起始状态没在状态集合中则返回false
            return false;
        }

        // 检测状态转移是否满足DFA定义的约束，即满足(状态,输入)的任意组合都有且只有一个规则。
        for (auto state : stateSet)
        {
            if (_acceptStates.accept(state))
            {
                continue;
            }
            const auto it = stateRelation.find(state);
            if (it == stateRelation.end())
            {
                // 找不到以某状态作为开始的状态转移
                return false;
            }
            const auto& transform = (*it).second;

            for (const auto& input : inputSet)
            {
                if (!transform.contains(input))
                {
                    // 在当前开始状态下没有找到针对某个输入符号的状态转移
                    return false;
                }
            }
        }

        // 检查是否存在转移的下一个状态属于结束状态，若存在则返回“有效”，当前代码仅实现功能（不做优化）
        for (const auto& [startState, transform] : stateRelation)
        {
            for (const auto& [input, nextState] : transform)
            {
                if (_acceptStates.accept(nextState))
                {
                    return true;
                }
            }
        }

        return false;
    }

    // 获取确定性状态转移表
    const auto& getTransformRelation() const
    {
        return _transformRelation;
    }


    // 判断从初始状态开始，此FA是否接受输入序列
    bool accept(const std::vector<InputType>& inputs) const
    {
        auto currentState = _initialState;
        for (const auto& currentInput : inputs)
        {
            const auto it = _transformRelation.find(currentState);
            if (it == _transformRelation.end())
            {
                return false;
            }
            const auto nextStateIt = it->second.find(currentInput);
            if (nextStateIt == it->second.end())
            {
                return false;
            }
            currentState = nextStateIt->second;
        }

        // 处理完输入序列之后，判断当前状态是否属于终止状态（即可接受状态)
        return _acceptStates.accept(currentState);
    }

private:
    // 构造确定性状态转移表
    std::map<State, std::map<InputType, State>> generateTransformRelation() const
    {
        std::map<State, std::map<InputType, State>> transformMap;
        for (const auto& rule : _rules)
        {
            transformMap[rule.startState()][rule.input()] = rule.nextState();
        }
        return transformMap;
    }


private:
    const State _initialState;
    const std::vector<DFARule> _rules;
    const DFAAcceptStates _acceptStates;

    const std::map<State, std::map<InputType, State>> _transformRelation;
};
