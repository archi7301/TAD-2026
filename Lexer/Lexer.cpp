#include "Lexer.h"
#include "FST.h"
#include "../Common/Bytecode.h"

#include <cstddef>
#include <string>
#include <utility>

namespace Lexer
{
    struct Token
    {
        std::string text;
        int line;
        int pos;
    };

    struct Cursor
    {
        std::size_t index{};
        int line{ 1 };
        int pos{ 1 };
    };

    struct Rule
    {
        FST::FST fst;
        char lexema;
        char attr = LT::ATTR_NONE;
    };

    static void AddWord(std::vector<Rule>& rules, const std::string& word, char lexema, char attr = LT::ATTR_NONE)
    {
        rules.push_back({ FST::MakeWord(word), lexema, attr });
    }

    static std::vector<Rule> BuildRules()
    {
        std::vector<Rule> rules;

        // Classify выбирает первое совпадение, поэтому ключевые слова идут до идентификатора.
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

        {
            Rule rule;
            rule.fst = FST::Create(2);
            FST::AddRelation(rule.fst, 0, FST::ANY_LETTER, 1);
            FST::AddRelation(rule.fst, 1, FST::ANY_LETTER, 1);
            FST::AddRelation(rule.fst, 1, FST::ANY_DIGIT, 1);
            FST::AddRelation(rule.fst, 1, '_', 1);
            FST::SetFinal(rule.fst, 1);

            rule.lexema = LEX_ID;
            rules.push_back(std::move(rule));
        }

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
            rules.push_back(std::move(rule));
        }

        {
            Rule rule;
            rule.fst = FST::Create(3);
            FST::AddRelation(rule.fst, 0, '\'', 1);
            FST::AddRelation(rule.fst, 1, FST::STRING_CHAR, 1);
            FST::AddRelation(rule.fst, 1, '\'', 2);
            FST::SetFinal(rule.fst, 2);

            rule.lexema = LEX_STR_LITERAL;
            rules.push_back(std::move(rule));
        }

        return rules;
    }

    static int TokenError(const Token& token)
    {
        const char first = token.text.front();
        const std::size_t length = token.text.size();

        if (first == '\'')
        {
            if (length < 2 || token.text.back() != '\'')
                return 101;

            if (length - 2 > TBC_MAX_STRING_SIZE)
                return 104;
        }
        else if (first >= 'a' && first <= 'z' && length > IT::ID_MAXSIZE)
        {
            return 103;
        }

        return 0;
    }

    static const Rule* Classify(const std::vector<Rule>& rules, const std::string& text)
    {
        for (const Rule& rule : rules)
        {
            if (FST::Execute(rule.fst, text))
                return &rule;
        }

        return nullptr;
    }

    static void Advance(const std::string& text, Cursor& cursor)
    {
        if (text[cursor.index] == '\n')
        {
            cursor.line++;
            cursor.pos = 1;
        }
        else
            cursor.pos++;

        cursor.index++;
    }

    static void SkipWhitespace(const std::string& text, Cursor& cursor)
    {
        while (cursor.index < text.size())
        {
            const char ch = text[cursor.index];
            if (ch == ' ' || ch == '\t' || ch == '\n')
                Advance(text, cursor);
            else
                break;
        }
    }

    static bool IsWordChar(char ch)
    {
        return (ch >= 'a' && ch <= 'z') ||
               (ch >= '0' && ch <= '9') ||
               ch == '_';
    }

    static bool IsDoubleSymbol(char first, char second)
    {
        return (first == '-' && second == '>') ||
            (first == '<' && second == '-') ||
            (first == '~' && second == '>') ||
            (first == '=' && second == '=') ||
            (first == '!' && second == '=');
    }

    static bool NextToken(const std::string& text, Cursor& cursor, Token& token)
    {
        SkipWhitespace(text, cursor);
        if (cursor.index == text.size())
            return false;

        const std::size_t start = cursor.index;
        token.line = cursor.line;
        token.pos = cursor.pos;

        if (text[start] == '\'')
        {
            Advance(text, cursor);

            // Незакрытая строка заканчивается перед переводом строки или на конце файла.
            while (cursor.index < text.size() && text[cursor.index] != '\'' && text[cursor.index] != '\n')
                Advance(text, cursor);

            if (cursor.index < text.size() && text[cursor.index] == '\'')
                Advance(text, cursor);
        }
        else if (IsWordChar(text[start]))
        {
            while (cursor.index < text.size() && IsWordChar(text[cursor.index]))
                Advance(text, cursor);
        }
        else
        {
            Advance(text, cursor);
            if (cursor.index < text.size() && IsDoubleSymbol(text[start], text[cursor.index]))
                Advance(text, cursor);
        }

        token.text.assign(text, start, cursor.index - start);
        return true;
    }

    static int SaveIdEntry(LEX& result, const Token& token, char lexema)
    {
        if (lexema != LEX_ID && lexema != LEX_STR_LITERAL && lexema != LEX_INT_LITERAL)
            return LT::LT_TI_NULLIDX;

        IT::Entry entry{};
        int index = IT::TI_NULLIDX;

        if (lexema == LEX_ID)
        {
            entry.name = token.text;
        }
        else if (lexema == LEX_STR_LITERAL)
        {
            entry.id_type = IT::ID_TYPE::LITERAL;
            entry.id_data_type = IT::ID_DATA_TYPE::STR;
            entry.str = token.text.substr(1, token.text.size() - 2);
            index = IT::FindLiteral(result.idTable, entry.str);
        }
        else
        {
            entry.id_type = IT::ID_TYPE::LITERAL;
            // yes и no имеют тот же код лексемы, что и целые числа.
            if (token.text == "yes" || token.text == "no")
            {
                entry.id_data_type = IT::ID_DATA_TYPE::TRUTH;
                entry.value = (token.text == "yes") ? 1 : 0;
            }
            else
            {
                entry.id_data_type = IT::ID_DATA_TYPE::OCTET;
                for (char ch : token.text)
                {
                    entry.value = entry.value * 10 + (ch - '0');
                    if (entry.value > 255)
                    {
                        // 256 означает выход за диапазон для семантической проверки.
                        entry.value = 256;
                        break;
                    }
                }
            }
            index = IT::FindLiteral(result.idTable, entry.id_data_type, entry.value);
        }

        if (index == IT::TI_NULLIDX)
        {
            index = static_cast<int>(result.idTable.table.size());
            entry.idx_first_LE = static_cast<int>(result.lexTable.table.size());
            IT::Add(result.idTable, std::move(entry));
        }

        return index;
    }

    LEX Analyze(const In::IN& in)
    {
        static const auto rules = BuildRules();
        LEX result{ LT::Create(), IT::Create(), {} };
        Cursor cursor;
        Token token;

        while (NextToken(in.text, cursor, token))
        {
            int error = TokenError(token);
            const Rule* rule = nullptr;
            if (error == 0)
            {
                rule = Classify(rules, token.text);
                if (rule == nullptr)
                    error = 102;
            }

            if (error != 0)
            {
                result.errors.push_back(Error::geterrorin(error, token.line, token.pos));
                continue;
            }

            LT::Entry entry{};
            entry.lexema = rule->lexema;
            entry.attr = rule->attr;
            entry.sn = token.line;
            entry.pos = token.pos;
            entry.idxTI = SaveIdEntry(result, token, entry.lexema);
            LT::Add(result.lexTable, entry);
        }

        return result;
    }
}
