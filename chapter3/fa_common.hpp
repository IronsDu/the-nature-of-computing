#pragma once

#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

using State = std::string;
using InputType = char;

static std::list<InputType> convertStringToInputs(const std::string& str)
{
    std::list<InputType> inputs;
    for (char const c : str)
    {
        inputs.push_back(c);
    }
    return inputs;
}

static std::string convertCombinationStateToString(const std::set<State>& stateSet)
{
    std::string str;
    // 必须依靠稳定顺序
    for (const auto& s : stateSet)
    {
        str += s;
    }
    return str;
}

// 将一个状态组合转换为一个字符串，通常，这个字符串作为构造新的状态机中的新状态
static std::string convertCombinationStateToString(const std::vector<State>& stateSet)
{
    std::string str;
    // 必须依靠稳定顺序
    for (const auto& s : stateSet)
    {
        str += s;
    }
    return str;
}