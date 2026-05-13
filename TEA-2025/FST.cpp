#include "FST.h"
#include "LT.h"
#include "IT.h"
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include "Error.h"

namespace FST {
	RELATION::RELATION(char c, unsigned short ns) {
		symbol = c;
		nnode = ns;
	}

	NODE::NODE() {
		n_relation = 0;
		RELATION* relations = NULL;
	};

	NODE::NODE(unsigned short n, RELATION rel, ...) {
		n_relation = n;
		RELATION* p = &rel;
		relations = new RELATION[n];
		for (int i = 0; i < n; i++) {
			relations[i] = p[i];
		}
	}

	FST::FST(std::vector<char> s, unsigned short ns, NODE n, ...) {
		string = s;
		nstates = ns;
		NODE* p = &n;
		for (int i = 0; i < ns; i++) {
			nodes.push_back(p[i]);
		};
		rstates = new unsigned short[nstates];
		memset(rstates, 0xff, sizeof(short) * nstates);
		rstates[0] = 0;
		position = -1;
	};

	bool step(FST& fst, unsigned short*& rstates) {
		bool rc = false;
		std::swap(rstates, fst.rstates);
		for (int i = 0; i < fst.nstates; i++) {
			if (rstates[i] == fst.position) {
				for (int j = 0; j < fst.nodes[i].n_relation; j++) {
					if (fst.nodes[i].relations[j].symbol == fst.string[fst.position]) {
						fst.rstates[fst.nodes[i].relations[j].nnode] = fst.position + 1;
						rc = true;
						return rc;
					}
				}
			}
		}
		return rc;
	}

	bool execute(FST& fst) {
		fst.FSTreturn();
		unsigned short* rstates = new unsigned short[fst.nstates];
		memset(rstates, 0xff, sizeof(unsigned short) * fst.nstates);
		unsigned short lstring = fst.string.size();
		bool rc = true;
		for (int i = 0; i < lstring && rc; i++) {
			fst.position++;
			rc = step(fst, rstates);
		};
		delete[] rstates;
		return (rc ? (fst.rstates[fst.nstates - 1] == lstring) : rc);
	}

	bool isSymbolIsStopSymbol(char ch, bool inLiteral = false) {
		char stopSymbols[] = {
			' ', ';', ':', ',', '(', '{', '}', ')', '=', '!', '<', '>',
			'+', '-', '*', '/', '%', '^', '\'', '\"', '\n', '~'
		};

		if (inLiteral) {
			char stopSymbolsInLiteral[] = {
				';', ',', '(', '{', '}', ')', '=', '!', '<', '>',
				'+', '-', '*', '/', '%', '^', '\'', '\"', '\n', '~'
			};
			int numberOfStopSymbols = sizeof(stopSymbolsInLiteral) / sizeof(char);
			for (int i = 0; i < numberOfStopSymbols; i++) {
				if (ch == stopSymbolsInLiteral[i]) return true;
			}
			return false;
		}
		else {
			int numberOfStopSymbols = sizeof(stopSymbols) / sizeof(char);
			for (int i = 0; i < numberOfStopSymbols; i++) {
				if (ch == stopSymbols[i]) return true;
			}
			return false;
		}
	}

	struct Lexema {
		std::vector<char> word;
		int line;
		int col;
		int idxTI;
		Lexema(std::vector<char>word, int line, int col, int idxTI) {
			this->word = word;
			this->line = line;
			this->col = col;
			this->idxTI = idxTI;
		}
	};

	std::vector<Lexema>lexems;

	int numberOfMainDefine = -1;

	void executeWordAndClear(std::vector<char>& word, std::vector<char>& str, std::vector<FSTAssigned>& FSTarray,
		int line, int& col, LT::LexTable& lextable, IT::IdTable& idtable) {

		word.erase(word.begin(), std::find_if(word.begin(), word.end(), [](char c) {
			return !std::isspace(c);
			}));

		word.erase(std::find_if(word.rbegin(), word.rend(), [](char c) {
			return !std::isspace(c);
			}).base(), word.end());
		if (word.empty()) {
			word.clear();
			return;
		}
		str.clear();
		str = word;

		int startCol = col - (int)word.size();
		if (startCol < 0) startCol = 0;

		Lexema lexema = Lexema(word, line, startCol, 0xffffffff);
		for (int i = 0; i < FSTarray.size(); i++) {
			FSTarray[i].fst->string = str;
			if (execute(*FSTarray[i].fst)) {
				lexema = Lexema(word, line, startCol, 0xffffffff);
				lexems.push_back(lexema);

				if (FSTarray[i].lex == LEX_MAIN) {
					numberOfMainDefine += 1;
				}
				if (numberOfMainDefine > 1) {
					throw ERROR_THROW(121);
				}

				word.clear();
				return;
			}
		}
		if (isdigit(word[0])) throw ERROR_THROW_IN(132, lexema.line, lexema.col);
		throw ERROR_THROW_IN(120, lexema.line, lexema.col);
		word.clear();
	}

	bool compareVectorToCString(const std::vector<char>& vec, const char* cstr) {
		if (vec.size() != std::strlen(cstr)) return false;
		for (size_t i = 0; i < vec.size(); ++i) {
			if (vec[i] != cstr[i]) return false;
		}
		return true;
	}

	int VectorToInt(std::vector<char>word) {
		std::string str(word.begin(), word.end());
		std::stringstream ss(str);
		unsigned long long result; // Используем большее целое для проверки
		ss >> result;

		// Проверяем, влезает ли значение в int
		if (result > 2147483647) { // MAX_INT для знакового 32-битного int
			throw ERROR_THROW(130);
		}

		return static_cast<int>(result);
	}

	void AnalyzerWords(std::vector<FSTAssigned> FSTarray, LT::LexTable& lextable, IT::IdTable& idtable) {

		bool isExecuted = false;
		bool isDeclared = false;
		bool isParam = false;
		bool isWasFunc = false;
		bool isWasMain = false;
		bool isFuncStarted = false;

		bool expectIdAfterType = false;

		std::string ParentName;
		std::vector<std::string> parentStack;
		parentStack.push_back("\0");

		IT::IDDATATYPE iddatatype = (IT::IDDATATYPE)0;
		IT::IDTYPE idtype = (IT::IDTYPE)0;
		int counterOfLiterals = 0;
		std::string LIT = "LIT";

		const char* keywords[] = { "announce", "integer", "string", "char", "boolean", "func", "return", "cout", "main", "cycle", "true", "false" };


		for (int i = 0; i < lexems.size(); i++) {
			for (int j = 0; j < FSTarray.size(); j++) {
				FSTarray[j].fst->string = lexems[i].word;
				if (execute(*FSTarray[j].fst)) {
					if ((compareVectorToCString(FSTarray[j].fst->string, "false") || compareVectorToCString(FSTarray[j].fst->string, "true")) && FSTarray[j].lex == 'i') {
						continue;
					}
					isExecuted = true;
					LT::Entry LTObj(FSTarray[j].lex, lexems[i].line, lexems[i].col, 0xffffffff);
					switch (FSTarray[j].lex)
					{
					case LEX_ANNOUNCE:
					{
						isDeclared = true;
						expectIdAfterType = false;
						idtype = IT::VARIABLE;
						break;
					}
					case LEX_INTEGER:
					{
						if (expectIdAfterType) {
							throw ERROR_THROW_IN(136, lexems[i].line, lexems[i].col);
						}

						if (!isDeclared && !isParam) {
							throw ERROR_THROW_IN(137, lexems[i].line, lexems[i].col);
						}

						if (FSTarray[j].iddatatype == IT::DT_INT) iddatatype = IT::DT_INT;
						else if (FSTarray[j].iddatatype == IT::DT_STR) iddatatype = IT::DT_STR;
						else if (FSTarray[j].iddatatype == IT::DT_CHAR) iddatatype = IT::DT_CHAR;
						else if (FSTarray[j].iddatatype == IT::DT_BOOL) iddatatype = IT::DT_BOOL;
						else iddatatype = (IT::IDDATATYPE)0;

						expectIdAfterType = true;
						break;
					}
					case LEX_FUNC:
					{
						while (!parentStack.empty()) {
							parentStack.pop_back();
						}
						idtype = IT::FUNCTION;
						break;
					}
					case LEX_COUT:
					case LEX_RETURN:
						break;
					case LEX_SEMICOLON:
					{
						expectIdAfterType = false;
						break;
					}
					case LEX_COMMA:
					{
						expectIdAfterType = false;
						break;
					}

					case LEX_ID:
					{
						std::string currentWord(lexems[i].word.begin(), lexems[i].word.end());
						for (const char* kw : keywords) {
							if (currentWord == kw) {
								throw ERROR_THROW_IN(136, lexems[i].line, lexems[i].col);
							}
						}

						expectIdAfterType = false;
						if (compareVectorToCString(lexems[i].word, "power_of")) {
							if (lextable.table[lextable.size - 1].lexema == LEX_FUNC) {
								throw ERROR_THROW_IN(127, lexems[i].line, lexems[i].col);
							}
							LTObj.idxTI = 0;
							break;
						}
						if (compareVectorToCString(lexems[i].word, "to_str")) {
							if (lextable.table[lextable.size - 1].lexema == LEX_FUNC) {
								throw ERROR_THROW_IN(127, lexems[i].line, lexems[i].col);
							}
							LTObj.idxTI = 1;
							break;
						}
						std::string scope;
						char* strr = new char[lexems[i].word.size() + 1];
						for (int g = 0; g < lexems[i].word.size(); g++) {
							strr[g] = lexems[i].word[g];
						}
						strr[lexems[i].word.size()] = '\0';
						if (IT::IsId(idtable, strr) != TI_NULLIDX && IT::GetEntry(idtable, IT::IsId(idtable, strr)).idtype == (IT::FUNCTION)) {
							scope += strr;
						}
						else {
							if (parentStack.size() > 0) {
								scope += parentStack.at(parentStack.size() - 1);
							}
							if (idtype != IT::FUNCTION) {
								scope += "$";
								scope += strr;
							}
							else {
								scope += strr;
							}
						}
						if (IT::IsId(idtable, (char*)scope.c_str()) == TI_NULLIDX) {
							if (idtype != IT::PARAMETER && idtype != IT::FUNCTION && !isDeclared) {
								throw ERROR_THROW_IN(124, lexems[i].line, lexems[i].col);
							}
							if (iddatatype != (IT::IDDATATYPE)0 && idtype != (IT::IDTYPE)0) {
								if (idtype == IT::FUNCTION) {
									ParentName = strr;
								}
								IT::Entry IDTObj;
								try {
									IDTObj = IT::Entry(lextable.size, (char*)scope.c_str(), iddatatype, idtype);
								}
								catch (Error::ERROR e) {
									if (e.id == 125) throw ERROR_THROW_IN(125, lexems[i].line, lexems[i].col);
									throw;
								}
								if (iddatatype == IT::DT_INT) IDTObj.value.vint = 0;
								else if (iddatatype == IT::DT_BOOL) IDTObj.value.vboolean = 0;
								else if (iddatatype == IT::DT_CHAR) IDTObj.value.vchar = ' ';
								else {
									for (int i = 0; i < TI_TEXT_MAXSIZE; i++) IDTObj.value.vtext->str[i] = '\0';
								}
								IT::Add(idtable, IDTObj);
								LTObj.idxTI = idtable.size - 1;
								isDeclared = false;
							}
							else {
								throw ERROR_THROW_IN(121, lexems[i].line, lexems[i].col);
							}
							if (!isParam) {
								idtype = (IT::IDTYPE)0;
								iddatatype = (IT::IDDATATYPE)0;
							}
						}
						else {
							if (lextable.table[lextable.size - 2].lexema == LEX_ANNOUNCE || lextable.table[lextable.size - 1].lexema == LEX_FUNC) {
								throw ERROR_THROW_IN(128, lexems[i].line, lexems[i].col);
							}
							LTObj.idxTI = IT::IsId(idtable, (char*)scope.c_str());
						}
						delete[] strr;
						break;
					}
					case LEX_LEFTTHESIS:
					{
						if (lextable.size != 0 && lextable.table[lextable.size - 1].idxTI != TI_NULLIDX && idtable.table[lextable.table[lextable.size - 1].idxTI].idtype == IT::FUNCTION) {
							isParam = true;
							idtype = IT::PARAMETER;
							parentStack.push_back(ParentName);
						}
						break;
					}
					case LEX_RIGHTTHESIS:
					{
						expectIdAfterType = false;
						if (isParam) {
							isParam = false;
							isWasFunc = true;
							isFuncStarted = false;
						}
						idtype = (IT::IDTYPE)0;
						iddatatype = (IT::IDDATATYPE)0;
						break;
					}
					case LEX_LEFTBRACE:
					{
						expectIdAfterType = false;
						if (!isFuncStarted) isFuncStarted = true;
						if (lextable.table[lextable.size - 1].lexema == LEX_RIGHTTHESIS && isWasFunc) {
							parentStack.push_back(ParentName);
						}
						break;
					}
					case LEX_BRACELET:
					{
						expectIdAfterType = false;
						if (isWasFunc) isWasFunc = false;
						break;
					}
					case LEX_MAIN:
					{
						if (!isWasMain) {
							isWasMain = true;
							while (!parentStack.empty()) parentStack.pop_back();
							parentStack.push_back("main");
						}
						else {
							throw ERROR_THROW_IN(122, lexems[i].line, lexems[i].col);
						}
						break;
					}
					case LEX_LITERAL:
					{
						counterOfLiterals += 1;
						LIT += std::to_string(counterOfLiterals);
						IT::Entry ITObj;
						if (lexems[i].word[0] == '\"') {
							if (lexems[i].word.size() <= 2) { 
								throw ERROR_THROW_IN(135, lexems[i].line, lexems[i].col); 
							}
							iddatatype = IT::DT_STR;

							int strLength = lexems[i].word.size() - 2; 
							if (strLength > TI_TEXT_MAXSIZE) {
								throw ERROR_THROW_IN(114, lexems[i].line, lexems[i].col);
							}
							ITObj.value.vtext->len = lexems[i].word.size();
							for (int g = 0; g < lexems[i].word.size(); g++) ITObj.value.vtext->str[g] = lexems[i].word[g];
							ITObj.value.vtext->str[lexems[i].word.size()] = '\0';
						}
						else if (lexems[i].word[0] == '\'') {
							if (lexems[i].word.size() < 3) { 
								throw ERROR_THROW_IN(135, lexems[i].line, lexems[i].col);
							}
							iddatatype = IT::DT_CHAR;
							ITObj.value.vchar = static_cast<int>(lexems[i].word[1]);
						}
						else if (compareVectorToCString(lexems[i].word, "false") || compareVectorToCString(lexems[i].word, "true")) {
							iddatatype = IT::DT_BOOL;
							if (compareVectorToCString(lexems[i].word, "false")) ITObj.value.vboolean = false;
							else ITObj.value.vboolean = true;
						}
						else {
							iddatatype = IT::DT_INT;
							try {
								ITObj.value.vint = VectorToInt(lexems[i].word);
							}
							catch (Error::ERROR e) {
								throw ERROR_THROW_IN(134, lexems[i].line, lexems[i].col);
							}
						}
						idtype = IT::IDTYPE::LITERAL;
						LTObj.idxTI = idtable.size;
						ITObj.iddatatype = iddatatype;
						ITObj.idtype = idtype;
						ITObj.idxfirstLE = lextable.size;
						strncpy_s(ITObj.id, LIT.c_str(), ID_MAXSIZE);
						IT::Add(idtable, ITObj);
						LIT = "LIT";
						break;
					}
					}
					if (LTObj.lexema == LEX_MORE || LTObj.lexema == LEX_LEFTBRACE || LTObj.lexema == LEX_BRACELET || LTObj.lexema == LEX_LEFTTHESIS || LTObj.lexema == LEX_RIGHTTHESIS) {
						LTObj.data = lexems[i].word;
					}
					LT::AddEntry(lextable, LTObj);
					j = FSTarray.size();
				}
			}
			if (!isExecuted) {
				throw ERROR_THROW_IN(120, lexems[i].line, lexems[i].col);
			}
		}
		if (!isWasMain) {
			throw ERROR_THROW(123);
		}
	}


	void Analyze(In::IN in, LT::LexTable& lextable, IT::IdTable& idtable) {
		std::vector<char>str;

		FST lex_integer(str, 8,
			NODE(1, RELATION('i', 1)),
			NODE(1, RELATION('n', 2)),
			NODE(1, RELATION('t', 3)),
			NODE(1, RELATION('e', 4)),
			NODE(1, RELATION('g', 5)),
			NODE(1, RELATION('e', 6)),
			NODE(1, RELATION('r', 7)),
			NODE());

		FST lex_string(str, 7,
			NODE(1, RELATION('s', 1)),
			NODE(1, RELATION('t', 2)),
			NODE(1, RELATION('r', 3)),
			NODE(1, RELATION('i', 4)),
			NODE(1, RELATION('n', 5)),
			NODE(1, RELATION('g', 6)),
			NODE());

		FST lex_boolean(str, 8, NODE(1, RELATION('b', 1)), NODE(1, RELATION('o', 2)), NODE(1, RELATION('o', 3)), NODE(1, RELATION('l', 4)), NODE(1, RELATION('e', 5)), NODE(1, RELATION('a', 6)), NODE(1, RELATION('n', 7)), NODE());
		FST lex_char(str, 5, NODE(1, RELATION('c', 1)), NODE(1, RELATION('h', 2)), NODE(1, RELATION('a', 3)), NODE(1, RELATION('r', 4)), NODE());
		FST lex_announce(str, 9, NODE(1, RELATION('a', 1)), NODE(1, RELATION('n', 2)), NODE(1, RELATION('n', 3)), NODE(1, RELATION('o', 4)), NODE(1, RELATION('u', 5)), NODE(1, RELATION('n', 6)), NODE(1, RELATION('c', 7)), NODE(1, RELATION('e', 8)), NODE());
		FST lex_return(str, 7, NODE(1, RELATION('r', 1)), NODE(1, RELATION('e', 2)), NODE(1, RELATION('t', 3)), NODE(1, RELATION('u', 4)), NODE(1, RELATION('r', 5)), NODE(1, RELATION('n', 6)), NODE());
		FST lex_cout(str, 5, NODE(1, RELATION('c', 1)), NODE(1, RELATION('o', 2)), NODE(1, RELATION('u', 3)), NODE(1, RELATION('t', 4)), NODE());
		FST lex_main(str, 5, NODE(1, RELATION('m', 1)), NODE(1, RELATION('a', 2)), NODE(1, RELATION('i', 3)), NODE(1, RELATION('n', 4)), NODE());
		FST lex_assigment(str, 2, NODE(1, RELATION('=', 1)), NODE());
		FST lex_semicolon(str, 2, NODE(1, RELATION(';', 1)), NODE());
		FST lex_comma(str, 2, NODE(1, RELATION(',', 1)), NODE());
		FST lex_leftbrace(str, 2, NODE(1, RELATION('{', 1)), NODE());
		FST lex_bracelet(str, 2, NODE(1, RELATION('}', 1)), NODE());
		FST lex_leftthesis(str, 2, NODE(1, RELATION('(', 1)), NODE());
		FST lex_rightthesis(str, 2, NODE(1, RELATION(')', 1)), NODE());

		FST lex_plus(str, 2, NODE(1, RELATION('+', 1)), NODE());
		FST lex_minus(str, 2, NODE(1, RELATION('-', 1)), NODE());
		FST lex_multiply(str, 2, NODE(1, RELATION('*', 1)), NODE());
		FST lex_divide(str, 2, NODE(1, RELATION('/', 1)), NODE());
		FST lex_mod(str, 2, NODE(1, RELATION('%', 1)), NODE());
		FST lex_not(str, 2, NODE(1, RELATION('!', 1)), NODE());

		FST lex_increment(str, 3, NODE(1, RELATION('+', 1)), NODE(1, RELATION('+', 2)), NODE());
		FST lex_decrement(str, 3, NODE(1, RELATION('-', 1)), NODE(1, RELATION('-', 2)), NODE());
		FST lex_inversion(str, 2, NODE(1, RELATION('~', 1)), NODE());

		FST lex_more(str, 2, NODE(1, RELATION('>', 1)), NODE());
		FST lex_less(str, 2, NODE(1, RELATION('<', 1)), NODE());
		FST lex_equal(str, 3, NODE(1, RELATION('=', 1)), NODE(1, RELATION('=', 2)), NODE());
		FST lex_exclamation(str, 3, NODE(1, RELATION('!', 1)), NODE(1, RELATION('=', 2)), NODE());
		FST lex_lessEqual(str, 3, NODE(1, RELATION('<', 1)), NODE(1, RELATION('=', 2)), NODE());
		FST lex_moreEqual(str, 3, NODE(1, RELATION('>', 1)), NODE(1, RELATION('=', 2)), NODE());

		FST lex_cycle(str, 6, NODE(1, RELATION('c', 1)), NODE(1, RELATION('y', 2)), NODE(1, RELATION('c', 3)), NODE(1, RELATION('l', 4)), NODE(1, RELATION('e', 5)), NODE());

		FST lex_id(str, 2, NODE(52, RELATION('a', 1), RELATION('b', 1), RELATION('c', 1), RELATION('d', 1), RELATION('e', 1), RELATION('f', 1), RELATION('g', 1), RELATION('h', 1), RELATION('i', 1), RELATION('j', 1), RELATION('k', 1), RELATION('l', 1), RELATION('m', 1), RELATION('n', 1), RELATION('o', 1), RELATION('p', 1), RELATION('q', 1), RELATION('r', 1), RELATION('s', 1), RELATION('t', 1), RELATION('u', 1), RELATION('v', 1), RELATION('w', 1), RELATION('x', 1), RELATION('y', 1), RELATION('z', 1), RELATION('A', 1), RELATION('B', 1), RELATION('C', 1), RELATION('D', 1), RELATION('E', 1), RELATION('F', 1), RELATION('G', 1), RELATION('H', 1), RELATION('I', 1), RELATION('J', 1), RELATION('K', 1), RELATION('L', 1), RELATION('M', 1), RELATION('N', 1), RELATION('O', 1), RELATION('P', 1), RELATION('Q', 1), RELATION('R', 1), RELATION('S', 1), RELATION('T', 1), RELATION('U', 1), RELATION('V', 1), RELATION('W', 1), RELATION('X', 1), RELATION('Y', 1), RELATION('Z', 1)), NODE(63, RELATION('a', 1), RELATION('b', 1), RELATION('c', 1), RELATION('d', 1), RELATION('e', 1), RELATION('f', 1), RELATION('g', 1), RELATION('h', 1), RELATION('i', 1), RELATION('j', 1), RELATION('k', 1), RELATION('l', 1), RELATION('m', 1), RELATION('n', 1), RELATION('o', 1), RELATION('p', 1), RELATION('q', 1), RELATION('r', 1), RELATION('s', 1), RELATION('t', 1), RELATION('u', 1), RELATION('v', 1), RELATION('w', 1), RELATION('x', 1), RELATION('y', 1), RELATION('z', 1), RELATION('A', 1), RELATION('B', 1), RELATION('C', 1), RELATION('D', 1), RELATION('E', 1), RELATION('F', 1), RELATION('G', 1), RELATION('H', 1), RELATION('I', 1), RELATION('J', 1), RELATION('K', 1), RELATION('L', 1), RELATION('M', 1), RELATION('N', 1), RELATION('O', 1), RELATION('P', 1), RELATION('Q', 1), RELATION('R', 1), RELATION('S', 1), RELATION('T', 1), RELATION('U', 1), RELATION('V', 1), RELATION('W', 1), RELATION('X', 1), RELATION('Y', 1), RELATION('Z', 1), RELATION('_', 1), RELATION('0', 1), RELATION('1', 1), RELATION('2', 1), RELATION('3', 1), RELATION('4', 1), RELATION('5', 1), RELATION('6', 1), RELATION('7', 1), RELATION('8', 1), RELATION('9', 1)));
		FST lex_func(str, 5, NODE(1, RELATION('f', 1)), NODE(1, RELATION('u', 2)), NODE(1, RELATION('n', 3)), NODE(1, RELATION('c', 4)), NODE());

		FST lex_textLiteral(str, 3, NODE(1, RELATION('\"', 1)), NODE(256, RELATION('\"', 2), RELATION((char)0, 1), RELATION((char)1, 1), RELATION((char)2, 1), RELATION((char)3, 1), RELATION((char)4, 1), RELATION((char)5, 1), RELATION((char)6, 1), RELATION((char)7, 1), RELATION((char)8, 1), RELATION((char)9, 1), RELATION((char)10, 1), RELATION((char)11, 1), RELATION((char)12, 1), RELATION((char)13, 1), RELATION((char)14, 1), RELATION((char)15, 1), RELATION((char)16, 1), RELATION((char)17, 1), RELATION((char)18, 1), RELATION((char)19, 1), RELATION((char)20, 1), RELATION((char)21, 1), RELATION((char)22, 1), RELATION((char)23, 1), RELATION((char)24, 1), RELATION((char)25, 1), RELATION((char)26, 1), RELATION((char)27, 1), RELATION((char)28, 1), RELATION((char)29, 1), RELATION((char)30, 1), RELATION((char)31, 1), RELATION((char)32, 1), RELATION((char)33, 1), RELATION((char)35, 1), RELATION((char)36, 1), RELATION((char)37, 1), RELATION((char)38, 1), RELATION((char)39, 1), RELATION((char)40, 1), RELATION((char)41, 1), RELATION((char)42, 1), RELATION((char)43, 1), RELATION((char)44, 1), RELATION((char)45, 1), RELATION((char)46, 1), RELATION((char)47, 1), RELATION((char)48, 1), RELATION((char)49, 1), RELATION((char)50, 1), RELATION((char)51, 1), RELATION((char)52, 1), RELATION((char)53, 1), RELATION((char)54, 1), RELATION((char)55, 1), RELATION((char)56, 1), RELATION((char)57, 1), RELATION((char)58, 1), RELATION((char)59, 1), RELATION((char)60, 1), RELATION((char)61, 1), RELATION((char)62, 1), RELATION((char)63, 1), RELATION((char)64, 1), RELATION((char)65, 1), RELATION((char)66, 1), RELATION((char)67, 1), RELATION((char)68, 1), RELATION((char)69, 1), RELATION((char)70, 1), RELATION((char)71, 1), RELATION((char)72, 1), RELATION((char)73, 1), RELATION((char)74, 1), RELATION((char)75, 1), RELATION((char)76, 1), RELATION((char)77, 1), RELATION((char)78, 1), RELATION((char)79, 1), RELATION((char)80, 1), RELATION((char)81, 1), RELATION((char)82, 1), RELATION((char)83, 1), RELATION((char)84, 1), RELATION((char)85, 1), RELATION((char)86, 1), RELATION((char)87, 1), RELATION((char)88, 1), RELATION((char)89, 1), RELATION((char)90, 1), RELATION((char)91, 1), RELATION((char)92, 1), RELATION((char)93, 1), RELATION((char)94, 1), RELATION((char)95, 1), RELATION((char)96, 1), RELATION((char)97, 1), RELATION((char)98, 1), RELATION((char)99, 1), RELATION((char)100, 1), RELATION((char)101, 1), RELATION((char)102, 1), RELATION((char)103, 1), RELATION((char)104, 1), RELATION((char)105, 1), RELATION((char)106, 1), RELATION((char)107, 1), RELATION((char)108, 1), RELATION((char)109, 1), RELATION((char)110, 1), RELATION((char)111, 1), RELATION((char)112, 1), RELATION((char)113, 1), RELATION((char)114, 1), RELATION((char)115, 1), RELATION((char)116, 1), RELATION((char)117, 1), RELATION((char)118, 1), RELATION((char)119, 1), RELATION((char)120, 1), RELATION((char)121, 1), RELATION((char)122, 1), RELATION((char)123, 1), RELATION((char)124, 1), RELATION((char)125, 1), RELATION((char)126, 1), RELATION((char)127, 1), RELATION((char)128, 1), RELATION((char)129, 1), RELATION((char)130, 1), RELATION((char)131, 1), RELATION((char)132, 1), RELATION((char)133, 1), RELATION((char)134, 1), RELATION((char)135, 1), RELATION((char)136, 1), RELATION((char)137, 1), RELATION((char)138, 1), RELATION((char)139, 1), RELATION((char)140, 1), RELATION((char)141, 1), RELATION((char)142, 1), RELATION((char)143, 1), RELATION((char)144, 1), RELATION((char)145, 1), RELATION((char)146, 1), RELATION((char)147, 1), RELATION((char)148, 1), RELATION((char)149, 1), RELATION((char)150, 1), RELATION((char)151, 1), RELATION((char)152, 1), RELATION((char)153, 1), RELATION((char)154, 1), RELATION((char)155, 1), RELATION((char)156, 1), RELATION((char)157, 1), RELATION((char)158, 1), RELATION((char)159, 1), RELATION((char)160, 1), RELATION((char)161, 1), RELATION((char)162, 1), RELATION((char)163, 1), RELATION((char)164, 1), RELATION((char)165, 1), RELATION((char)166, 1), RELATION((char)167, 1), RELATION((char)168, 1), RELATION((char)169, 1), RELATION((char)170, 1), RELATION((char)171, 1), RELATION((char)172, 1), RELATION((char)173, 1), RELATION((char)174, 1), RELATION((char)175, 1), RELATION((char)176, 1), RELATION((char)177, 1), RELATION((char)178, 1), RELATION((char)179, 1), RELATION((char)180, 1), RELATION((char)181, 1), RELATION((char)182, 1), RELATION((char)183, 1), RELATION((char)184, 1), RELATION((char)185, 1), RELATION((char)186, 1), RELATION((char)187, 1), RELATION((char)188, 1), RELATION((char)189, 1), RELATION((char)190, 1), RELATION((char)191, 1), RELATION((char)192, 1), RELATION((char)193, 1), RELATION((char)194, 1), RELATION((char)195, 1), RELATION((char)196, 1), RELATION((char)197, 1), RELATION((char)198, 1), RELATION((char)199, 1), RELATION((char)200, 1), RELATION((char)201, 1), RELATION((char)202, 1), RELATION((char)203, 1), RELATION((char)204, 1), RELATION((char)205, 1), RELATION((char)206, 1), RELATION((char)207, 1), RELATION((char)208, 1), RELATION((char)209, 1), RELATION((char)210, 1), RELATION((char)211, 1), RELATION((char)212, 1), RELATION((char)213, 1), RELATION((char)214, 1), RELATION((char)215, 1), RELATION((char)216, 1), RELATION((char)217, 1), RELATION((char)218, 1), RELATION((char)219, 1), RELATION((char)220, 1), RELATION((char)221, 1), RELATION((char)222, 1), RELATION((char)223, 1), RELATION((char)224, 1), RELATION((char)225, 1), RELATION((char)226, 1), RELATION((char)227, 1), RELATION((char)228, 1), RELATION((char)229, 1), RELATION((char)230, 1), RELATION((char)231, 1), RELATION((char)232, 1), RELATION((char)233, 1), RELATION((char)234, 1), RELATION((char)235, 1), RELATION((char)236, 1), RELATION((char)237, 1), RELATION((char)238, 1), RELATION((char)239, 1), RELATION((char)240, 1), RELATION((char)241, 1), RELATION((char)242, 1), RELATION((char)243, 1), RELATION((char)244, 1), RELATION((char)245, 1), RELATION((char)246, 1), RELATION((char)247, 1), RELATION((char)248, 1), RELATION((char)249, 1), RELATION((char)250, 1), RELATION((char)251, 1), RELATION((char)252, 1), RELATION((char)253, 1), RELATION((char)254, 1), RELATION((char)255, 1)), NODE());

		FST lex_symbolLiteral(
			str,
			3,
			NODE(1, RELATION('\'', 1)),
			NODE(256,
				RELATION('\'', 2),
				RELATION((char)0, 1), RELATION((char)1, 1), RELATION((char)2, 1), RELATION((char)3, 1), RELATION((char)4, 1), RELATION((char)5, 1), RELATION((char)6, 1), RELATION((char)7, 1), RELATION((char)8, 1), RELATION((char)9, 1), RELATION((char)11, 1), RELATION((char)12, 1), RELATION((char)13, 1), RELATION((char)14, 1), RELATION((char)15, 1), RELATION((char)16, 1), RELATION((char)17, 1), RELATION((char)18, 1), RELATION((char)19, 1), RELATION((char)20, 1), RELATION((char)21, 1), RELATION((char)22, 1), RELATION((char)23, 1), RELATION((char)24, 1), RELATION((char)25, 1), RELATION((char)26, 1), RELATION((char)27, 1), RELATION((char)28, 1), RELATION((char)29, 1), RELATION((char)30, 1), RELATION((char)31, 1), RELATION((char)32, 1), RELATION((char)33, 1), RELATION((char)35, 1), RELATION((char)36, 1), RELATION((char)37, 1), RELATION((char)38, 1), RELATION((char)39, 1), RELATION((char)40, 1), RELATION((char)41, 1), RELATION((char)42, 1), RELATION((char)43, 1), RELATION((char)44, 1), RELATION((char)45, 1), RELATION((char)46, 1), RELATION((char)47, 1), RELATION((char)48, 1), RELATION((char)49, 1), RELATION((char)50, 1), RELATION((char)51, 1), RELATION((char)52, 1), RELATION((char)53, 1), RELATION((char)54, 1), RELATION((char)55, 1), RELATION((char)56, 1), RELATION((char)57, 1), RELATION((char)58, 1), RELATION((char)59, 1), RELATION((char)60, 1), RELATION((char)61, 1), RELATION((char)62, 1), RELATION((char)63, 1), RELATION((char)64, 1), RELATION((char)65, 1), RELATION((char)66, 1), RELATION((char)67, 1), RELATION((char)68, 1), RELATION((char)69, 1), RELATION((char)70, 1), RELATION((char)71, 1), RELATION((char)72, 1), RELATION((char)73, 1), RELATION((char)74, 1), RELATION((char)75, 1), RELATION((char)76, 1), RELATION((char)77, 1), RELATION((char)78, 1), RELATION((char)79, 1), RELATION((char)80, 1), RELATION((char)81, 1), RELATION((char)82, 1), RELATION((char)83, 1), RELATION((char)84, 1), RELATION((char)85, 1), RELATION((char)86, 1), RELATION((char)87, 1), RELATION((char)88, 1), RELATION((char)89, 1), RELATION((char)90, 1), RELATION((char)91, 1), RELATION((char)92, 1), RELATION((char)93, 1), RELATION((char)94, 1), RELATION((char)95, 1), RELATION((char)96, 1), RELATION((char)97, 1), RELATION((char)98, 1), RELATION((char)99, 1), RELATION((char)100, 1), RELATION((char)101, 1), RELATION((char)102, 1), RELATION((char)103, 1), RELATION((char)104, 1), RELATION((char)105, 1), RELATION((char)106, 1), RELATION((char)107, 1), RELATION((char)108, 1), RELATION((char)109, 1), RELATION((char)110, 1), RELATION((char)111, 1), RELATION((char)112, 1), RELATION((char)113, 1), RELATION((char)114, 1), RELATION((char)115, 1), RELATION((char)116, 1), RELATION((char)117, 1), RELATION((char)118, 1), RELATION((char)119, 1), RELATION((char)120, 1), RELATION((char)121, 1), RELATION((char)122, 1), RELATION((char)123, 1), RELATION((char)124, 1), RELATION((char)125, 1), RELATION((char)126, 1), RELATION((char)127, 1), RELATION((char)128, 1), RELATION((char)129, 1), RELATION((char)130, 1), RELATION((char)131, 1), RELATION((char)132, 1), RELATION((char)133, 1), RELATION((char)134, 1), RELATION((char)135, 1), RELATION((char)136, 1), RELATION((char)137, 1), RELATION((char)138, 1), RELATION((char)139, 1), RELATION((char)140, 1), RELATION((char)141, 1), RELATION((char)142, 1), RELATION((char)143, 1), RELATION((char)144, 1), RELATION((char)145, 1), RELATION((char)146, 1), RELATION((char)147, 1), RELATION((char)148, 1), RELATION((char)149, 1), RELATION((char)150, 1), RELATION((char)151, 1), RELATION((char)152, 1), RELATION((char)153, 1), RELATION((char)154, 1), RELATION((char)155, 1), RELATION((char)156, 1), RELATION((char)157, 1), RELATION((char)158, 1), RELATION((char)159, 1), RELATION((char)160, 1), RELATION((char)161, 1), RELATION((char)162, 1), RELATION((char)163, 1), RELATION((char)164, 1), RELATION((char)165, 1), RELATION((char)166, 1), RELATION((char)167, 1), RELATION((char)168, 1), RELATION((char)169, 1), RELATION((char)170, 1), RELATION((char)171, 1), RELATION((char)172, 1), RELATION((char)173, 1), RELATION((char)174, 1), RELATION((char)175, 1), RELATION((char)176, 1), RELATION((char)177, 1), RELATION((char)178, 1), RELATION((char)179, 1), RELATION((char)180, 1), RELATION((char)181, 1), RELATION((char)182, 1), RELATION((char)183, 1), RELATION((char)184, 1), RELATION((char)185, 1), RELATION((char)186, 1), RELATION((char)187, 1), RELATION((char)188, 1), RELATION((char)189, 1), RELATION((char)190, 1), RELATION((char)191, 1), RELATION((char)192, 1), RELATION((char)193, 1), RELATION((char)194, 1), RELATION((char)195, 1), RELATION((char)196, 1), RELATION((char)197, 1), RELATION((char)198, 1), RELATION((char)199, 1), RELATION((char)200, 1), RELATION((char)201, 1), RELATION((char)202, 1), RELATION((char)203, 1), RELATION((char)204, 1), RELATION((char)205, 1), RELATION((char)206, 1), RELATION((char)207, 1), RELATION((char)208, 1), RELATION((char)209, 1), RELATION((char)210, 1), RELATION((char)211, 1), RELATION((char)212, 1), RELATION((char)213, 1), RELATION((char)214, 1), RELATION((char)215, 1), RELATION((char)216, 1), RELATION((char)217, 1), RELATION((char)218, 1), RELATION((char)219, 1), RELATION((char)220, 1), RELATION((char)221, 1), RELATION((char)222, 1), RELATION((char)223, 1), RELATION((char)224, 1), RELATION((char)225, 1), RELATION((char)226, 1), RELATION((char)227, 1), RELATION((char)228, 1), RELATION((char)229, 1), RELATION((char)230, 1), RELATION((char)231, 1), RELATION((char)232, 1), RELATION((char)233, 1), RELATION((char)234, 1), RELATION((char)235, 1), RELATION((char)236, 1), RELATION((char)237, 1), RELATION((char)238, 1), RELATION((char)239, 1), RELATION((char)240, 1), RELATION((char)241, 1), RELATION((char)242, 1), RELATION((char)243, 1), RELATION((char)244, 1), RELATION((char)245, 1), RELATION((char)246, 1), RELATION((char)247, 1), RELATION((char)248, 1), RELATION((char)249, 1), RELATION((char)250, 1), RELATION((char)251, 1), RELATION((char)252, 1), RELATION((char)253, 1), RELATION((char)254, 1), RELATION((char)255, 1)
			),
			NODE()
		);

		FST lex_byteLiteral(str, 3, NODE(11, RELATION('-', 1), RELATION('0', 2), RELATION('1', 2), RELATION('2', 2), RELATION('3', 2), RELATION('4', 2), RELATION('5', 2), RELATION('6', 2), RELATION('7', 2), RELATION('8', 2), RELATION('9', 2)), NODE(10, RELATION('0', 2), RELATION('1', 2), RELATION('2', 2), RELATION('3', 2), RELATION('4', 2), RELATION('5', 2), RELATION('6', 2), RELATION('7', 2), RELATION('8', 2), RELATION('9', 2)), NODE(10, RELATION('0', 2), RELATION('1', 2), RELATION('2', 2), RELATION('3', 2), RELATION('4', 2), RELATION('5', 2), RELATION('6', 2), RELATION('7', 2), RELATION('8', 2), RELATION('9', 2)));
		FST lex_booleanLiteral(str, 9, NODE(2, RELATION('t', 1), RELATION('f', 4)), NODE(1, RELATION('r', 2)), NODE(1, RELATION('u', 3)), NODE(1, RELATION('e', 8)), NODE(1, RELATION('a', 5)), NODE(1, RELATION('l', 6)), NODE(1, RELATION('s', 7)), NODE(1, RELATION('e', 8)), NODE());


		std::vector<FSTAssigned> FSTarray = {
			FSTAssigned(&lex_boolean, IT::DT_BOOL, LEX_BOOLEAN),
			FSTAssigned(&lex_integer, IT::DT_INT, LEX_INTEGER),
			FSTAssigned(&lex_string, IT::DT_STR, LEX_STRING),
			FSTAssigned(&lex_char, IT::DT_CHAR, LEX_CHAR),
			FSTAssigned(&lex_func, (IT::IDDATATYPE)0, LEX_FUNC),
			FSTAssigned(&lex_announce, (IT::IDDATATYPE)0, LEX_ANNOUNCE),
			FSTAssigned(&lex_return, (IT::IDDATATYPE)0, LEX_RETURN),
			FSTAssigned(&lex_cout, (IT::IDDATATYPE)0, LEX_COUT),
			FSTAssigned(&lex_main, (IT::IDDATATYPE)0, LEX_MAIN),
			FSTAssigned(&lex_cycle, (IT::IDDATATYPE)0, LEX_CYCLE),
			FSTAssigned(&lex_semicolon, (IT::IDDATATYPE)0, LEX_SEMICOLON),
			FSTAssigned(&lex_comma, (IT::IDDATATYPE)0, LEX_COMMA),
			FSTAssigned(&lex_leftbrace, (IT::IDDATATYPE)0, LEX_LEFTBRACE),
			FSTAssigned(&lex_bracelet, (IT::IDDATATYPE)0, LEX_BRACELET),
			FSTAssigned(&lex_leftthesis, (IT::IDDATATYPE)0, LEX_LEFTTHESIS),
			FSTAssigned(&lex_rightthesis, (IT::IDDATATYPE)0, LEX_RIGHTTHESIS),
			FSTAssigned(&lex_assigment, (IT::IDDATATYPE)0, LEX_ASSIGNMENT),

			FSTAssigned(&lex_plus, (IT::IDDATATYPE)0, '+'),
			FSTAssigned(&lex_minus, (IT::IDDATATYPE)0, '-'),
			FSTAssigned(&lex_multiply, (IT::IDDATATYPE)0, '*'),
			FSTAssigned(&lex_divide, (IT::IDDATATYPE)0, '/'),
			FSTAssigned(&lex_mod, (IT::IDDATATYPE)0, '%'),

			FSTAssigned(&lex_increment, (IT::IDDATATYPE)0, 'u'),
			FSTAssigned(&lex_decrement, (IT::IDDATATYPE)0, 'd'),
			FSTAssigned(&lex_inversion, (IT::IDDATATYPE)0, '~'),
			FSTAssigned(&lex_not, (IT::IDDATATYPE)0, '!'),

			FSTAssigned(&lex_more, (IT::IDDATATYPE)0, '>'),
			FSTAssigned(&lex_less, (IT::IDDATATYPE)0, '<'),
			FSTAssigned(&lex_equal, (IT::IDDATATYPE)0, 'q'), // ==
			FSTAssigned(&lex_exclamation, (IT::IDDATATYPE)0, 'w'), // !=
			FSTAssigned(&lex_moreEqual, (IT::IDDATATYPE)0, 'x'), // >=
			FSTAssigned(&lex_lessEqual, (IT::IDDATATYPE)0, 'y'), // <=

			FSTAssigned(&lex_id, (IT::IDDATATYPE)0, LEX_ID),
			FSTAssigned(&lex_textLiteral, IT::DT_STR, LEX_LITERAL),
			FSTAssigned(&lex_symbolLiteral, IT::DT_CHAR, LEX_LITERAL),
			FSTAssigned(&lex_byteLiteral, IT::DT_INT, LEX_LITERAL),
			FSTAssigned(&lex_booleanLiteral, IT::DT_BOOL, LEX_LITERAL)
		};

		int FSTarrayLen = (int)FSTarray.size();

		IT::IDDATATYPE iddatatype = (IT::IDDATATYPE)0;
		IT::IDTYPE idtype = (IT::IDTYPE)0;

		int numberOfMainDefine = 0;
		int numberOfLexems = 0;
		bool inLiteral = false;
		char expectedSymbol;
		bool isPrevWasStopSymbol = false;

		char nextSymbol = '\0';
		char ch;
		int line = 1;
		int col = 0;
		std::vector<char>word;

		std::stack<std::pair<char, std::pair<int, int>>> bracketStack;

		for (int i = 0; i < in.size; i++) {
			ch = in.text[i];

			// Проверка на незакрытый строковый литерал
			if (ch == '\n' && inLiteral) {
				throw ERROR_THROW_IN(116, line, col);
			}

			if (ch == '\n') {
				line += 1;
				col = 0;
			}

			// Проверка скобок
			if (!inLiteral) {
				if (ch == '(' || ch == '{' || ch == '[') {
					bracketStack.push({ ch, {line, col} });
				}
				else if (ch == ')' || ch == '}' || ch == ']') {
					if (bracketStack.empty()) {
						if (ch == ')') throw ERROR_THROW_IN(613, line, col); // Ожидалась (
						if (ch == '}') throw ERROR_THROW_IN(614, line, col); // Ожидалась {
					}

					char openBracket = bracketStack.top().first;
					// Проверяем соответствие пар
					bool isMatch = (openBracket == '(' && ch == ')') ||
						(openBracket == '{' && ch == '}') ||
						(openBracket == '[' && ch == ']');

					if (!isMatch) {
						if (openBracket == '(') throw ERROR_THROW_IN(612, line, col); // Ждали )
						if (openBracket == '{') throw ERROR_THROW_IN(615, line, col); // Ждали }
					}
					bracketStack.pop();
				}
			}

			if (isSymbolIsStopSymbol(ch, inLiteral)) {
				if (ch == '\'' || ch == '\"') {
					if (inLiteral) {
						if (ch == expectedSymbol) {
							inLiteral = false;
							word.push_back(ch);
							executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
						}
						else {
							word.push_back(ch);
							throw ERROR_THROW_IN(126, line, col);

						}
					}
					else {
						inLiteral = true;
						expectedSymbol = ch;
						executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
						word.push_back(ch);
					}
				}
				else {
					if (inLiteral) {
						word.push_back(ch);
					}
					else {
						executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
						if (ch != ' ' && ch != '\n' && ch != '\'' && ch != '\"') {
							if (ch == '<' || ch == '>' || ch == '!' || ch == '=' || ch == '+' || ch == '-' || ch == '~') {
								// Сначала обрабатываем накопленное слово
								executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);

								if (ch == '~') {
									word.push_back(ch);
									executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
								}
								else if (i == in.size - 1) {
									word.push_back(ch);
									executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
								}
								else {
									nextSymbol = in.text[i + 1];
									if (nextSymbol == '=' || (ch == '+' && nextSymbol == '+') || (ch == '-' && nextSymbol == '-')) {
										word.push_back(ch);
										word.push_back(nextSymbol);
										executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
										i++;
										col++;
									}
									else {
										word.push_back(ch);
										executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
									}
								}
							}
							else {
								word.push_back(ch);
								executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
							}
						}
					}
				}
			}
			else {
				word.push_back(ch);
			}
			col++;
		}

		if (!bracketStack.empty()) {

			char openBracket = bracketStack.top().first;
			int errLine = bracketStack.top().second.first;
			int errCol = bracketStack.top().second.second;

			if (openBracket == '(') throw ERROR_THROW_IN(612, errLine, errCol); // Забыли )
			if (openBracket == '{') throw ERROR_THROW_IN(615, errLine, errCol); // Забыли }
		}

		executeWordAndClear(word, str, FSTarray, line, col, lextable, idtable);
		AnalyzerWords(FSTarray, lextable, idtable);
	}
}