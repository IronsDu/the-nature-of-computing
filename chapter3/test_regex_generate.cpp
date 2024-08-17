#define CATCH_CONFIG_MAIN// This tells Catch to provide a main() - only do this in one cpp file
#include <iostream>

#include "catch2/catch_test_macros.hpp"
#include "dfa2graphviz.hpp"
#include "nfa2dfa.hpp"
#include "nfa2graphviz.hpp"
#include "regex_generate.hpp"

TEST_CASE("Test regex generage match str", "[Test regex generage match str]")
{
    const auto nfa = regex_generate::Str("abc");

    const std::vector<std::pair<std::string, bool>> tests = {
            {"ad", false},
            {"ab", false},
            {"aa", false},
            {"a", false},
            {"aba", false},
            {"abab", false},
            {"aaaa", false},
            {"aa", false},
            {"bbbbbbb", false},
            {"", false},
            {"abc", true},
    };

    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = nfa.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }
    auto output = nfa2graphviz::nfa2graphviz(nfa);
    REQUIRE(output.length() > 0);
}


TEST_CASE("Test regex generage match repeat str", "[Test regex generage match repeat str]")
{
    // (abc|abcd)*

    const auto abcNfa = regex_generate::Str("abc");
    const auto abcdNfa = regex_generate::Str("abcd");
    auto alternationNfa = nfa_operator::alternation(abcNfa, abcdNfa);
    auto nfa = nfa_operator::repeat(alternationNfa);

    const std::vector<std::pair<std::string, bool>> tests = {
            {"ad", false},
            {"ab", false},
            {"aa", false},
            {"a", false},
            {"aba", false},
            {"abab", false},
            {"aaaa", false},
            {"aa", false},
            {"bbbbbbb", false},
            {"", true},
            {"abc", true},
            {"abcabc", true},
            {"abcd", true},
            {"abcdabcd", true},
            {"abcabcd", true},
            {"abcdabcabcabcd", true},
    };

    auto nfaGraphviz = nfa2graphviz::nfa2graphviz(nfa);
    REQUIRE(nfaGraphviz.length() > 0);

    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = nfa.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }

    auto dfa = nfa2dfa::convertNFA2DFA(nfa);
    auto dfaGraphviz = dfa2graphviz::dfa2graphviz(dfa);

    std::cout << dfaGraphviz << std::endl;
    REQUIRE(dfaGraphviz.length() > 0);

    for (const auto& [input, expectedAccepted] : tests)
    {
        const auto isAccepted = dfa.accept(convertStringToInputs(input));
        REQUIRE(expectedAccepted == isAccepted);
    }
}