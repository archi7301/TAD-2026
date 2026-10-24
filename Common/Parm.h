#pragma once

// Ключи командной строки (формат: -in:файл)
#define PARM_IN  L"-in:"
#define PARM_LOG L"-log:"
#define PARM_OUT L"-out:"

#define PARM_MAX_SIZE 260                 // максимальная длина значения параметра (с завершающим нулём)
#define PARM_LOG_DEFAULT_EXT L".log"      // дописывается к имени входного файла, если -log не задан
#define PARM_OUT_DEFAULT_EXT L".js"       // дописывается к имени входного файла, если -out не задан

namespace Parm
{
	// Пути к файлам, указанным в командной строке
	struct PARM
	{
		wchar_t in[PARM_MAX_SIZE];        // входной файл с исходным кодом
		wchar_t log[PARM_MAX_SIZE];       // файл протокола
		wchar_t out[PARM_MAX_SIZE];       // выходной файл (JavaScript)
	};

	// Разбирает параметры командной строки.
	// При ошибке бросает Error::Entry (коды 1, 5, 6).
	PARM GetParm(int argc, wchar_t* argv[]);
}
