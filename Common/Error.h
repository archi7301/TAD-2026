#pragma once

// Без точки с запятой внутри: использование  throw ERROR_THROW(1);
#define ERROR_THROW(id)           Error::geterror(id)
#define ERROR_THROW_IN(id, l, c)  Error::geterrorin(id, l, c)

#define ERROR_ENTRY(id, m) {id, m, {-1, -1}}
#define ERROR_MAXSIZE_MESSAGE 200

namespace Error
{
	// Запись об ошибке
	struct Entry
	{
		int id;                                  // код ошибки
		char message[ERROR_MAXSIZE_MESSAGE];     // текст сообщения

		struct IN
		{
			short line;                          // строка в исходном тексте (-1, если неприменимо)
			short col;                           // позиция в строке (-1, если неприменимо)
		} inext;
	};

	// Возвращает ошибку по коду; для неизвестного кода возвращает ошибку 0
	Entry geterror(int id);

	// То же, но с указанием позиции в исходном тексте
	Entry geterrorin(int id, int line = -1, int col = -1);
}
