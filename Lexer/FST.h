#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace FST
{

    constexpr char ANY_LETTER = '\x01';
    constexpr char ANY_DIGIT = '\x02';
    constexpr char STRING_CHAR = '\x03';

    struct Relation
    {
        char symbol;
        std::size_t next;
    };

    // Состояние 0 — начальное.
    struct FST
    {

        std::vector<std::vector<Relation>> states;

        std::vector<bool> isFinal;
    };

    FST Create(std::size_t stateCount);

    void AddRelation(FST& fst, std::size_t from, char symbol, std::size_t to);

    void SetFinal(FST& fst, std::size_t state);

    FST MakeWord(const std::string& word);

    bool Execute(const FST& fst, const std::string& text);
}
