#pragma once

#include <atomic>
#include <exception>
#include <string>
#include <string_view>

#include "nfa_alternation.hpp"
#include "nfa_concatenate.hpp"
#include "nfa_repeat.hpp"

namespace regex_generate {

static std::atomic_int stateSeq;

static NFA Empty()
{
    auto seq = stateSeq.fetch_add(1);
    auto seqStr = std::to_string(seq);

    std::vector<NFARule> const rules = {
            {seqStr + "initial", std::nullopt, seqStr + "final"},
    };
    NFAAcceptStates const acceptState({seqStr + "final"});

    NFA const nfa(seqStr + "initial", rules, acceptState);
    return nfa;
}

// 构造接受一个字符c的状态机
static NFA Symbol(char c)
{
    auto seq = stateSeq.fetch_add(1);
    auto seqStr = std::to_string(seq);
    std::vector<NFARule> const rules = {
            {seqStr + "initial", c, seqStr + "final"},
    };
    NFAAcceptStates const acceptState({seqStr + "final"});

    NFA const nfa(seqStr + "initial", rules, acceptState);
    return nfa;
}

// 构造一个接受接受[start, end]区间内任意字符的状态机, 即： start | .. | end
static NFA Range(char start, char end)
{
    if (start > end)
    {
        throw std::exception("the range of start can't greater than end");
    }
    else if (start == end)
    {
        return Symbol(end);
    }
    else
    {
        return nfa_operator::alternation(Symbol(start), Range(start + 1, end));
    }
}

// 构造接受字符串str的状态机
static NFA Str(std::string_view str)
{
    if (str.empty())
    {
        return Empty();
    }
    return nfa_operator::concatenate(Symbol(str.front()), Str(str.substr(1, std::string_view::npos)));
}
}// namespace regex_generate