#pragma once
#define ID_MAXSIZE 16 // макс длина идентификатора
#define TI_MAXSIZE 4096 // макс размер таблицы идентификаторов
#define TI_BYTE_DEFAULT 0 // default для integer
#define TI_TEXT_DEFAULT "" // default для string
#define TI_SYMBOL_DEFAULT '\0' // default для char
#define TI_BOOLEAN_DEFAULT false // default для boolean
#define TI_NULLIDX 0xffffffff // нет элемента
#define TI_TEXT_MAXSIZE 254 // макс длина строкового литерала
#define TI_SYMBOL_MAXSIZE 1 // макс длина символьного литерала

#include <vector>
#include "Error.h"

namespace IT {
	enum IDDATATYPE { // типы данных
		DT_UNKNOWN = 0,
		DT_INT = 1,   
		DT_CHAR = 2,  
		DT_STR = 3,   
		DT_BOOL = 4   
	};

	enum IDTYPE { // тип идентификатора
		VARIABLE = 1,
		FUNCTION = 2,
		PARAMETER = 3,
		LITERAL = 4,
		STATIC_FUNCTION = 5
	};

    struct Entry {
        int idxfirstLE; // индекс первой лексемы
        char id[ID_MAXSIZE]{}; // имя идентификатора
        IDDATATYPE iddatatype; // тип данных
        IDTYPE idtype; // вид идентификатора
        union {
            int vint; // значение int
            bool vboolean; // значение boolean
            char vchar; // значение char
            struct {
                int len;
                char str[TI_TEXT_MAXSIZE - 1];
            } vtext[TI_TEXT_MAXSIZE]; // значение string
        } value;

        Entry(
            int ifLE,
            char* name,
            IDDATATYPE iddatatype,
            IDTYPE idtype
        ) {
            this->idxfirstLE = ifLE;
            int nameSize = strlen(name);

            if (nameSize >= ID_MAXSIZE) {
                throw ERROR_THROW(125);
            }

            strncpy_s(id, name, ID_MAXSIZE);
            id[ID_MAXSIZE - 1] = '\0'; 

            this->iddatatype = iddatatype;
            this->idtype = idtype;
        }

        Entry() {};
    };

	struct IdTable {
		int size;
		std::vector<Entry>table;
	};
	IdTable Create();
	void Add(
		IdTable& idtable,
		Entry entry
	);
	Entry GetEntry(
		IdTable& idtable,
		int n
	);
	int IsId(
		IdTable& idtable,
		char id[ID_MAXSIZE]
	);
	void Delete(IdTable& idtable);

	void AddFunctionStaticLib(IdTable& idtable);
}