#pragma once

#include <string>

using GRBALPHABET = short;

namespace GRB
{
    struct Chain
    {
        short size;
        const GRBALPHABET* symbols;
    };

    struct Rule
    {
        GRBALPHABET left;
        int error;
        short size;
        const Chain* chains;
    };

    struct Greibach
    {
        short size;
        GRBALPHABET start;
        GRBALPHABET end;
        const Rule* rules;
    };

    Greibach getGreibach();
    short findRule(const Greibach& grammar, GRBALPHABET symbol);
    short findChain(const Rule& rule, GRBALPHABET token, short from);
    char symbolChar(GRBALPHABET symbol);
    std::string ruleText(const Rule& rule, short chainIndex);
}
