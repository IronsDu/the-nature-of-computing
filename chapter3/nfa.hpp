#pragma once

#include <list>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "fa_common.hpp"

// 有限状态机（状态转移）规则
// 定义某个状态下接收到某个输入时转移到哪个状态
// 因为用于实现NFA，所以输入允许为空
class NFARule
{
public:
    NFARule(State startState, std::optional<InputType> character, State nextState)
        : _startState(startState),
          _input(character),
          _nextState(nextState)
    {
    }

    const State& startState() const
    {
        return _startState;
    }

    const std::optional<InputType>& input() const
    {
        return _input;
    }

    const auto& nextState() const
    {
        return _nextState;
    }

    // 判断某开始状态和输入是否匹配此规则，匹配则表示接受
    bool accept(State currentState, std::optional<InputType> input) const
    {
        return _startState == currentState && _input == input;
    }

private:
    // 开始状态
    const State _startState;
    // 接受的输入是什么，允许为空
    const std::optional<InputType> _input;
    // 下一个状态
    const State _nextState;
};

// 定义终止状态，即可接受的状态有哪些
class NFAAcceptStates
{
public:
    NFAAcceptStates(std::set<State> acceptStateSet)
        : _acceptStateSet(std::move(acceptStateSet))
    {}

    bool accept(const State& state) const
    {
        return _acceptStateSet.contains(state);
    }

    const std::set<State>& getAcceptStateSet() const
    {
        return _acceptStateSet;
    }

private:
    const std::set<State> _acceptStateSet;
};

class NFA
{
public:
    NFA(State initialState, std::vector<NFARule> rules, NFAAcceptStates acceptStates)
        : _initialState(initialState),
          _rules(std::move(rules)),
          _acceptStates(std::move(acceptStates))
    {}

    const auto& getTnitialState() const
    {
        return _initialState;
    }

    const NFAAcceptStates& getAcceptStates() const
    {
        return _acceptStates;
    }

    bool accept(std::list<InputType> inputs) const
    {
        if (inputs.empty() && _acceptStates.accept(_initialState))
        {
            return true;
        }

        // 收集当前起始状态下空输入能满足的规则
        auto matchRules = _matchRules(_initialState, std::nullopt);
        for (auto rule : matchRules)
        {
            // 以空输入匹配的规则的转移状态为起始状态派生新的NFA，尝试判断新的NFA是否接受输入
            NFA const subNfa(rule.nextState(), _rules, _acceptStates);
            if (subNfa.accept(inputs))
            {
                return true;
            }
        }

        if (!inputs.empty())
        {
            // 收集当前起始状态下，第一个符号所能匹配的规则
            matchRules = _matchRules(_initialState, inputs.front());
            // 去掉开始符号，以剩下的输入构造新的NFA输入
            inputs.pop_front();

            for (auto rule : matchRules)
            {
                // 派生新的NFA，尝试判断新的NFA是否接受输入
                NFA const subNfa(rule.nextState(), _rules, _acceptStates);
                if (subNfa.accept(inputs))
                {
                    return true;
                }
            }
        }

        return false;
    }

    // 获取状态转移表
    // 外层map的key作为起始状态，其value表示此状态下接受的非空输入所能达到的状态集合
    std::map<State, std::map<InputType, std::set<State>>> getTransformRelation() const
    {
        std::map<State, std::map<InputType, std::set<State>>> transformMap;

        for (const auto& rule : _rules)
        {
            auto& transform = transformMap[rule.startState()];
            if (!rule.input())
            {
                continue;
            }

            auto& nextStateSet = transform[rule.input().value()];
            nextStateSet.insert(rule.nextState());
            auto emptyInputTargetStateSet = getEmptyInputTargetState(rule.nextState());
            for (const auto& s : emptyInputTargetStateSet)
            {
                nextStateSet.insert(s);
            }
        }

        return transformMap;
    }

    // 计算以某状态开始，以空作为输入所能达到的状态集
    std::set<State> getEmptyInputTargetState(State startState) const
    {
        std::set<State> targetStateSet;
        for (const auto& rule : _rules)
        {
            if (!rule.accept(startState, std::nullopt))
            {
                continue;
            }

            targetStateSet.insert(rule.nextState());
            auto subTargetStateSet = getEmptyInputTargetState(rule.nextState());
            for (const auto& s : subTargetStateSet)
            {
                targetStateSet.insert(s);
            }
        }
        return targetStateSet;
    }

private:
    // 返回满足当前状态和输入的规则列表
    std::vector<NFARule> _matchRules(const State currentState, const std::optional<InputType> input) const
    {
        std::vector<NFARule> nextRules;
        for (const auto& rule : _rules)
        {
            if (rule.accept(currentState, input))
            {
                nextRules.push_back(rule);
            }
        }
        return nextRules;
    }

private:
    const State _initialState;
    const std::vector<NFARule> _rules;
    const NFAAcceptStates _acceptStates;
};
