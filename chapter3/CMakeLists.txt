enable_testing()

include_directories("${PROJECT_SOURCE_DIR}/include/")

# Create the executable
add_executable(test_dfa test_dfa.cpp)
add_test(TestDFA test_dfa)

add_executable(test_nfa test_nfa.cpp)
add_test(TestNFA test_nfa)


add_executable(test_nfa2dfa test_nfa2dfa.cpp)
add_test(TestNFA2DFA test_nfa2dfa)
