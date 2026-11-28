#include "FST.h"

#include <algorithm>

namespace FST
{
    FST Create(std::size_t stateCount)
    {
        FST fst;
        fst.states.resize(stateCount);
        fst.isFinal.resize(stateCount, false);
        return fst;
    }

    void AddRelation(FST& fst, std::size_t from, char symbol, std::size_t to)
    {
        fst.states[from].push_back({ symbol, to });
    }

    void SetFinal(FST& fst, std::size_t state)
    {
        fst.isFinal[state] = true;
    }

    FST MakeWord(const std::string& word)
    {
        const std::size_t len = word.size();
        FST fst = Create(len + 1);

        for (std::size_t i = 0; i < len; ++i)
            AddRelation(fst, i, word[i], i + 1);

        SetFinal(fst, len);
        return fst;
    }

    static bool Match(char symbol, char ch)
    {
        switch (symbol)
        {
        case ANY_LETTER:  return ch >= 'a' && ch <= 'z';
        case ANY_DIGIT:   return ch >= '0' && ch <= '9';
        case STRING_CHAR: return ch >= ' ' && ch <= '~' && ch != '\'';
        default:          return symbol == ch;
        }
    }

    bool Execute(const FST& fst, const std::string& text)
    {
        const std::size_t count = fst.states.size();
        if (count == 0)
            return false;

        std::vector<bool> current(count, false);
        std::vector<bool> next(count, false);
        current[0] = true;

        // Переходы по одному символу строят новый набор состояний.
        for (char ch : text)
        {
            std::fill(next.begin(), next.end(), false);
            bool matched = false;

            for (std::size_t i = 0; i < count; ++i)
            {
                if (!current[i])
                    continue;

                for (const Relation& rel : fst.states[i])
                {
                    if (Match(rel.symbol, ch))
                    {
                        next[rel.next] = true;
                        matched = true;
                    }
                }
            }

            if (!matched)
                return false;

            current.swap(next);
        }

        for (std::size_t i = 0; i < count; ++i)
        {
            if (current[i] && fst.isFinal[i])
                return true;
        }

        return false;
    }
}