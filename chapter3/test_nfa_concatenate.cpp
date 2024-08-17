#define CATCH_CONFIG_MAIN// This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch_test_macros.hpp"
#include "nfa2graphviz.hpp"
#include "nfa_concatenate.hpp"

TEST_CASE("Test NFA concatenate", "[Test NFA concatenate]")
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

    auto oneNfaOutput = nfa2graphviz::nfa2graphviz(nfa);

    auto newNFA = nfa_operator::concatenate(nfa, nfa);

    const auto output = nfa2graphviz::nfa2graphviz(newNFA);

    const std::vector<std::pair<std::string, bool>> tests = {
            {"ab", false},
            {"ad", false},
            {"abab", true},
    };

    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = newNFA.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }
}
