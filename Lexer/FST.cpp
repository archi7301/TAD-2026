#include "FST.h"

namespace FST
{
	FST Create(size_t stateCount)
	{
		FST fst;
		fst.states.resize(stateCount);
		fst.isFinal.resize(stateCount, false);
		return fst;
	}

	void AddRelation(FST& fst, size_t from, char symbol, size_t to)
	{
		fst.states[from].push_back({ symbol, to });
	}

	void SetFinal(FST& fst, size_t state)
	{
		fst.isFinal[state] = true;
	}

	FST MakeWord(const std::string& word)
	{
		const size_t len = word.length();
		FST fst = Create(len + 1);

		for (size_t i{}; i < len; ++i)
		{
			AddRelation(fst, i, word[i], i + 1);
		}

		SetFinal(fst, len);
		return fst;
	}

	static bool Match(char symbol, char ch)
	{
		switch (symbol)
		{
			case ANY_LETTER:
				return ch >= 'a' && ch <= 'z';

			case ANY_DIGIT:
				return ch >= '0' && ch <= '9';

			case STRING_CHAR:
				return ch >= ' ' && ch <= '~' && ch != '\'';

			default:
				return symbol == ch;
		}
	}

	bool Execute(const FST& fst, const std::string& text)
	{
		if (fst.states.empty())
			return false;

		std::vector<bool> currStates(fst.states.size(), false);
		currStates[0] = true;

		for (const char ch : text)
		{
			std::vector<bool> nextStates(fst.states.size(), false);
			for (size_t i{}; i < fst.states.size(); ++i)
			{
				if (!currStates[i])
					continue;

				for (const Relation& rel : fst.states[i])
				{
					if (Match(rel.symbol, ch))
						nextStates[rel.next] = true;
				}
			}

			currStates.swap(nextStates);
		}

		for (size_t i{}; i < fst.states.size(); ++i)
		{
			if (currStates[i] && fst.isFinal[i])
				return true;
		}

		return false;
	}
}