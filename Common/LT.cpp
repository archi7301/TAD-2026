#include "stdadx.h"
#include "LT.h"

namespace LT
{
	LexTable Create(size_t reserve_size)
	{
		LexTable lex_table;
		lex_table.table.reserve(reserve_size);
		return lex_table;
	}

	void Add(LexTable& lex_table, const Entry& entry)
	{
		lex_table.table.push_back(entry);
	}

	const Entry& GetEntry(const LexTable& lex_table, int n)
	{
		return lex_table.table[n];
	}
}