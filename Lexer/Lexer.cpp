#include "Lexer.h"

namespace Lexer
{
	// Один выделенный фрагмент, который ещё предстоит распознать
	struct Token
	{
		std::string text;	// написание токена, например flux или <-
		int line;			// номер строки, в которой встречается токен
		int pos;			// позиция токена в строке
	};

	// Текущее положение сканера в тексте, который он анализирует
	struct Cursor
	{
		size_t idx{};		// индекс байта в in.text, на котором находится сканер
		int line{ 1 };		// номер текущей строки
		int pos{ 1 };		// позиция в текущей строке
	};

	// Продвинуть курсор с текущего символа на следущий, 
	// обноивляя при этом номер строки и позицию в строке
	static void Advance(const std::string& text, Cursor& cursor)
	{
		if (cursor.idx >= text.size())
			return;

		if (text[cursor.idx] == '\n')
		{
			cursor.line++;
			cursor.pos = 1;
		}
		else
			cursor.pos++;

		cursor.idx++;
	}

	// Пропускает пробелы, '\t' и '\n', вызывая Advance
	// Останавливается перед первым непробельным символом или в конце текста
	static void SkipWhitespace(const std::string& text, Cursor& cursor)
	{
		while (cursor.idx < text.size())
		{
			const char ch = text[cursor.idx];
			if (ch == ' ' || ch == '\t' || ch == '\n')
				Advance(text, cursor);
			else
				break;
		}
	}

	// Возвращает true, если символ ch может быть частью идентификатора (токена-слова)
	static bool IsWordChar(char ch)
	{
		return (ch >= 'a' && ch <= 'z') ||
			   (ch >= '0' && ch <= '9') ||
			   ch == '_';
	}

	// Вспомогательная проверка для двухсимвольных знаков
	static bool IsDoubleSymbol(char first, char second)
	{
		return (first == '-' && second == '>') ||
			(first == '<' && second == '-') ||
			(first == '~' && second == '>') ||
			(first == '=' && second == '=') ||
			(first == '!' && second == '=');
	}

	// Возвращает true, если удалось выделить следующий токен из текста, начиная с позиции курсора
	static bool NextToken(const std::string& text, Cursor& cursor, Token& token)
	{

		SkipWhitespace(text, cursor);

		if (cursor.idx >= text.size())
			return false;
		
		token.text.clear();
		token.line = cursor.line;
		token.pos = cursor.pos;

		// 1. Строковые литералы, заключённые в одинарные кавычки
		if (text[cursor.idx] == '\'')
		{
			token.text.push_back(text[cursor.idx]);
			Advance(text, cursor);

			while (cursor.idx < text.size() && text[cursor.idx] != '\'' && text[cursor.idx] != '\n')
			{
				token.text.push_back(text[cursor.idx]);
				Advance(text, cursor);
			}

			if (cursor.idx < text.size() && text[cursor.idx] == '\'')
			{
				token.text.push_back(text[cursor.idx]);
				Advance(text, cursor);
			}
		}

		// 2. Словоподобные фрагменты
		else if (IsWordChar(text[cursor.idx]))
		{
			while (cursor.idx < text.size() && IsWordChar(text[cursor.idx]))
			{
				token.text.push_back(text[cursor.idx]);
				Advance(text, cursor);
			}
		}

		// 3. Двухсимвольные знаки
		else if (cursor.idx + 1 < text.size() && IsDoubleSymbol(text[cursor.idx], text[cursor.idx + 1]))
		{
			token.text.push_back(text[cursor.idx]);
			Advance(text, cursor);

			token.text.push_back(text[cursor.idx]);
			Advance(text, cursor);
		}

		// 4. Одиночные символы
		else
		{
			token.text.push_back(text[cursor.idx]);
			Advance(text, cursor);
		}

		return true;
	}
}