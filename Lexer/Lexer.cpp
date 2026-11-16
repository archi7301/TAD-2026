#include "Lexer.h"
#include "FST.h"

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

	// Правило распознавания токена
	struct Rule
	{
		FST::FST fst;		// конечный автомат, распознающий токен
		char lexema;		// код распознанной лексемы
		char attr{LT::ATTR_NONE};			// дополнительный атрибут
	};

	static void AddWord(std::vector<Rule>& rules, const std::string& word, char lexema, char attr = LT::ATTR_NONE)
	{
		Rule rule;
		rule.fst = FST::MakeWord(word);
		rule.lexema = lexema;
		rule.attr = attr;
		rules.push_back(rule);
	}

	// Строит правила в порядке проверки.
	// Ключевые слова идут раньше общего правила идентификатора:
	// Classify возвращает первое совпадение.
	static std::vector<Rule> BuildRules()
	{
		std::vector<Rule> rules;

		// Добавляет правило точного совпадения с word:
		// автомат, код лексемы и дополнительный атрибут.
		AddWord(rules, "summon", LEX_SUMMON);
		AddWord(rules, "lore", LEX_LORE);
		AddWord(rules, "spell", LEX_SPELL);
		AddWord(rules, "ritual", LEX_RITUAL);
		AddWord(rules, "open", LEX_OPEN);
		AddWord(rules, "close", LEX_CLOSE);
		AddWord(rules, "rune", LEX_RUNE);
		AddWord(rules, "flux", LEX_FLUX);
		AddWord(rules, "chant", LEX_CHANT);
		AddWord(rules, "when", LEX_WHEN);
		AddWord(rules, "then", LEX_THEN);
		AddWord(rules, "else", LEX_ELSE);
		AddWord(rules, "cycle", LEX_CYCLE);
		AddWord(rules, "until", LEX_UNTIL);
		AddWord(rules, "octet", LEX_OCTET, LT::ATTR_OCTET);
		AddWord(rules, "truth", LEX_TRUTH, LT::ATTR_TRUTH);
		AddWord(rules, "yes", LEX_YES);
		AddWord(rules, "no", LEX_NO);
		AddWord(rules, "fuse", LEX_FUSE);
		AddWord(rules, "mask", LEX_MASK);
		AddWord(rules, "flip", LEX_FLIP);

		AddWord(rules, ";", LEX_SEMICOLON);
		AddWord(rules, ",", LEX_COMMA);
		AddWord(rules, "(", LEX_LEFTHESIS);
		AddWord(rules, ")", LEX_RIGHTHESIS);
		AddWord(rules, ":", LEX_COLON);
		AddWord(rules, "=", LEX_EQUALS);
		AddWord(rules, "+", LEX_PLUS);
		AddWord(rules, "-", LEX_MINUS);
		AddWord(rules, "*", LEX_STAR);
		AddWord(rules, "<", LEX_LEFT_ANGLE_BRACKET);
		AddWord(rules, ">", LEX_RIGHT_ANGLE_BRACKET);
		AddWord(rules, "->", LEX_RIGHT_ARROW);
		AddWord(rules, "<-", LEX_LEFT_ARROW);
		AddWord(rules, "~>", LEX_TILDE_ARROW);
		AddWord(rules, "==", LEX_EQUAL);
		AddWord(rules, "!=", LEX_NOT_EQUAL);

		// Идентификатор: [a-z][a-z0-9_]*.
		// Ограничение длины проверяет ValidateToken, чтобы выдать ошибку 103.
		{
			Rule rule;
			rule.fst = FST::Create(2);
			FST::AddRelation(rule.fst, 0, FST::ANY_LETTER, 1);
			FST::AddRelation(rule.fst, 1, FST::ANY_LETTER, 1);
			FST::AddRelation(rule.fst, 1, FST::ANY_DIGIT, 1);
			FST::AddRelation(rule.fst, 1, '_', 1);
			FST::SetFinal(rule.fst, 1);

			rule.lexema = LEX_ID;
			rules.push_back(rule);
		}

		// Целое без знака и ведущих нулей: 0 | [1-9][0-9]*.
		// Состояние 1 принимает только одиночный ноль,
		// состояние 2 позволяет продолжать число любыми цифрами.
		// Проверка значения на превышение 255 выполняется отдельно.
		{
			Rule rule;
			rule.fst = FST::Create(3);
			FST::AddRelation(rule.fst, 0, '0', 1);
			for (char ch = '1'; ch <= '9'; ++ch)
				FST::AddRelation(rule.fst, 0, ch, 2);
			FST::AddRelation(rule.fst, 2, FST::ANY_DIGIT, 2);
			FST::SetFinal(rule.fst, 1);
			FST::SetFinal(rule.fst, 2);

			rule.lexema = LEX_INT_LITERAL;
			rules.push_back(rule);

		}

		// Строка: открывающий апостроф, содержимое, закрывающий апостроф.
		// Петля в состоянии 1 допускает пустое содержимое.
		// Закрытие и длину строки отдельно проверяет ValidateToken.
		{
			Rule rule;
			rule.fst = FST::Create(3);
			FST::AddRelation(rule.fst, 0, '\'', 1);
			FST::AddRelation(rule.fst, 1, FST::STRING_CHAR, 1);
			FST::AddRelation(rule.fst, 1, '\'', 2);
			FST::SetFinal(rule.fst, 2);

			rule.lexema = LEX_STR_LITERAL;
			rules.push_back(rule);
		}

		return rules;
	}

	// Проверяет закрытие строкового литерала и ограничения длины.
	// При первой обнаруженной ошибке добавляет диагностику и возвращает false.
	// Допустимость написания токена затем проверяет Classify.
	static bool ValidateToken(const Token& token, LEX& result)
	{
		if (token.text.empty())
			return true;

		const char firstChar = token.text.front();

		// Один апостроф не образует закрытую строку:
		// нужны как минимум открывающий и закрывающий разделители.
		if (firstChar == '\'')
		{
			if (token.text.size() < 2 || token.text.back() != '\'')
			{
				result.errors.push_back(
					Error::geterrorin(101, token.line, token.pos)
				);
				return false;
			}

			// Апострофы не входят в длину содержимого.
			// Вычитание безопасно: наличие обоих разделителей уже проверено.
			const size_t contentLength = token.text.size() - 2;
			if (contentLength > TBC_MAX_STRING_SIZE)
			{
				result.errors.push_back(
					Error::geterrorin(104, token.line, token.pos)
				);
				return false;
			}

			return true;
		}

		if (firstChar >= 'a' && firstChar <= 'z')
		{
			if (token.text.size() > IT::ID_MAXSIZE)
			{
				result.errors.push_back(
					Error::geterrorin(103, token.line, token.pos)
				);
				return false;
			}

			return true;
		}

		return true;
	}

	// Возвращает указатель на первое подходящее правило или nullptr.
	// Указатель ссылается на элемент rules: вектор должен продолжать
	// существовать и не изменяться во время использования результата.
	static const Rule* Classify(const std::vector<Rule>& rules, const std::string& text)
	{
		for (const Rule& rule : rules)
		{
			if (FST::Execute(rule.fst, text))
				return &rule;
		}

		return nullptr;
	}

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

	// Выделяет один фрагмент и сохраняет координаты его начала.
	// true означает, что фрагмент получен, но ещё не признан допустимым.
	// false означает конец текста; token при этом использовать нельзя.
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

			// Незакрытый фрагмент тоже возвращаем как токен.
			// Перевод строки оставляем для следующего SkipWhitespace.
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