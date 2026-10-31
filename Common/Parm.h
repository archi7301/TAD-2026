#pragma once

// Ключи командной строки (формат: -in:файл)
#define PARM_IN  L"-in:"
#define PARM_OUT L"-out:"
#define PARM_LOG L"-log:"

// Ключ без значения: выполнить байт-код виртуальной машиной после трансляции
#define PARM_RUN L"-run"

#define PARM_MAX_SIZE 260                 // максимальная длина значения параметра (с завершающим нулём)

// Расширения по умолчанию: заменяют расширение входного файла (или дописываются, если его нет)
#define PARM_OUT_DEFAULT_EXT L".tbc"      // если -out не задан
#define PARM_LOG_DEFAULT_EXT L".log"      // если -log не задан

namespace Parm
{
	// Параметры, указанные в командной строке
	struct PARM
	{
		wchar_t in[PARM_MAX_SIZE];        // входной файл с исходным кодом
		wchar_t out[PARM_MAX_SIZE];       // выходной файл с байт-кодом
		wchar_t log[PARM_MAX_SIZE];       // файл протокола
		bool run;                         // выполнить байт-код после трансляции (-run)
	};

	// Разбирает параметры командной строки.
	// При ошибке бросает Error::Entry (коды 1, 5, 6).
	PARM GetParm(int argc, wchar_t* argv[]);
}