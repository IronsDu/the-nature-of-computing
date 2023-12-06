#define CATCH_CONFIG_MAIN// This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"
#include "nfa.hpp"

TEST_CASE("Test NFA", "[Test NFA]")
{
    std::vector<FARule> const rules = {
            {"q0", 'a', "q1"},// 状态0下若接受到'a'则转移到状态1
            {"q0", 'b', "q2"},
            {"q0", 'c', "q1"},
            {"q1", 'a', "q2"},
            {"q1", 'b', "q2"},
            {"q1", 'c', "q2"},
    };
    AcceptStates const acceptState({"q2"});

    NFA const nfa("q0", rules, acceptState);

    const std::vector<std::pair<std::string, bool>> tests = {
            {"ab", true},
            {"ad", false},
    };

    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = nfa.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }
}
