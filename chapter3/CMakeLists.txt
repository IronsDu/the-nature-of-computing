enable_testing()

include_directories("${PROJECT_SOURCE_DIR}/include/")

# Create the executable
add_executable(test_dfa test_dfa.cpp)
add_test(TestDFA test_dfa)

add_executable(test_nfa test_nfa.cpp)
add_test(TestNFA test_nfa)


add_executable(test_nfa2dfa test_nfa2dfa.cpp)
add_test(TestNFA2DFA test_nfa2dfa)

add_executable(test_nfa_concatenate test_nfa_concatenate.cpp)
add_test(TestNFAConcatenate test_nfa_concatenate)

add_executable(test_nfa_alternation test_nfa_alternation.cpp)
add_test(TestNFAAlternation test_nfa_alternation)

add_executable(test_nfa_repeat test_nfa_repeat.cpp)
add_test(TestNFARepeat test_nfa_repeat)


add_executable(test_nfa2graphviz test_nfa2graphviz.cpp)
add_test(TestNFA2Graphviz test_nfa2graphviz)

add_executable(test_regex_generate test_regex_generate.cpp)
add_test(TestRegexGenerate test_regex_generate)
