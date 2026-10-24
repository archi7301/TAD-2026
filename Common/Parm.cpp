#include "stdadx.h"
#include "Parm.h"
#include "Error.h"

namespace Parm
{
	// Дописывает расширение к строке; если не помещается в буфер, бросает ошибку 6.
	// wcsncat_s с _TRUNCATE не вызывает аварийное завершение, а возвращает STRUNCATE.
	static void AppendExt(wchar_t* dest, const wchar_t* ext)
	{
		if (wcsncat_s(dest, PARM_MAX_SIZE, ext, _TRUNCATE) == STRUNCATE)
			throw ERROR_THROW(6);   // превышена длина параметра
	}

	// Проверяет, начинается ли строка с заданного ключа
	static bool StartsWith(const wchar_t* str, const wchar_t* key)
	{
		return wcsncmp(str, key, wcslen(key)) == 0;
	}

	PARM GetParm(int argc, wchar_t* argv[])
	{
		PARM parm;
		parm.in[0] = L'\0';
		parm.log[0] = L'\0';
		parm.out[0] = L'\0';

		for (int i = 1; i < argc; i++)
		{
			// длина аргумента меньше PARM_MAX_SIZE, значит значение после ключа
			// гарантированно помещается в буфер
			if (wcslen(argv[i]) >= PARM_MAX_SIZE)
				throw ERROR_THROW(6);   // превышена длина параметра

			if (StartsWith(argv[i], PARM_IN))
				wcscpy_s(parm.in, PARM_MAX_SIZE, argv[i] + wcslen(PARM_IN));
			else if (StartsWith(argv[i], PARM_LOG))
				wcscpy_s(parm.log, PARM_MAX_SIZE, argv[i] + wcslen(PARM_LOG));
			else if (StartsWith(argv[i], PARM_OUT))
				wcscpy_s(parm.out, PARM_MAX_SIZE, argv[i] + wcslen(PARM_OUT));
			else
				throw ERROR_THROW(5);   // неизвестный параметр командной строки
		}

		if (parm.in[0] == L'\0')
			throw ERROR_THROW(1);       // не указан обязательный параметр -in

		if (parm.log[0] == L'\0')
		{
			wcscpy_s(parm.log, PARM_MAX_SIZE, parm.in);
			AppendExt(parm.log, PARM_LOG_DEFAULT_EXT);
		}

		if (parm.out[0] == L'\0')
		{
			wcscpy_s(parm.out, PARM_MAX_SIZE, parm.in);
			AppendExt(parm.out, PARM_OUT_DEFAULT_EXT);
		}

		return parm;
	}
}
