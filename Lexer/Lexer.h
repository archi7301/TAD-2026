#pragma once
#include <vector>

#include "../Common/In.h"
#include "../Common/LT.h"
#include "../Common/IT.h"
#include "../Common/Error.h"

namespace Lexer
{
    struct LEX
    {
        LT::LexTable lexTable;
        IT::IdTable idTable;
        std::vector<Error::Entry> errors;
    };

    LEX Analyze(const In::IN& in);
}
