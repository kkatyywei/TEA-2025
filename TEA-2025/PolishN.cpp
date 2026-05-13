#include "PolishN.h"
#include "Error.h"
#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include "LT.h" 

using namespace std;

namespace PolishNotation {

    // Приоритеты 

    int get_priority(char a)
    {
        switch (a)
        {
        case LEX_LEFTTHESIS: case LEX_RIGHTTHESIS: return 0;
        case LEX_COMMA: return 1;
        case LEX_NEWPROC: return 2;
        case LEX_ASSIGNMENT: case LEX_RETURN: case LEX_COUT: return 3;

        case LEX_MORE: case LEX_LESS:
        case LEX_EQUAL: case LEX_NOTEQUAL:
        case LEX_MOREEQUAL: case LEX_LESSEQUAL:
            return 4;

        case LEX_PLUS: case LEX_MINUS: return 5;

        case LEX_MULTIPLY: case LEX_DIVIDE: case LEX_MOD: return 6;

        case LEX_NOT: case LEX_INVERSION: case 'n': // 'n' - унарный минус
        case LEX_INCREMENT: case LEX_DECREMENT: return 7;

        default: return 0;
        }
    }

    std::string toString(int n) {
        char buf[16]; sprintf_s(buf, "%d", n); return std::string(buf);
    }

    void FixLT(LT::LexTable& lextable, const std::string& str, int start_pos, int count, std::vector<int>& ids, Log::LOG log) {
        int id_idx = 0;
        for (int i = 0; i < str.length(); i++) {
            lextable.table[start_pos + i].lexema = str[i];
            if (lextable.table[start_pos + i].lexema == LEX_ID || lextable.table[start_pos + i].lexema == LEX_LITERAL) {
                if (id_idx < ids.size()) lextable.table[start_pos + i].idxTI = ids[id_idx++];
                else lextable.table[start_pos + i].idxTI = LT_TI_NULLIDX;
            }
            else lextable.table[start_pos + i].idxTI = LT_TI_NULLIDX;
        }
        for (int i = str.length(); i < count; i++) {
            lextable.table[start_pos + i].lexema = '#';
            lextable.table[start_pos + i].idxTI = LT_TI_NULLIDX;
        }
    }

    bool ConvertToPolish(int start_pos, int end_pos, LT::LexTable& lextable,
        IT::IdTable& idtable, Log::LOG log) {
        std::stack<char> stack;
        std::string PolishString = "";
        std::vector<int> ids;
        int params_counter = 0;
        bool inFunction = false;

        for (int i = start_pos; i < end_pos; i++) {
            char lexem = lextable.table[i].lexema;

            if (lexem == '#') continue;

            if (lexem == LEX_ID || lexem == LEX_LITERAL) {
                bool isFuncName = false;
                if (lexem == LEX_ID && lextable.table[i].idxTI != LT_TI_NULLIDX) {
                    IT::IDTYPE type = idtable.table[lextable.table[i].idxTI].idtype;
                    if (type == IT::FUNCTION || type == IT::STATIC_FUNCTION) {
                        isFuncName = true;
                    }
                }
                PolishString += lexem;
                if (lextable.table[i].idxTI != LT_TI_NULLIDX) {
                    ids.push_back(lextable.table[i].idxTI);
                }
                if (isFuncName) {
                    inFunction = true;
                    params_counter = 0;
                }
                else if (inFunction && params_counter == 0) {
                    params_counter = 1;
                }
                continue;
            }
            if (lexem == LEX_ID && i + 1 < end_pos && lextable.table[i + 1].lexema == LEX_LEFTTHESIS) {
                // Это вызов функции
                bool isFuncName = false;
                if (lextable.table[i].idxTI != LT_TI_NULLIDX) {
                    IT::IDTYPE type = idtable.table[lextable.table[i].idxTI].idtype;
                    if (type == IT::FUNCTION || type == IT::STATIC_FUNCTION) {
                        isFuncName = true;
                    }
                }

                if (isFuncName) {
                    // Добавляем идентификатор функции
                    PolishString += lexem;
                    ids.push_back(lextable.table[i].idxTI);

                    // Обрабатываем аргументы
                    inFunction = true;
                    params_counter = 0;

                    // Пропускаем '('
                    i++;
                    stack.push(LEX_LEFTTHESIS);

                    // Обрабатываем аргументы пока не встретим ')'
                    int balance = 1;
                    while (i < end_pos && balance > 0) {
                        char next = lextable.table[i].lexema;
                        if (next == LEX_LEFTTHESIS) balance++;
                        if (next == LEX_RIGHTTHESIS) {
                            balance--;
                            if (balance == 0) {
                                // Закрываем скобку
                                while (!stack.empty() && stack.top() != LEX_LEFTTHESIS) {
                                    PolishString += stack.top();
                                    stack.pop();
                                }
                                if (!stack.empty()) stack.pop(); // Убираем '('

                                // Добавляем вызов функции
                                PolishString += LEX_NEWPROC;
                                PolishString += toString(params_counter);
                                inFunction = false;
                                break;
                            }
                        }

                        if (next == LEX_COMMA) {
                            while (!stack.empty() && stack.top() != LEX_LEFTTHESIS) {
                                PolishString += stack.top();
                                stack.pop();
                            }
                            if (inFunction) params_counter++;
                            i++;
                            continue;
                        }

                        i--;
                        break;
                    }
                }
                continue;
            }
            if (lexem == LEX_LEFTTHESIS) {
                stack.push(lexem);
                continue;
            }

            if (lexem == LEX_RIGHTTHESIS) {
                while (!stack.empty() && stack.top() != LEX_LEFTTHESIS) {
                    PolishString += stack.top();
                    stack.pop();
                }
                if (!stack.empty()) stack.pop();
                if (inFunction) {
                    PolishString += LEX_NEWPROC;
                    PolishString += toString(params_counter);
                    inFunction = false;
                    params_counter = 0;
                }
                continue;
            }

            if (lexem == LEX_COMMA) {
                while (!stack.empty() && stack.top() != LEX_LEFTTHESIS) {
                    PolishString += stack.top();
                    stack.pop();
                }
                if (inFunction) params_counter++;
                continue;
            }

            // Обработка унарных операторов
            if (lexem == LEX_MINUS || lexem == LEX_INVERSION || lexem == LEX_NOT) {
                bool isUnary = false;
                if (lexem == LEX_MINUS) {
                    if (i == start_pos) isUnary = true;
                    else if (i > start_pos) {
                        char prev = lextable.table[i - 1].lexema;
                        if (prev == LEX_LEFTTHESIS || prev == LEX_ASSIGNMENT ||
                            prev == LEX_COMMA || prev == LEX_PLUS ||
                            prev == LEX_MINUS || prev == LEX_MULTIPLY ||
                            prev == LEX_DIVIDE || prev == LEX_MOD ||
                            prev == LEX_MORE || prev == LEX_LESS ||
                            prev == LEX_EQUAL || prev == LEX_NOTEQUAL ||
                            prev == LEX_MOREEQUAL || prev == LEX_LESSEQUAL ||
                            prev == LEX_INVERSION || prev == LEX_NOT) {
                            isUnary = true;
                        }
                    }
                }
                else {
                    isUnary = true;
                }

                if (isUnary) {
                    if (lexem == LEX_MINUS) lexem = 'n';
                    while (!stack.empty() && stack.top() != LEX_LEFTTHESIS &&
                        get_priority(stack.top()) >= get_priority(lexem)) {
                        PolishString += stack.top();
                        stack.pop();
                    }
                    stack.push(lexem);
                    continue;
                }
            }

            // Проверка всех бинарных операторов
            bool isOp = (lexem == LEX_PLUS || lexem == LEX_MINUS || lexem == LEX_MULTIPLY ||
                lexem == LEX_DIVIDE || lexem == LEX_MOD || lexem == LEX_ASSIGNMENT ||
                lexem == LEX_INCREMENT || lexem == LEX_DECREMENT ||
                lexem == LEX_MORE || lexem == LEX_LESS ||
                lexem == LEX_EQUAL || lexem == LEX_NOTEQUAL ||
                lexem == LEX_MOREEQUAL || lexem == LEX_LESSEQUAL ||
                lexem == LEX_NOT);

            if (isOp) {
                while (!stack.empty() && stack.top() != LEX_LEFTTHESIS &&
                    get_priority(stack.top()) >= get_priority(lexem)) {
                    PolishString += stack.top();
                    stack.pop();
                }
                stack.push(lexem);
                continue;
            }
        }

        while (!stack.empty()) {
            PolishString += stack.top();
            stack.pop();
        }

        FixLT(lextable, PolishString, start_pos, end_pos - start_pos, ids, log);
        return true;
    }

    void PrintPolishCode(LT::LexTable& lextable, IT::IdTable& idtable, Log::LOG log) {
        string fullString = "";
        int currentLine = 1;
        fullString += to_string(currentLine) + " ";

        for (int i = 0; i < lextable.size; i++) {
            if (lextable.table[i].sn != currentLine) {
                fullString += "\n";
                currentLine = lextable.table[i].sn;
                if (currentLine != -1) fullString += to_string(currentLine) + " ";
            }
            char lexem = lextable.table[i].lexema;
            if (lexem == '#') continue;
            fullString += lexem;
        }

        if (log.stream && log.stream->is_open()) {
            *log.stream << endl << endl << fullString << endl;
        }
        else {
            cout << "Error: Log stream is not open!" << endl;
        }
    }

    void CreatePolishTable(MFST::LEX& lex, Log::LOG log) {
        for (int i = 0; i < lex.lextable.size; i++) {
            char lexem = lex.lextable.table[i].lexema;

            // 1. Присваивание
            if (lexem == LEX_ID && i + 1 < lex.lextable.size && lex.lextable.table[i + 1].lexema == LEX_ASSIGNMENT) {
                int start = i;
                int j = i + 1;
                while (j < lex.lextable.size && lex.lextable.table[j].lexema != LEX_SEMICOLON) j++;
                ConvertToPolish(start, j, lex.lextable, lex.idtable, log);
                i = j;
            }
            // 2. Объявление с инициализацией
            else if (lexem == LEX_ANNOUNCE) {
                int k = i;
                while (k < lex.lextable.size && lex.lextable.table[k].lexema != LEX_SEMICOLON) {
                    if (lex.lextable.table[k].lexema == LEX_ASSIGNMENT) {
                        int exprStart = k + 1;
                        int exprEnd = k + 1;
                        while (exprEnd < lex.lextable.size && lex.lextable.table[exprEnd].lexema != LEX_SEMICOLON) exprEnd++;
                        ConvertToPolish(exprStart, exprEnd, lex.lextable, lex.idtable, log);
                        i = exprEnd;
                        break;
                    }
                    k++;
                }
                if (i < k) i = k;
            }
            else if (lexem == LEX_RETURN) {
                int j = i + 1; while (j < lex.lextable.size && lex.lextable.table[j].lexema != LEX_SEMICOLON) j++;
                ConvertToPolish(i + 1, j, lex.lextable, lex.idtable, log); i = j;
            }
            else if (lexem == LEX_COUT) {
                int j = i + 1;
                if (lex.lextable.table[j].lexema == LEX_LEFTTHESIS) {
                    int balance = 1; int k = j + 1;
                    while (k < lex.lextable.size && balance > 0) {
                        if (lex.lextable.table[k].lexema == LEX_LEFTTHESIS) balance++;
                        if (lex.lextable.table[k].lexema == LEX_RIGHTTHESIS) balance--; k++;
                    }
                    ConvertToPolish(j + 1, k - 1, lex.lextable, lex.idtable, log); i = k;
                }
            }
            else if (lexem == LEX_ID && i + 1 < lex.lextable.size) {
                // Проверяем, является ли это вызовом функции
                char next = lex.lextable.table[i + 1].lexema;
                if (next == LEX_LEFTTHESIS) {
                    // Это вызов функции
                    int start = i;
                    int j = i + 1;
                    int balance = 1;
                    while (j < lex.lextable.size && balance > 0) {
                        if (lex.lextable.table[j].lexema == LEX_LEFTTHESIS) balance++;
                        if (lex.lextable.table[j].lexema == LEX_RIGHTTHESIS) balance--;
                        j++;
                    }
                    // Проверяем, является ли это частью выражения
                    if (j < lex.lextable.size) {
                        char after = lex.lextable.table[j].lexema;
                        if (after == LEX_SEMICOLON) {
                            // Просто вызов функции как оператор
                            ConvertToPolish(start, j, lex.lextable, lex.idtable, log);
                            i = j;
                        }
                        else if (after == LEX_PLUS || after == LEX_MINUS ||
                            after == LEX_MULTIPLY || after == LEX_DIVIDE ||
                            after == LEX_MOD || after == '>' || after == '<' ||
                            after == LEX_EQUAL || after == LEX_NOTEQUAL ||
                            after == LEX_MOREEQUAL || after == LEX_LESSEQUAL) {
                            // Найдем конец всего выражения
                            int exprEnd = j;
                            while (exprEnd < lex.lextable.size &&
                                lex.lextable.table[exprEnd].lexema != LEX_SEMICOLON) {
                                exprEnd++;
                            }
                            ConvertToPolish(start, exprEnd, lex.lextable, lex.idtable, log);
                            i = exprEnd;
                        }
                    }
                }
            }
            else if (lexem == LEX_CYCLE) {
                int j = i + 1;
                if (lex.lextable.table[j].lexema == LEX_LEFTTHESIS) {
                    int balance = 1; int k = j + 1;
                    while (k < lex.lextable.size && balance > 0) {
                        if (lex.lextable.table[k].lexema == LEX_LEFTTHESIS) balance++;
                        if (lex.lextable.table[k].lexema == LEX_RIGHTTHESIS) balance--; k++;
                    }
                    ConvertToPolish(j + 1, k - 1, lex.lextable, lex.idtable, log); i = k - 1;
                }
            }
        }
        PrintPolishCode(lex.lextable, lex.idtable, log);
    }

    bool PolishNotation(int& pos, LT::LexTable& lextable, IT::IdTable& idtable, Log::LOG log) { return true; }
}