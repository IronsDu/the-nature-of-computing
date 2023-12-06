enable_testing()

include_directories("${PROJECT_SOURCE_DIR}/include/")

# Create the executable
add_executable(test_dfa test_dfa.cpp)
add_test(TestDFA test_dfa)

add_executable(test_nfa test_nfa.cpp)
add_test(TestNFA test_nfa)