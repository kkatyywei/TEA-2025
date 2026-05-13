#include "Semantic.h"
#include "Error.h"
#pragma warning(disable : 1041)

namespace SA {

    void operands(MFST::LEX lex) {
        for (int i = 0; i < lex.lextable.size; i++) {
            char lexem = lex.lextable.table[i].lexema;

            // 1. Унарные операторы
            if (lexem == LEX_MINUS || lexem == LEX_INVERSION || lexem == LEX_NOT) {
                bool isUnaryMinus = (lexem == LEX_MINUS);
                bool isUnary = false;
                if (isUnaryMinus) {
                    if (i == 0) isUnary = true;
                    else if (i > 0) {
                        char prevLex = lex.lextable.table[i - 1].lexema;
                        if (strchr("=,(+-*/%><!~", prevLex) || prevLex == LEX_ANNOUNCE || prevLex == LEX_RETURN) isUnary = true;
                        if (i > 2 && lex.lextable.table[i - 1].lexema == LEX_ASSIGNMENT &&
                            lex.lextable.table[i - 4].lexema == LEX_ANNOUNCE) isUnary = true;
                    }
                    if (!isUnary) goto CHECK_BINARY; // Переход к проверке бинарных
                }
                else isUnary = true;

                if (isUnary) { continue; }
            }

        CHECK_BINARY:
            // 2. Бинарные операции
            if (lexem == LEX_PLUS || lexem == LEX_MINUS || lexem == LEX_MULTIPLY ||
                lexem == LEX_DIVIDE || lexem == LEX_MOD ||
                lexem == LEX_MORE || lexem == LEX_LESS || lexem == LEX_EQUAL ||
                lexem == LEX_NOTEQUAL || lexem == LEX_MOREEQUAL || lexem == LEX_LESSEQUAL) {

                if (i > 0 && i < lex.lextable.size - 1) {
                    int leftIdx = lex.lextable.table[i - 1].idxTI;
                    int rightIdx = lex.lextable.table[i + 1].idxTI;

                    IT::IDDATATYPE leftType = (leftIdx != LT_TI_NULLIDX) ? lex.idtable.table[leftIdx].iddatatype : IT::DT_UNKNOWN;
                    IT::IDDATATYPE rightType = (rightIdx != LT_TI_NULLIDX) ? lex.idtable.table[rightIdx].iddatatype : IT::DT_UNKNOWN;

                    // Арифметика
                    if (lexem == LEX_PLUS || lexem == LEX_MINUS ||
                        lexem == LEX_MULTIPLY || lexem == LEX_DIVIDE || lexem == LEX_MOD) {

                        if (leftType != IT::DT_INT || rightType != IT::DT_INT) {
                            throw ERROR_THROW_IN(704, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                        }

                        if (lexem == LEX_DIVIDE || lexem == LEX_MOD) {
                            if (lex.lextable.table[i + 1].lexema == LEX_LITERAL && rightIdx != LT_TI_NULLIDX) {
                                if (lex.idtable.table[rightIdx].iddatatype == IT::DT_INT &&
                                    lex.idtable.table[rightIdx].value.vint == 0) {

                                    throw ERROR_THROW_IN(715, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                                }
                            }
                        }
                    }
                    // Сравнение
                    else {
                        bool compatible = (leftType == IT::DT_INT && rightType == IT::DT_INT) ||
                            (leftType == IT::DT_BOOL && rightType == IT::DT_BOOL) ||
                            (leftType == IT::DT_CHAR && rightType == IT::DT_CHAR);
                        if (!compatible) {
                            throw ERROR_THROW_IN(703, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                        }
                    }
                }
            }

            // 3. Присваивание
            if (lexem == LEX_ASSIGNMENT) {
                if (i > 0) {
                    int leftIdx = lex.lextable.table[i - 1].idxTI;
                    IT::IDDATATYPE targetType = (leftIdx != LT_TI_NULLIDX) ? lex.idtable.table[leftIdx].iddatatype : IT::DT_UNKNOWN;

                    bool hasComparison = false;
                    int checkPos = i + 1;
                    while (checkPos < lex.lextable.size && lex.lextable.table[checkPos].lexema != LEX_SEMICOLON) {
                        char lx = lex.lextable.table[checkPos].lexema;
                        if (lx == LEX_MORE || lx == LEX_LESS || lx == LEX_EQUAL ||
                            lx == LEX_NOTEQUAL || lx == LEX_MOREEQUAL || lx == LEX_LESSEQUAL) {
                            hasComparison = true;
                        }
                        checkPos++;
                    }

                    if (hasComparison) {
                        if (targetType != IT::DT_BOOL) throw ERROR_THROW_IN(703, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                    }
                    else {
                        int cur = 1;
                        IT::IDDATATYPE exprType = IT::DT_UNKNOWN;
                        bool firstOperand = true;

                        while (i + cur < lex.lextable.size && lex.lextable.table[i + cur].lexema != LEX_SEMICOLON) {
                            char curLex = lex.lextable.table[i + cur].lexema;
                            int idx = lex.lextable.table[i + cur].idxTI;
                            bool handled = false;

                            if ((curLex == LEX_ID || curLex == LEX_LITERAL) && idx != LT_TI_NULLIDX) {
                                IT::Entry& entry = lex.idtable.table[idx];
                                IT::IDDATATYPE currentOpType = entry.iddatatype;

                                if (curLex == LEX_ID && entry.idtype == IT::FUNCTION) {
                                    currentOpType = entry.iddatatype;
                                    if (i + cur + 1 < lex.lextable.size && lex.lextable.table[i + cur + 1].lexema == LEX_LEFTTHESIS) {
                                        int balance = 1;
                                        cur += 2;
                                        while (i + cur < lex.lextable.size && balance > 0) {
                                            if (lex.lextable.table[i + cur].lexema == LEX_LEFTTHESIS) balance++;
                                            if (lex.lextable.table[i + cur].lexema == LEX_RIGHTTHESIS) balance--;
                                            if (balance > 0) cur++;
                                        }
                                    }
                                }

                                if (firstOperand) { exprType = currentOpType; firstOperand = false; }
                                handled = true;
                            }
                            cur++;
                        }

                        if (!firstOperand && targetType != IT::DT_UNKNOWN && exprType != IT::DT_UNKNOWN) {
                            if (targetType != exprType) throw ERROR_THROW_IN(703, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                        }
                    }
                }
            }

            // 4. Проверка булевых литералов
            if (lexem == LEX_LITERAL) {
                int idxTI = lex.lextable.table[i].idxTI;
                if (idxTI != LT_TI_NULLIDX && lex.idtable.table[idxTI].iddatatype == IT::DT_BOOL) {
                    // Проверка контекста (нельзя в арифметике)
                    if (i > 0) {
                        char prev = lex.lextable.table[i - 1].lexema;
                        if (strchr("+-*/%", prev)) throw ERROR_THROW_IN(704, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                    }
                    if (i < lex.lextable.size - 1) {
                        char next = lex.lextable.table[i + 1].lexema;
                        if (strchr("+-*/%", next)) throw ERROR_THROW_IN(704, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                    }
                }
            }
        }

        // Проверка UNKNOWN типов
        for (int i = 0; i < lex.idtable.size; i++) {
            if (lex.idtable.table[i].iddatatype == IT::DT_UNKNOWN && lex.idtable.table[i].idtype != IT::FUNCTION) {
                throw ERROR_THROW_IN(708, lex.idtable.table[i].idxfirstLE, lex.idtable.table[i].idxfirstLE);
            }
        }
    }

    void functions(MFST::LEX lex) {
        for (int i = 0; i < lex.lextable.size; i++) {
            if (lex.lextable.table[i].lexema == LEX_COUT) {
                if (lex.lextable.table[i + 2].lexema == ')') {
                    throw ERROR_THROW_IN(714, lex.lextable.table[i + 2].sn, lex.lextable.table[i + 2].cn);
                }
                if (lex.idtable.table[lex.lextable.table[i + 2].idxTI].idtype == IT::FUNCTION &&
                    lex.idtable.table[lex.lextable.table[i + 2].idxTI].iddatatype == IT::DT_UNKNOWN) {
                    throw ERROR_THROW_IN(711, lex.lextable.table[i + 2].sn, lex.lextable.table[i + 2].cn);
                }
            }

            if (lex.lextable.table[i].lexema == LEX_ID &&
                lex.lextable.table[i - 1].lexema == LEX_FUNC &&
                lex.idtable.table[lex.lextable.table[i].idxTI].idtype == IT::FUNCTION)
            {
                int cur = 1;
                IT::IDDATATYPE returnType = lex.idtable.table[lex.lextable.table[i].idxTI].iddatatype;

                if (returnType == IT::DT_UNKNOWN) {
                    while (i + cur < lex.lextable.size && lex.lextable.table[i + cur].lexema != LEX_RETURN) cur++;
                    if (i + cur < lex.lextable.size && lex.lextable.table[i + cur].lexema == LEX_RETURN) {
                        throw ERROR_THROW_IN(709, lex.lextable.table[i + cur].sn, lex.lextable.table[i + cur].cn);
                    }
                }
                else {
                    while (i + cur < lex.lextable.size && lex.lextable.table[i + cur].lexema != LEX_RETURN) cur++;
                    if (i + cur == lex.lextable.size) {
                        throw ERROR_THROW_IN(700, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                    }
                    if (i + cur < lex.lextable.size &&
                        (lex.lextable.table[i + cur + 1].lexema == LEX_ID || lex.lextable.table[i + cur + 1].lexema == LEX_LITERAL)) {
                        int retIdx = lex.lextable.table[i + cur + 1].idxTI;
                        if (lex.idtable.table[retIdx].idtype != IT::FUNCTION &&
                            lex.idtable.table[retIdx].iddatatype != returnType) {
                            throw ERROR_THROW_IN(700, lex.lextable.table[i + cur].sn, lex.lextable.table[i + cur].cn);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < lex.lextable.size; i++) {
            if (lex.lextable.table[i].lexema == LEX_ID &&
                lex.idtable.table[lex.lextable.table[i].idxTI].idtype == IT::FUNCTION &&
                lex.lextable.table[i - 1].lexema == LEX_FUNC)
            {
                IT::IDDATATYPE ids[16];
                int idsSize = 0;
                int funcPos = lex.idtable.table[lex.lextable.table[i].idxTI].idxfirstLE;

                while (lex.lextable.table[funcPos + 1].lexema != LEX_RIGHTTHESIS) {
                    if (lex.lextable.table[funcPos + 1].lexema == LEX_ID || lex.lextable.table[funcPos + 1].lexema == LEX_LITERAL) {
                        ids[idsSize++] = lex.idtable.table[lex.lextable.table[funcPos + 1].idxTI].iddatatype;
                    }
                    funcPos++;
                    if (idsSize >= 16) throw ERROR_THROW_IN(705, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                }
            }

            if (lex.lextable.table[i].lexema == LEX_ID &&
                lex.idtable.table[lex.lextable.table[i].idxTI].idtype == IT::STATIC_FUNCTION)
            {
                int cur = 2;
                IT::IDDATATYPE dt = IT::DT_INT;
                if (strcmp(lex.idtable.table[lex.lextable.table[i].idxTI].id, "power_of") == 0) dt = IT::DT_INT;
                else if (strcmp(lex.idtable.table[lex.lextable.table[i].idxTI].id, "to_str") == 0) dt = IT::DT_INT;

                int numberOfParams = 0;
                while (lex.lextable.table[i + cur].lexema != LEX_RIGHTTHESIS) {
                    if (lex.lextable.table[i + cur].lexema == LEX_ID || lex.lextable.table[i + cur].lexema == LEX_LITERAL) {
                        if (lex.idtable.table[lex.lextable.table[i + cur].idxTI].iddatatype == dt) numberOfParams++;
                        else throw ERROR_THROW_IN(713, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                    }
                    cur++;
                }
                if (strcmp(lex.idtable.table[lex.lextable.table[i].idxTI].id, "power_of") == 0 && numberOfParams != 2) {
                    throw ERROR_THROW_IN(713, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                }
                else if (strcmp(lex.idtable.table[lex.lextable.table[i].idxTI].id, "to_str") == 0 && numberOfParams != 1)
                    throw ERROR_THROW_IN(713, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
            }

            if (lex.lextable.table[i].lexema == LEX_ID &&
                lex.idtable.table[lex.lextable.table[i].idxTI].idtype == IT::FUNCTION &&
                lex.lextable.table[i - 1].lexema != LEX_FUNC)
            {
                IT::IDDATATYPE ids[16];
                int idsSize = 0;
                int funcPos = lex.idtable.table[lex.lextable.table[i].idxTI].idxfirstLE;

                if (lex.lextable.table[i + 1].lexema != LEX_LEFTTHESIS) {
                    throw ERROR_THROW_IN(706, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                }

                while (lex.lextable.table[funcPos + 1].lexema != LEX_RIGHTTHESIS) {
                    if (lex.lextable.table[funcPos + 1].lexema == LEX_ID || lex.lextable.table[funcPos + 1].lexema == LEX_LITERAL) {
                        ids[idsSize++] = lex.idtable.table[lex.lextable.table[funcPos + 1].idxTI].iddatatype;
                    }
                    funcPos++;
                    if (idsSize >= 16) throw ERROR_THROW_IN(705, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                }

                int cur = 1;
                int paramCount = 0;
                while (lex.lextable.table[i + cur].lexema != LEX_RIGHTTHESIS) {
                    if (lex.lextable.table[i + cur].lexema == LEX_ID || lex.lextable.table[i + cur].lexema == LEX_LITERAL) {
                        if (lex.idtable.table[lex.lextable.table[i + cur].idxTI].iddatatype != ids[paramCount]) {
                            throw ERROR_THROW_IN(702, lex.lextable.table[i + cur].sn, lex.lextable.table[i + cur].cn);
                        }
                        paramCount++;
                    }
                    cur++;
                }

                if (paramCount != idsSize) {
                    throw ERROR_THROW_IN(701, lex.lextable.table[i + cur].sn, lex.lextable.table[i + cur].cn);
                }
                i += cur;
            }
        }

        // Проверка присваивания функции в переменную
        for (int i = 0; i < lex.lextable.size; i++) {
            if (lex.lextable.table[i].lexema == LEX_ASSIGNMENT &&
                lex.lextable.table[i - 1].lexema == LEX_ID &&
                lex.lextable.table[i + 1].lexema == LEX_ID &&
                lex.idtable.table[lex.lextable.table[i + 1].idxTI].idtype == IT::FUNCTION) {

                int funcIdx = lex.lextable.table[i + 1].idxTI;
                IT::IDDATATYPE returnType = lex.idtable.table[funcIdx].iddatatype;

                int destIdx = lex.lextable.table[i - 1].idxTI;
                IT::IDDATATYPE destType = lex.idtable.table[destIdx].iddatatype;

                if (returnType != destType) {
                    throw ERROR_THROW_IN(707, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                }
            }
        }
    }

    void literals(MFST::LEX lex) {
        for (int i = 0; i < lex.idtable.size; i++) {
            if (lex.idtable.table[i].idtype == IT::LITERAL && lex.idtable.table[i].iddatatype == IT::DT_INT) {
                unsigned long value = lex.idtable.table[i].value.vint;
                if (value < 0 || value >= 4294967295) {
                    throw ERROR_THROW_IN(712, lex.lextable.table[lex.idtable.table[i].idxfirstLE].sn, lex.lextable.table[lex.idtable.table[i].idxfirstLE].cn);
                }
            }
        }
    }

    void cycles(MFST::LEX lex) {
        for (int i = 0; i < lex.lextable.size; i++) {
            if (lex.lextable.table[i].lexema == LEX_CYCLE) {
                int cur = 2;
                bool isFunc = false;
                while (lex.lextable.table[i + cur].lexema != LEX_RIGHTTHESIS || isFunc) {
                    if (lex.lextable.table[i + cur].lexema == LEX_LEFTTHESIS) isFunc = true;
                    if (lex.lextable.table[i + cur].lexema == LEX_RIGHTTHESIS) isFunc = false;

                    if (!isFunc && (lex.lextable.table[i + cur].lexema == LEX_ID || lex.lextable.table[i + cur].lexema == LEX_LITERAL)) {
                        IT::IDDATATYPE type = lex.idtable.table[lex.lextable.table[i + cur].idxTI].iddatatype;
                        if (type != IT::DT_INT) {
                            throw ERROR_THROW_IN(710, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                        }
                    }
                    cur++;
                }
            }
        }
    }
    struct FunctionInfo {
        std::string name;
        IT::IDDATATYPE returnType;
        bool hasReturnStatement;
        bool returnsCorrectType;
        int declarationLine;
        int declarationCol;
        int bodyStartPos; // позиция начала тела функции
        int bodyEndPos;   // позиция конца тела функции
    };

    std::vector<FunctionInfo> functionInfos;
    void collectFunctionInfo(MFST::LEX lex) {
        functionInfos.clear();

        for (int i = 0; i < lex.lextable.size; i++) {
            if (lex.lextable.table[i].lexema == LEX_FUNC &&
                i + 1 < lex.lextable.size &&
                lex.lextable.table[i + 1].lexema == LEX_ID) {

                int funcIdx = lex.lextable.table[i + 1].idxTI;
                if (funcIdx != LT_TI_NULLIDX) {
                    IT::Entry& funcEntry = lex.idtable.table[funcIdx];

                    if (funcEntry.idtype == IT::FUNCTION) {
                        FunctionInfo info;
                        info.name = funcEntry.id;
                        info.returnType = funcEntry.iddatatype;
                        info.hasReturnStatement = false;
                        info.returnsCorrectType = false;
                        info.declarationLine = lex.lextable.table[i].sn;
                        info.declarationCol = lex.lextable.table[i].cn;

                        // Находим начало тела функции
                        int pos = i + 1;
                        while (pos < lex.lextable.size &&
                            lex.lextable.table[pos].lexema != LEX_LEFTBRACE) {
                            pos++;
                        }
                        info.bodyStartPos = (pos < lex.lextable.size) ? pos + 1 : -1;

                        // Находим конец тела функции
                        if (info.bodyStartPos != -1) {
                            int braceCount = 1;
                            pos = info.bodyStartPos;
                            while (pos < lex.lextable.size && braceCount > 0) {
                                if (lex.lextable.table[pos].lexema == LEX_LEFTBRACE) braceCount++;
                                if (lex.lextable.table[pos].lexema == LEX_BRACELET) braceCount--;
                                pos++;
                            }
                            info.bodyEndPos = (braceCount == 0) ? pos - 1 : -1;
                        }
                        else {
                            info.bodyEndPos = -1;
                        }

                        functionInfos.push_back(info);
                    }
                }
            }
        }
    }
    void checkFunctionReturns(MFST::LEX lex) {
        for (auto& funcInfo : functionInfos) {
            // Пропускаем функции без типа возврата (void/unknown)
            if (funcInfo.returnType == IT::DT_UNKNOWN) {
                continue;
            }

            // Проверяем только если тело функции найдено
            if (funcInfo.bodyStartPos == -1 || funcInfo.bodyEndPos == -1) {
                continue;
            }

            bool hasReturn = false;
            bool correctReturnType = false;

            // Ищем return в теле функции
            for (int i = funcInfo.bodyStartPos; i <= funcInfo.bodyEndPos; i++) {
                if (lex.lextable.table[i].lexema == LEX_RETURN) {
                    hasReturn = true;

                    // Проверяем тип возвращаемого значения
                    if (i + 1 < lex.lextable.size) {
                        int nextIdx = lex.lextable.table[i + 1].idxTI;
                        if (nextIdx != LT_TI_NULLIDX) {
                            IT::Entry& returnEntry = lex.idtable.table[nextIdx];

                            if (funcInfo.returnType == IT::DT_INT &&
                                returnEntry.iddatatype == IT::DT_INT) {
                                correctReturnType = true;
                            }
                            else if (funcInfo.returnType == IT::DT_BOOL &&
                                returnEntry.iddatatype == IT::DT_BOOL) {
                                correctReturnType = true;
                            }
                            else if (funcInfo.returnType == IT::DT_CHAR &&
                                returnEntry.iddatatype == IT::DT_CHAR) {
                                correctReturnType = true;
                            }
                            else if (funcInfo.returnType == IT::DT_STR &&
                                returnEntry.iddatatype == IT::DT_STR) {
                                correctReturnType = true;
                            }
                            // Также проверяем, если возвращается функция с правильным типом
                            else if (returnEntry.idtype == IT::FUNCTION &&
                                returnEntry.iddatatype == funcInfo.returnType) {
                                correctReturnType = true;
                            }
                        }
                    }

                    // Проверяем также литералы
                    if (!correctReturnType && i + 1 < lex.lextable.size) {
                        if (lex.lextable.table[i + 1].lexema == LEX_LITERAL) {
                            int litIdx = lex.lextable.table[i + 1].idxTI;
                            if (litIdx != LT_TI_NULLIDX) {
                                IT::Entry& litEntry = lex.idtable.table[litIdx];
                                if (litEntry.iddatatype == funcInfo.returnType) {
                                    correctReturnType = true;
                                }
                            }
                        }
                    }

                    break;
                }
            }

            if (!hasReturn) {
                throw ERROR_THROW_IN(716, funcInfo.declarationLine, funcInfo.declarationCol);
            }

            // Ошибка: функция возвращает значение неправильного типа
            if (hasReturn && !correctReturnType) {
                for (int i = funcInfo.bodyStartPos; i <= funcInfo.bodyEndPos; i++) {
                    if (lex.lextable.table[i].lexema == LEX_RETURN) {

                        throw ERROR_THROW_IN(700, lex.lextable.table[i].sn, lex.lextable.table[i].cn);
                    }
                }
            }
        }
    }
    bool startSA(MFST::LEX lex) {
        collectFunctionInfo(lex);

        checkFunctionReturns(lex);
        functions(lex);
        operands(lex);
        literals(lex);
        cycles(lex);
        return true;
    };
     
}