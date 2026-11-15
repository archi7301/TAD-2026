#pragma once
#include "../Common/stdadx.h"
#include "../Common/In.h"
#include "../Common/LT.h"
#include "../Common/IT.h"
#include "../Common/Error.h"

namespace Lexer
{
	struct LEX
	{
		LT::LexTable lexTable;	// таблица лексем (ТЛ)
		IT::IdTable idTable;	// таблица идентификаторов (ТИ)
		std::vector<Error::Entry> errors;	// список ошибок
	};

	LEX Analyze(const In::IN& in);	// выполнить лексический анализ
}