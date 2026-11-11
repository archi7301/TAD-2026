#include "stdadx.h"
#include "IT.h"

namespace IT
{
	IdTable Create(size_t reserve_size)
	{
		IdTable id_table;
		id_table.table.reserve(reserve_size);
		return id_table;
	}

	void Add(IdTable& idtable, Entry entry)
	{
		idtable.table.push_back(entry);
	}

	const Entry& GetEntry(const IdTable& idtable, int n)
	{
		return idtable.table[n];
	}

	int IsId(const IdTable& idtable, const std::string& scope, const std::string& name)
	{
		for (size_t i = 0; i < idtable.table.size(); ++i)
		{
			const Entry& entry = idtable.table[i];

			if (entry.scope == scope && entry.name == name)
				return static_cast<int>(i);
		}

		return TI_NULLIDX;
	}
}