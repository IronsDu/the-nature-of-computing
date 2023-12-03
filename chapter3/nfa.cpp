#include <list>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace std;

using State = int;
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

    const int& nextState() const
    {
        return _nextState;
    }

    // 判断某开始状态和输入是否匹配此规则，匹配则表示接受
    bool accept(State currentState, std::optional<InputType> input) const
    {
        return _startState == currentState && (_input == std::nullopt || _input == input);
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

    bool accept(State state) const
    {
        return _acceptStateSet.find(state) != _acceptStateSet.end();
    }

private:
    const std::unordered_set<State> _acceptStateSet;
};

class NFA
{
public:
    NFA(std::vector<FARule> rules, AcceptStates acceptStates)
        : _rules(std::move(rules)),
          _acceptStates(std::move(acceptStates))
    {}

    bool accept(const State initialState, std::list<InputType> inputs) const
    {
        // 收集当前起始状态下空输入能满足的规则
        auto matchRules = _matchRules(initialState, std::nullopt);

        if (inputs.empty())
        {
            // 当输入为空时，直接判断采用空输入匹配的规则是否有处于接受状态
            for (auto rule : matchRules)
            {
                if (_acceptStates.accept(rule.nextState()))
                {
                    return true;
                }
            }
            return false;
        }

        for (auto rule : matchRules)
        {
            // 以空输入匹配的规则的转移状态为起始状态派生新的NFA，尝试判断新的NFA是否接受输入
            NFA const subNfa(_rules, _acceptStates);
            if (subNfa.accept(rule.nextState(), inputs))
            {
                return true;
            }
        }

        // 收集当前起始状态下，第一个符号所能匹配的规则
        matchRules = _matchRules(initialState, inputs.front());
        // 去掉开始符号，用于构造新的NFA输入
        inputs.pop_front();

        if (inputs.empty())
        {
            for (auto rule : matchRules)
            {
                if (_acceptStates.accept(rule.nextState()))
                {
                    return true;
                }
            }
        }

        for (auto rule : matchRules)
        {
            // 派生新的NFA，尝试判断新的NFA是否接受输入
            NFA const subNfa(_rules, _acceptStates);
            if (subNfa.accept(rule.nextState(), inputs))
            {
                return true;
            }
        }

        return false;
    }

private:
    // 返回满足当前状态和输入的规则列表
    std::vector<FARule> _matchRules(const State currentState, const std::optional<InputType> input) const
    {
        std::vector<FARule> nextRules;
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
                nextRules.push_back(rule);
            }
        }
        return nextRules;
    }

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

int main()
{
    std::vector<FARule> const rules = {
            {0, 'a', 1},// 状态0下若接受到'a'则转移到状态1
            {0, 'b', 2},
            {0, 'c', 1},
            {1, 'a', 2},
            {1, 'b', 2},
            {1, 'c', 2},
    };
    AcceptStates const acceptState({2});

    NFA const nfa(rules, acceptState);

    auto isAccepted = nfa.accept(0, convertStringToInputs("ab"));
    isAccepted = nfa.accept(0, convertStringToInputs("ad"));

    return 0;
}
