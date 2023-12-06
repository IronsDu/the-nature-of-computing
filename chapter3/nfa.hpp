#include <list>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

using State = std::string;
using InputType = char;

// 有限状态机（状态转移）规则
// 定义某个状态下接收到某个输入时转移到哪个状态
// 因为用于实现NFA，所以输入允许为空
class FARule
{
public:
    FARule(State startState, std::optional<InputType> character, State nextState)
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
class AcceptStates
{
public:
    AcceptStates(std::unordered_set<State> acceptStateSet)
        : _acceptStateSet(std::move(acceptStateSet))
    {}

    bool accept(const State& state) const
    {
        return _acceptStateSet.contains(state);
    }

    const std::unordered_set<State>& getAcceptStateSet() const
    {
        return _acceptStateSet;
    }

private:
    const std::unordered_set<State> _acceptStateSet;
};

class NFA
{
public:
    NFA(State initialState, std::vector<FARule> rules, AcceptStates acceptStates)
        : _initialState(initialState),
          _rules(std::move(rules)),
          _acceptStates(std::move(acceptStates))
    {}

    const auto& getTnitialState() const
    {
        return _initialState;
    }

    const AcceptStates& getAcceptStates() const
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
    std::vector<FARule> _matchRules(const State currentState, const std::optional<InputType> input) const
    {
        std::vector<FARule> nextRules;
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
    const std::vector<FARule> _rules;
    const AcceptStates _acceptStates;
};


static std::list<InputType> convertStringToInputs(const std::string& str)
{
    std::list<InputType> inputs;
    for (char const c : str)
    {
        inputs.push_back(c);
    }
    return inputs;
}

// 将一个状态组合转换为一个字符串，通常，这个字符串作为构造新的状态机中的新状态
static std::string convertCombStateToString(std::set<State> stateSet)
{
    std::string str;
    // 必须依靠稳定顺序
    for (const auto& s : stateSet)
    {
        str += s;
    }
    return str;
}

static void convertNFA2DFA(const NFA& nfa)
{
    // 获取NFA的状态转移表
    const auto transformRelation = nfa.getTransformRelation();

    // 新的起始状态
    State newInitialState;
    {
        // 获取NFA的起始状态，以及获取此起始状态在空输入下所能达到的状态
        std::set<State> newInitialStateSet;
        auto initialState = nfa.getTnitialState();
        auto initialStateTarget = nfa.getEmptyInputTargetState(initialState);

        newInitialStateSet.insert(initialState);
        newInitialStateSet.insert(initialStateTarget.begin(), initialStateTarget.end());
        newInitialState = convertCombStateToString(newInitialStateSet);
    }

    // 从状态转移表中构造状态顺序表，用于后续的状态组合
    std::vector<State> stateList;
    for (const auto& [state, _] : transformRelation)
    {
        stateList.push_back(state);
    }

    std::unordered_set<State> acceptedState;

    std::vector<FARule> rules;
    // 计算状态的排列组合，计算每一个组合状态集在一些输入下将转移到的状态集
    // 状态集都将转换为字符串，作为新的状态机的状态，并以此构造新的转换规则
    for (int i = 0; i < stateList.size(); i++)
    {
        std::set<State> stateComb;
        for (int j = i; j < stateList.size(); j++)
        {
            // 构造状态组合，作为起始状态集
            stateComb.insert(stateList[j]);

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
            for (const auto& [input, nextStateSet] : combTransform)
            {
                auto newNextState = convertCombStateToString(nextStateSet);
                rules.push_back(FARule(newState, input, newNextState));
            }

            // 判断当前状态集中是否存在NFA中的终止状态，若存在，则将状态组合的新状态添加到新的终止状态集合
            bool isAccepted = false;
            for (const auto& s : stateComb)
            {
                if (nfa.getAcceptStates().accept(s))
                {
                    isAccepted = true;
                    break;
                }
            }
            if (isAccepted)
            {
                acceptedState.insert(newState);
            }
        }
    }
}