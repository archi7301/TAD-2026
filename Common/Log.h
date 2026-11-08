#pragma once

#include "stdadx.h"
#include "Error.h"
#include "Parm.h"
#include "In.h"

// Название транслятора для заголовка протокола
#define LOG_TITLE         "Протокол трансляции TAD-2026"

// Ширина линии-разделителя (в символах)
#define LOG_LINE_WIDTH    60

// Названия разделов протокола (таблица 2.3 пояснительной записки)
#define LOG_SECT_SOURCE   "Исходный текст"
#define LOG_SECT_LEXEMS   "Таблица лексем"
#define LOG_SECT_IDS      "Таблица идентификаторов"
#define LOG_SECT_TREE     "Дерево разбора"
#define LOG_SECT_RPN      "Выражения в обратной польской записи"
#define LOG_SECT_BYTECODE "Листинг байт-кода"
#define LOG_SECT_ERRORS   "Диагностические сообщения"

namespace Log
{
	// Протокол трансляции: текстовый файл в кодировке UTF-8
	struct LOG
	{
		std::ofstream file;   // открытый файл протокола (nullptr, если закрыт)
	};

	// Открывает файл протокола на запись (прежнее содержимое затирается).
	// Если файл невозможно создать, бросает Error::Entry (код 3).
	LOG Open(const wchar_t* path);

	// Закрывает протокол и обнуляет log.file. Повторный вызов безопасен.
	// Вызывается на всех путях выхода из программы после успешного Open.
	void Close(LOG& log);

	// Пишет заголовок протокола: название, дату и время запуска,
	// итоговые значения параметров и линию-разделитель.
	void WriteHeader(LOG& log, const Parm::PARM& parm);

	// Пишет название раздела протокола, линию-разделитель и пустую строку.
	// Название берётся из макросов LOG_SECT_*.
	void WriteSection(LOG& log, const char* title);

	// Пишет строку текста (UTF-8) и перевод строки.
	void WriteLine(LOG& log, const char* text);

	// Пишет исходный текст построчно с номерами строк.
	// Число строк берётся из in.lines.
	void WriteSource(LOG& log, const In::IN& in);

	// Пишет сообщение об ошибке в том же формате, что и в консоли.
	void WriteError(LOG& log, const Error::Entry& e);

	// Сбрасывает буфер на диск; вызывается после каждого раздела.
	void Flush(LOG& log);
}