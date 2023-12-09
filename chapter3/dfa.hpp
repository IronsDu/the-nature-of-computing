#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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
          _acceptStates(std::move(acceptStates))
    {}

    DFA(DFA&& rvalue)
        : _initialState(std::move(rvalue._initialState)),
          _rules(std::move(rvalue._rules)),
          _acceptStates(std::move(rvalue._acceptStates))
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
        std::map<State, std::map<InputType, State>> transformRelation;
        for (const auto& rule : _rules)
        {
            transformRelation[rule.startState()][rule.input()] = rule.nextState();
        }

        std::vector<State> leftState = {_initialState};
        // 记录当前访问过的状态节点，这些节点也是能够到达的节点，这些节点之外的节点都是无法到达的.
        std::set<State> visitedState;
        while (!leftState.empty())
        {
            auto state = leftState.back();
            visitedState.insert(state);
            leftState.pop_back();

            if (!transformRelation.contains(state))
            {
                continue;
            }

            for (const auto& [input, nextState] : transformRelation[state])
            {
                if (!visitedState.contains(nextState))
                {
                    leftState.push_back(nextState);
                }
            }
        }

        // 从状态转移中删除无法访问的节点的状态转移
        for (auto it = transformRelation.begin(); it != transformRelation.end();)
        {
            if (!visitedState.contains(it->first))
            {
                it = transformRelation.erase(it);
            }
            else
            {
                it++;
            }
        }

        // 根据最新的状态转移表构建转移规则列表
        std::vector<DFARule> rules;
        for (const auto& [state, transfrom] : transformRelation)
        {
            for (const auto& [input, nextState] : transfrom)
            {
                rules.push_back(DFARule(state, input, nextState));
            }
        }

        // _acceptStates 无需修改，不影响正确性
        return DFA(_initialState, std::move(rules), _acceptStates);
    }

    // 访问状态机中的所有状态
    auto getStateSet() const
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

    // 判断从初始状态开始，此FA是否接受输入序列
    bool accept(const std::list<InputType>& inputs) const
    {
        auto currentState = _initialState;
        for (const auto& currentInput : inputs)
        {
            auto matchRule = _accept(currentState, currentInput);
            // 如果没有找到可接受的规则，则返回false，表示此FA不接受输入序列
            if (!matchRule)
            {
                return false;
            }
            // 状态切换到所匹配的规则指定的下一个状态
            currentState = matchRule->nextState();
        }

        // 处理完输入序列之后，判断当前状态是否属于终止状态（即可接受状态)
        return _acceptStates.accept(currentState);
    }

private:
    // 判断以某个状态为开始的规则是否接受输入，若是则返回接受它的规则
    std::optional<DFARule> _accept(const State currentState, const InputType input) const
    {
        for (const auto& rule : _rules)
        {
            if (rule.startState() != currentState)
            {
                // 如果规则开始状态不是当前状态，则跳过此规则
                // 注意：理论上可以不进行此判断，因为下面的rule.accept自然也会判断
                continue;
            }

            if (rule.accept(currentState, input))
            {
                return rule;
            }
        }
        return std::nullopt;
    }

    const State _initialState;
    const std::vector<DFARule> _rules;
    const DFAAcceptStates _acceptStates;
};
