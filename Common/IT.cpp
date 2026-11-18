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

	int FindId(const IdTable& idtable, const std::string& scope, const std::string& name)
	{
		std::string curr_scope = scope;
		while (true)
		{
			int idx = IsId(idtable, curr_scope, name);
			if (idx != TI_NULLIDX)
				return idx;

			if (curr_scope.empty())
				break;

			size_t pos = curr_scope.rfind('/');
			if (pos == std::string::npos)
				curr_scope.clear();
			else
				curr_scope.erase(pos);
		}

		return TI_NULLIDX;
	}

	int FindLiteral(const IdTable& idtable, ID_DATA_TYPE type, long long value)
	{
		for (size_t i = 0; i < idtable.table.size(); ++i)
		{
			const Entry& entry = idtable.table[i];
			if (entry.id_data_type == type && entry.value == value && entry.id_type == ID_TYPE::LITERAL)
				return static_cast<int>(i);
		}
		return TI_NULLIDX;
	}

	int FindLiteral(const IdTable& idtable, const std::string& str)
	{
		for (size_t i = 0; i < idtable.table.size(); ++i)
		{
			const Entry& entry = idtable.table[i];
			if (entry.id_data_type == ID_DATA_TYPE::STR && entry.str == str && entry.id_type == ID_TYPE::LITERAL)
				return static_cast<int>(i);
		}
		return TI_NULLIDX;
	}
}