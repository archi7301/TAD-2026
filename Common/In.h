#pragma once

#include "stdadx.h"
#include "Error.h"

#ifdef IN
#undef IN
#endif


// Максимальное число ошибок, которое накапливает модуль ввода.
#define IN_MAX_ERRORS 10

namespace In
{
	// Результат чтения и проверки исходного текста
	struct IN
	{
		std::string text;                    // очищенный текст: без комментариев и символов CR
		int lines;                           // число строк в исходном файле (пустой файл: 1)
		std::vector<Error::Entry> errors;    // найденные ошибки (код 100), не более IN_MAX_ERRORS
	};

	// Читает входной файл в кодировке Windows-1251, проверяет символы по таблице,
	// удаляет комментарии и символы CR. Ошибки допустимости символов (100)
	// не бросает, а накапливает в IN::errors.
	// Если файл невозможно открыть, бросает Error::Entry (код 2).
	IN GetIn(const wchar_t* path);
}