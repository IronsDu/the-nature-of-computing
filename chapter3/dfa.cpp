#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <optional>
using namespace std;

using State = int;
using InputType = char;

// 有限状态机（状态转移）规则
// 定义某个状态下接收到某个输入时转移到哪个状态
class FARule
{
public:
	FARule(State startState, InputType character, State nextState) :
		_startState(startState),
		_input(character),
		_nextState(nextState)
	{
	}

	const State& startState() const {
		return _startState;
	}

	const InputType& input() const {
		return _input;
	}

	const int& nextState() const {
		return	_nextState;
	}

	// 判断某开始状态和输入是否匹配此规则，匹配则表示接受
	bool accept(State currentState, InputType input) const {
		return _startState == currentState && _input == input;
	}

private:
	// 开始状态
	const State	_startState;
	// 接受的输入是什么
	const InputType _input;
	// 下一个状态
	const State _nextState;
};

// 定义终止状态，即可接受的状态有哪些
class AcceptStates {
public:
	AcceptStates(std::unordered_set<State> acceptStateSet)
		:_acceptStateSet(acceptStateSet)
	{}

	bool accept(State state) const {
		return _acceptStateSet.find(state) != _acceptStateSet.end();
	}

private:
	const std::unordered_set<State> _acceptStateSet;
};

// 有限状态机，由状态转移规则和可接受的中终止状态组成
class DFA {
public:
	DFA(std::vector<FARule> rules, AcceptStates acceptStates) :
		_rules(rules),
		_acceptStates(acceptStates)
	{}

	// 检查此有限状态机的定义是否有效
	bool valid() const {
		// 检测是否存在重复的状态转移规则
		// 即存在相同的[开始状态、输入]的组合。如果存在重复则存在不确定性、二义性。（后话：NFA则允许重复的组合）
		std::unordered_map < State, std::unordered_map<InputType, State>> stateRelation;
		for (const auto& rule : _rules) {
			auto& transform = stateRelation[rule.startState()];
			if (transform.find(rule.input()) != transform.end()) {
				// already exist [startState, input]
				return false;
			}
			transform[rule.input()] = rule.nextState();
		}

		// 检查是否存在转移的下一个状态属于结束状态，若存在则返回“有效”，当前代码仅实现功能（不做优化）
		for (const auto& [startState, transform] : stateRelation) {
			for (const auto& [input, nextState] : transform) {
				if (_acceptStates.accept(nextState)) {
					return true;
				}
			}
		}

		return false;
	}

	// 判断从初始状态开始，此FA是否接受输入序列
	bool accept(const State initialState, std::vector< InputType> inputs) const {
		auto currentState = initialState;
		for (const auto& currentInput : inputs) {
			auto matchRule = _accept(currentState, currentInput);
			// 如果没有找到可接受的规则，则返回false，表示此FA不接受输入序列
			if (!matchRule) {
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
	std::optional< FARule> _accept(const State currentState, const InputType input) const {
		for (const auto& rule : _rules) {
			if (rule.startState() != currentState) {
				// 如果规则开始状态不是当前状态，则跳过此规则
				// 注意：理论上可以不进行此判断，因为下面的rule.accept自然也会判断
				continue;
			}

			if (rule.accept(currentState, input)) {
				return rule;
			}
		}
		return std::nullopt;
	}
private:
	const std::vector< FARule>  _rules;
	const AcceptStates _acceptStates;
};

static std::vector< InputType> convertStringToInputs(std::string str) {
	std::vector< InputType> inputs;
	for (char c : str) {
		inputs.push_back(c);
	}
	return inputs;
}

int main()
{
	std::vector<FARule> rules = {
		{0, 'a', 1},	// 状态0下若接受到'a'则转移到状态1
		{0, 'b', 2} ,
		{0, 'c', 1} ,
		{1, 'a', 2} ,
		{1, 'b', 2} ,
		{1, 'c', 2} ,
	};
	AcceptStates acceptState({ 2 });

	DFA dfa(rules, acceptState);

	auto isAccepted = dfa.accept(0, convertStringToInputs("ac"));

	return 0;
}
