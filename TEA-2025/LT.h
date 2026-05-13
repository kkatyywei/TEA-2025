#pragma once
#define LEXEMA_FIXSIZE 1 // размер лексемы
#define LT_MAXSIZE 4096 // максимальное количество строк в таблице лексем
#define LT_TI_NULLIDX 0xffffffff // нет элемента в таблице идентификаторов

#define LEX_ANNOUNCE    'a'    // announce
#define LEX_INTEGER     't'    // integer
#define LEX_CHAR        't'    // char
#define LEX_STRING      't'    // string
#define LEX_BOOLEAN     't'    // boolean
#define LEX_FUNC        'f'    // func
#define LEX_CYCLE       'c'    // cycle
#define LEX_COUT        'o'    // cout
#define LEX_RETURN      'r'    // return
#define LEX_MAIN        'm'    // main
#define LEX_ID			'i'	   // идентификатор
#define LEX_LITERAL		'l'    // литерал

#define LEX_PLUS        '+'
#define LEX_MINUS       '-'
#define LEX_MULTIPLY    '*'
#define LEX_DIVIDE      '/'
#define LEX_MOD         '%'
#define LEX_INCREMENT   'u'    // ++
#define LEX_DECREMENT   'd'    // --
#define LEX_INVERSION   '~'    // битовая инверсия



#define LEX_ASSIGNMENT	'='		// присваивание
#define LEX_SEMICOLON	';'  
#define LEX_COMMA		','
#define LEX_LEFTBRACE	'{'
#define	LEX_BRACELET	'}'
#define LEX_LEFTTHESIS	'('
#define LEX_RIGHTTHESIS ')'

#define LEX_MORE		'>'		// больше
#define LEX_LESS		'<'     // меньше
#define LEX_EQUAL		'q'     // равенство
#define LEX_NOTEQUAL	'w'     // не равно
#define LEX_MOREEQUAL	'x'     // больше равно
#define LEX_LESSEQUAL	'y'     // меньше равно
#define LEX_NOT			'!'     // логическое отрицание

#define LEX_NEWPROC     '@'

#include "In.h"
#include <vector>

namespace LT {
	struct Entry {
		char lexema; // лексема
		int sn; // номер строки в исходном коде
		int cn;
		int idxTI; // индекс в таблице идентификаторов
		std::vector<char>data;

		Entry( // конструктор для записи элемента таблицы
			char lexema,
			int sn,
			int col,
			int idxTI
		) {
			this->lexema = lexema;
			this->sn = sn;
			this->cn = col;
			this->idxTI = idxTI;
		};

		Entry() = default;
	};

	struct LexTable {
		int size;			     // текущий размер таблицы
		std::vector<Entry>table; // массив записей таблицы
	};
	LexTable Create();
	void AddEntry(
		LexTable& lextable,
		Entry entry
	);
	Entry GetEntry(
		LexTable& lextable,
		int n                    // номер получаемой строки
	);
	void Delete(LexTable& lextable);
}