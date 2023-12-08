#define CATCH_CONFIG_MAIN// This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"
#include "dfa2graphviz.hpp"
#include "fa_common.hpp"
#include "nfa.hpp"
#include "nfa2dfa.hpp"
#include "nfa2graphviz.hpp"

TEST_CASE("Test NFA2DFA", "[Test NFA2DFA]")
{
    std::vector<NFARule> const rules = {
            {"q0", 'a', "q1"},// 状态0下若接受到'a'则转移到状态1
            {"q0", 'b', "q2"},
            {"q0", 'c', "q1"},
            {"q1", 'a', "q2"},
            {"q1", 'b', "q2"},
            {"q1", 'c', "q2"},
    };
    NFAAcceptStates const acceptState({"q2"});

    NFA const nfa("q0", rules, acceptState);

    auto dfa = nfa2dfa::convertNFA2DFA(nfa);

    const std::vector<std::pair<std::string, bool>> tests = {
            {"ab", true},
            {"ad", false},
    };

    auto nfaGraphviz = nfa2graphviz::nfa2graphviz(nfa);
    auto dfaGraphviz = dfa2graphviz::dfa2graphviz(dfa);
    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = dfa.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }
}
