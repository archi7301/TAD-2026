#include "stdadx.h"
#include "Parm.h"
#include "Error.h"

namespace Parm
{
	// Строит имя выходного файла: берёт имя src, заменяет его расширение на ext
	// (если расширения нет, ext дописывается) и записывает результат в dest.
	// Если результат не помещается в буфер, бросает ошибку 6.
	// src короче PARM_MAX_SIZE, поэтому копирование безопасно.
	// wcsncat_s с _TRUNCATE не вызывает аварийное завершение, а возвращает STRUNCATE.
	static void MakeName(wchar_t* dest, const wchar_t* src, const wchar_t* ext)
	{
		wcscpy_s(dest, PARM_MAX_SIZE, src);

		// последний разделитель пути: точка в имени каталога расширением не считается
		const wchar_t* sep = wcsrchr(dest, L'\\');
		const wchar_t* sep2 = wcsrchr(dest, L'/');
		if (sep == nullptr || (sep2 != nullptr && sep2 > sep))
			sep = sep2;

		wchar_t* dot = wcsrchr(dest, L'.');
		if (dot != nullptr && (sep == nullptr || dot > sep))
			*dot = L'\0';                       // отбрасываем старое расширение

		if (wcsncat_s(dest, PARM_MAX_SIZE, ext, _TRUNCATE) == STRUNCATE)
			throw ERROR_THROW(6);               // превышена длина параметра
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
		parm.out[0] = L'\0';
		parm.log[0] = L'\0';
		parm.run = false;

		for (int i = 1; i < argc; i++)
		{
			// длина аргумента меньше PARM_MAX_SIZE, значит значение после ключа
			// гарантированно помещается в буфер
			if (wcslen(argv[i]) >= PARM_MAX_SIZE)
				throw ERROR_THROW(6);   // превышена длина параметра

			if (wcscmp(argv[i], PARM_RUN) == 0)          // ключ без значения: сравнение целиком
				parm.run = true;
			else if (StartsWith(argv[i], PARM_IN))
				wcscpy_s(parm.in, PARM_MAX_SIZE, argv[i] + wcslen(PARM_IN));
			else if (StartsWith(argv[i], PARM_OUT))
				wcscpy_s(parm.out, PARM_MAX_SIZE, argv[i] + wcslen(PARM_OUT));
			else if (StartsWith(argv[i], PARM_LOG))
				wcscpy_s(parm.log, PARM_MAX_SIZE, argv[i] + wcslen(PARM_LOG));
			else
				throw ERROR_THROW(5);   // неизвестный параметр командной строки
		}

		if (parm.in[0] == L'\0')
			throw ERROR_THROW(1);       // не указан обязательный параметр -in

		if (parm.out[0] == L'\0')
			MakeName(parm.out, parm.in, PARM_OUT_DEFAULT_EXT);

		if (parm.log[0] == L'\0')
			MakeName(parm.log, parm.in, PARM_LOG_DEFAULT_EXT);

		return parm;
	}
}