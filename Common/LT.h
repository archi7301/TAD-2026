#pragma once

#include "stdadx.h"

// Лексемы для ключевых слов
#define LEX_SUMMON	'm'		// лексема для summon
#define LEX_LORE	'o'		// лексема для lore
#define LEX_SPELL	'f'		// лексема для spell
#define LEX_RITUAL	'r'		// лексема для ritual
#define LEX_OPEN	'{'		// лексема для open
#define LEX_CLOSE	'}'		// лексема для close
#define LEX_RUNE	'c'		// лексема для rune
#define LEX_FLUX	'v'		// лексема для flux
#define LEX_CHANT	'w'		// лексема для chant
#define LEX_WHEN	'q'		// лексема для when
#define LEX_THEN	'h'		// лексема для then
#define LEX_ELSE	'e'		// лексема для else
#define LEX_CYCLE	'y'		// лексема для cycle
#define LEX_UNTIL	'u'		// лексема для until
#define LEX_OCTET	't'		// лексема для octet
#define LEX_TRUTH	't'		// лексема для truth
#define LEX_YES		'l'		// лексема для yes
#define LEX_NO		'l'		// лексема для no
#define LEX_FUSE	'|'		// лексема для fuse
#define LEX_MASK	'&'		// лексема для mask
#define LEX_FLIP	'!'		// лексема для flip

// Лексемы для сепараторов и операций
#define LEX_SEMICOLON			';'		// лексема для ;
#define LEX_COMMA				','		// лексема для ,
#define LEX_LEFTHESIS			'('		// лексема для (
#define LEX_RIGHTHESIS			')'		// лексема для )
#define LEX_COLON				':'		// лексема для :
#define LEX_EQUALS				'='		// лексема для =
#define LEX_PLUS				'+'		// лексема для +
#define LEX_MINUS				'-'		// лексема для -
#define LEX_STAR				'*'		// лексема для *
#define LEX_LEFT_ANGLE_BRACKET	'<'		// лексема для <
#define LEX_RIGHT_ANGLE_BRACKET	'>'		// лексема для >
#define LEX_RIGHT_ARROW			'a'		// лексема для ->
#define LEX_LEFT_ARROW			'g'		// лексема для <-
#define LEX_TILDE_ARROW			'p'		// лексема для ~>
#define LEX_EQUAL			    'E'		// лексема для ==
#define LEX_NOT_EQUAL			'N'		// лексема для !=

// Лексемы для идентификаторов и литералов
#define LEX_ID			'i'		// лексема для идентификатора
#define LEX_INT_LITERAL	'l'		// лексема для целого литерала
#define LEX_STR_LITERAL	's'		// лексема для строкового литерала

namespace LT	// таблица лексем (TЛ)
{
	constexpr int    LT_TI_NULLIDX = -1;  // нет элемента в ТЛ
	constexpr char   ATTR_NONE = 0;       // атрибут отсутствует (для всех лексем, кроме 't')
	constexpr char   ATTR_OCTET = 'o';    // атрибут типа octet для лексемы 't'
	constexpr char   ATTR_TRUTH = 'b';    // атрибут типа truth для лексемы 't'

	struct Entry	// строка ТЛ
	{
		char lexema = 0;			  // лексема
		char attr   = 0;			  // только для t: 'o' (octet) или 'b' (truth)
		int  sn		= 0;			  // номер строки в source
		int  pos	= 0;			  // позиция в строке (в протокол не выводится)
		int	 idxTI	= LT_TI_NULLIDX;  // индекс в ТИ или LT_TI_NULLIDX
	};

	struct LexTable		// экземпляр ТЛ
	{
		std::vector<Entry> table;   // массив строк ТЛ
	};

	LexTable Create(size_t reserve_size = 512);                 // создать ТЛ (с резервированием памяти)
	void Add(LexTable& lex_table, const Entry& entry);          // добавить строку в ТЛ
	const Entry& GetEntry(const LexTable& lex_table, int n);    // получить строку ТЛ		
}