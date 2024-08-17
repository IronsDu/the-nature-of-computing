enable_testing()

# Create the executable
add_executable(test_dfa test_dfa.cpp)
target_link_libraries(test_dfa PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestDFA test_dfa)

add_executable(test_nfa test_nfa.cpp)
target_link_libraries(test_nfa PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestNFA test_nfa)


add_executable(test_nfa2dfa test_nfa2dfa.cpp)
target_link_libraries(test_nfa2dfa PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestNFA2DFA test_nfa2dfa)

add_executable(test_nfa_concatenate test_nfa_concatenate.cpp)
target_link_libraries(test_nfa_concatenate PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestNFAConcatenate test_nfa_concatenate)

add_executable(test_nfa_alternation test_nfa_alternation.cpp)
target_link_libraries(test_nfa_alternation PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestNFAAlternation test_nfa_alternation)

add_executable(test_nfa_repeat test_nfa_repeat.cpp)
target_link_libraries(test_nfa_repeat PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestNFARepeat test_nfa_repeat)


add_executable(test_nfa2graphviz test_nfa2graphviz.cpp)
target_link_libraries(test_nfa2graphviz PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestNFA2Graphviz test_nfa2graphviz)

add_executable(test_regex_generate test_regex_generate.cpp)
target_link_libraries(test_regex_generate PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
add_test(TestRegexGenerate test_regex_generate)
