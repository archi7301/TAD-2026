#pragma once

#include "GRB.h"
#include "../Common/Error.h"
#include "../Common/LT.h"

#include <array>
#include <cstddef>
#include <iosfwd>
#include <optional>
#include <vector>

namespace MFST
{
    struct AppliedRule
    {
        std::size_t position;
        short rule;
        short chain;
    };

    struct Result
    {
        bool accepted = false;
        std::vector<AppliedRule> rules;
        std::array<std::optional<Error::Entry>, 3> errors{};
    };

    Result parse(const LT::LexTable& lexTable, const GRB::Greibach& grammar,
                 std::ostream* trace = nullptr);
    void writeRules(const Result& result, const GRB::Greibach& grammar, std::ostream& output);
}
