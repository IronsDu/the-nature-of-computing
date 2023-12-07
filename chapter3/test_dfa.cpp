#define CATCH_CONFIG_MAIN// This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"
#include "dfa.hpp"

TEST_CASE("Test DFA", "[Test DFA]")
{
    std::vector<DFARule> const rules = {
            {"q1", '0', "q1"},// 状态q0下若接受到'0'则转移到状态q1
            {"q1", '1', "q2"},
            {"q2", '0', "q3"},
            {"q2", '1', "q2"},
            {"q3", '0', "q2"},
            {"q3", '1', "q2"},
    };
    DFAAcceptStates const acceptState({"q2"});

    DFA const dfa("q1", rules, acceptState);

    REQUIRE(dfa.valid());

    const std::vector<std::pair<std::string, bool>> tests = {
            {"11", true},
            {"0101010101", true},
            {"001", true},
            {"110", false},
    };

    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = dfa.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }
}
