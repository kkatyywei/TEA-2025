#include "CodeGeneration.h"
#include <stack>
#include <string>
#include <vector>
#include <iostream>
namespace CodeGeneration {

    static int loops = 1;
    static int divChecks = 1;
    static int overflowChecks = 1;

    void AddOverflowCheck(Out::OUT out, const std::string& operation, const std::string& labelPrefix = "overflow_check_") {
        std::string okLabel = labelPrefix + std::to_string(overflowChecks);
        std::string errorLabel = "overflow_error_" + std::to_string(overflowChecks);

        *out.stream << "\n; Overflow check for " << operation.c_str() << "\n";  
        *out.stream << "jo " << errorLabel.c_str() << "\n";  
        *out.stream << "jmp " << okLabel.c_str() << "\n"; 

        *out.stream << errorLabel.c_str() << ":\n"; 
        *out.stream << "push " << 131 << "  ; ERROR_THROW(131) - Integer overflow\n";
        *out.stream << "call F_IntegerOverflow\n";

        *out.stream << okLabel.c_str() << ":\n";  
        overflowChecks++;
    }

    void AddIntegerOverflowProcedure(Out::OUT out) {
        *out.stream << "\nF_IntegerOverflow PROC\n";
        *out.stream << "push ebp\n";
        *out.stream << "mov ebp, esp\n";
        *out.stream << "\n; Выводим сообщение об ошибке\n";
        *out.stream << "push offset OVERFLOW_ERROR\n";
        *out.stream << "call prints\n";
        *out.stream << "\n; Завершаем программу с кодом ошибки\n";
        *out.stream << "push -1\n";
        *out.stream << "call ExitProcess\n";
        *out.stream << "mov esp, ebp\n";
        *out.stream << "pop ebp\n";
        *out.stream << "ret 4\n";
        *out.stream << "F_IntegerOverflow ENDP\n";
    }

    int findMatchingRightBrace(LT::LexTable& lextable, int startPos) {
        int balance = 0;
        for (int i = startPos; i < lextable.size; i++) {
            if (lextable.table[i].lexema == LEX_LEFTBRACE) balance++;
            if (lextable.table[i].lexema == LEX_BRACELET) { // '}'
                balance--;
                if (balance == 0) return i;
            }
        }
        return -1;
    }

    void Head(Out::OUT out) {
        *out.stream << ".586P\n";
        *out.stream << ".model flat, stdcall\n";
        *out.stream << "includelib msvcrtd.lib\n";
        *out.stream << "includelib vcruntimed.lib\n";
        *out.stream << "includelib ucrtd.lib\n";
        *out.stream << "includelib legacy_stdio_definitions.lib\n";
        *out.stream << "includelib kernel32.lib\n";
        *out.stream << "includelib ../Debug/TEA-2025Lib.lib\n";

        *out.stream << "ExitProcess PROTO : DWORD\n\n";
        *out.stream << "SetConsoleCP PROTO : DWORD\n\n";
        *out.stream << "SetConsoleOutputCP PROTO : DWORD\n\n";
        *out.stream << "power_of PROTO : DWORD, : DWORD \n\n";
        *out.stream << "to_str PROTO : DWORD \n\n";
        *out.stream << "prints PROTO : DWORD \n\n";
        *out.stream << "printi PROTO : DWORD \n\n";
        *out.stream << "printb PROTO : DWORD \n\n";

        *out.stream << ".stack 4096\n\n";
    }

    void Const(Out::OUT out, MFST::LEX lex) {
        *out.stream << ".const\n";
        *out.stream << "DIVBYZERO DB \"Runtime Error: Division by zero\", 0\n";
        *out.stream << "OVERFLOW_ERROR DB \"Runtime Error: Integer overflow\", 0\n";

        for (int i = 0; i < lex.idtable.size; i++) {
            if (lex.idtable.table[i].idtype == IT::LITERAL) {
                *out.stream << lex.idtable.table[i].id;
                switch (lex.idtable.table[i].iddatatype) {
                case IT::DT_INT:
                    *out.stream << " DWORD " << lex.idtable.table[i].value.vint << " ; int";
                    break;
                case IT::DT_CHAR:
                    *out.stream << " DWORD " << (int)lex.idtable.table[i].value.vchar << " ; char";
                    break;
                case IT::DT_STR:
                    *out.stream << " DB " << lex.idtable.table[i].value.vtext->str << ", 0 ; string";
                    break;
                case IT::DT_BOOL:
                    *out.stream << " DWORD " << (lex.idtable.table[i].value.vboolean ? 1 : 0) << " ; boolean";
                    break;
                default: break;
                }
                *out.stream << "\n";
            }
        }
        *out.stream << "\n";
    }

    void Data(Out::OUT out, MFST::LEX lex) {
        *out.stream << ".data\n";
        for (int i = 0; i < lex.idtable.size; i++) {
            if (lex.idtable.table[i].idtype == IT::VARIABLE) {
                *out.stream << lex.idtable.table[i].id << " DWORD 0\n";
            }
        }
        *out.stream << "\n";
    }

    void Expression(Out::OUT out, MFST::LEX lex, int startPos, int endPos) {
        std::stack<std::string> operandStack;

        if (startPos >= lex.lextable.size) return;
        if (endPos > lex.lextable.size) endPos = lex.lextable.size;

        for (int i = startPos; i < endPos; i++) {
            char lexem = lex.lextable.table[i].lexema;

            if (lexem == '#' || (lexem >= '0' && lexem <= '9')) continue;

            // 1. Унарные 
            if (lexem == 'n' || lexem == '~' || lexem == '!') {
                if (!operandStack.empty()) {
                    std::string val = operandStack.top(); operandStack.pop();
                    if (val != "eax") *out.stream << "mov eax, " << val.c_str() << "\n";

                    if (lexem == 'n') {
                        *out.stream << "neg eax\n";
                        // Проверка переполнения для NEG
                        AddOverflowCheck(out, "negation");
                    }
                    else if (lexem == '~') *out.stream << "not eax\n";
                    else if (lexem == '!') {
                        *out.stream << "cmp eax, 0\n";
                        *out.stream << "sete al\n";
                        *out.stream << "movzx eax, al\n";
                    }
                    operandStack.push("eax");
                }
                continue;
            }

            // 1.1 Постфиксные 
            if (lexem == 'u' || lexem == 'd') {
                if (!operandStack.empty()) {
                    std::string id = operandStack.top(); operandStack.pop();
                    *out.stream << "\n; Postfix\n";
                    *out.stream << "mov eax, " << id.c_str() << "\n";
                    *out.stream << "push eax\n";
                    if (lexem == 'u') *out.stream << "inc " << id.c_str() << "\n";
                    else              *out.stream << "dec " << id.c_str() << "\n";
                    *out.stream << "pop eax\n";
                    operandStack.push("eax");
                }
                continue;
            }

            //  2. CYCLE 
            if (lexem == LEX_CYCLE) {
                int pos = i + 1;
                if (pos < endPos && lex.lextable.table[pos].lexema == LEX_LEFTTHESIS) {
                    int exprStart = pos + 1;
                    int exprEnd = exprStart;
                    int parenBalance = 1;
                    while (exprEnd < endPos) {
                        if (lex.lextable.table[exprEnd].lexema == LEX_LEFTTHESIS) parenBalance++;
                        if (lex.lextable.table[exprEnd].lexema == LEX_RIGHTTHESIS) {
                            parenBalance--;
                            if (parenBalance == 0) break;
                        }
                        exprEnd++;
                    }

                    if (exprEnd < endPos) {
                        int bracePos = exprEnd + 1;
                        if (bracePos < endPos && lex.lextable.table[bracePos].lexema == LEX_LEFTBRACE) {
                            int bodyEnd = findMatchingRightBrace(lex.lextable, bracePos);
                            if (bodyEnd != -1) {
                                std::string loopLabel = "cycle_label_" + std::to_string(loops);
                                std::string loopEndLabel = loopLabel + "_end";

                                *out.stream << "\n; --- Cycle ---\n";
                                Expression(out, lex, exprStart, exprEnd);
                                *out.stream << "mov ecx, eax\n";
                                *out.stream << "cmp ecx, 0\n";
                                *out.stream << "jle " << loopEndLabel.c_str() << "\n"; 

                                *out.stream << loopLabel.c_str() << ":\n";  
                                *out.stream << "push ecx\n";

                                Expression(out, lex, bracePos + 1, bodyEnd);

                                *out.stream << "pop ecx\n";
                                *out.stream << "dec ecx\n";
                                *out.stream << "cmp ecx, 0\n";
                                *out.stream << "jg " << loopLabel.c_str() << "\n";  

                                *out.stream << loopEndLabel.c_str() << ":\n";  
                                loops++;
                                i = bodyEnd;
                                continue;
                            }
                        }
                    }
                }
            }

            // 3. RETURN 
            if (lexem == LEX_RETURN) {
                int semiPos = -1;
                for (int k = i + 1; k < endPos; k++) {
                    if (lex.lextable.table[k].lexema == LEX_SEMICOLON) { semiPos = k; break; }
                }
                if (semiPos != -1) {
                    *out.stream << "\n; Return\n";
                    Expression(out, lex, i + 1, semiPos);
                    *out.stream << "ret\n";
                    i = semiPos;
                }
                continue;
            }

            //  4. ANNOUNCE 
            if (lexem == LEX_ANNOUNCE) {
                int semiPos = -1;
                for (int k = i + 1; k < endPos; k++) {
                    if (lex.lextable.table[k].lexema == LEX_SEMICOLON) { semiPos = k; break; }
                }
                if (semiPos != -1) {
                    int assignPos = -1;
                    for (int k = i + 1; k < semiPos; k++) {
                        if (lex.lextable.table[k].lexema == LEX_ASSIGNMENT) { assignPos = k; break; }
                    }
                    int varIdx = -1;
                    if (i + 2 < semiPos) varIdx = lex.lextable.table[i + 2].idxTI;

                    if (varIdx != LT_TI_NULLIDX && varIdx < lex.idtable.size && assignPos != -1) {
                        *out.stream << "\n; Init " << lex.idtable.table[varIdx].id << "\n";
                        Expression(out, lex, assignPos + 1, semiPos);
                        *out.stream << "mov " << lex.idtable.table[varIdx].id << ", eax\n";
                    }
                    i = semiPos;
                }
                continue;
            }

            //  5. Операнды 
            if (lexem == LEX_ID || lexem == LEX_LITERAL) {
                int idxTI = lex.lextable.table[i].idxTI;
                if (idxTI != LT_TI_NULLIDX && idxTI < lex.idtable.size) {
                    operandStack.push(lex.idtable.table[idxTI].id);
                }
                continue;
            }
            if (lexem == 'f') {
                int idxTI = lex.lextable.table[i].idxTI;
                if (idxTI != LT_TI_NULLIDX && idxTI < lex.idtable.size) {
                    operandStack.push(std::string("F") + lex.idtable.table[idxTI].id);
                }
                continue;
            }

            //  6. Вызовы (@) 
            if (lexem == '@') {
                if (i + 1 < endPos) {
                    int paramCount = lex.lextable.table[i + 1].lexema - '0';
                    std::vector<std::string> params;
                    for (int p = 0; p < paramCount && !operandStack.empty(); p++) {
                        params.push_back(operandStack.top()); operandStack.pop();
                    }
                    std::string funcName;
                    if (!operandStack.empty()) { funcName = operandStack.top(); operandStack.pop(); }

                    for (size_t p = 0; p < params.size(); p++) {
                        *out.stream << "push " << params[p].c_str() << "\n";  // Добавить .c_str()
                    }

                    if (funcName == "Fpower_of" || funcName == "power_of") *out.stream << "call power_of\n";
                    else if (funcName == "Fto_str" || funcName == "to_str") *out.stream << "call to_str\n";
                    else if (funcName == "prints") *out.stream << "call prints\n";
                    else if (funcName == "printi") *out.stream << "call printi\n";
                    else {
                        if (funcName.rfind("F", 0) == 0) *out.stream << "call " << funcName.c_str() << "\n";  // Добавить .c_str()
                        else *out.stream << "call F" << funcName.c_str() << "\n";  // Добавить .c_str()
                    }
                    operandStack.push("eax");
                    i++;
                }
                continue;
            }

            //  7. Присваивание 
            if (lexem == LEX_ASSIGNMENT) {
                if (operandStack.size() >= 2) {
                    std::string right = operandStack.top(); operandStack.pop();
                    std::string left = operandStack.top(); operandStack.pop();

                    *out.stream << "\n; Assign: " << left.c_str() << " = " << right.c_str() << "\n";  

                    if (right == "eax") *out.stream << "mov " << left.c_str() << ", eax\n";  
                    else {
                        int rIdx = IT::IsId(lex.idtable, (char*)right.c_str());
                        bool useOffset = false;
                        if (rIdx != TI_NULLIDX) {
                            if (lex.idtable.table[rIdx].iddatatype == IT::DT_STR &&
                                lex.idtable.table[rIdx].idtype == IT::LITERAL) {
                                useOffset = true;
                            }
                        }
                        if (useOffset) *out.stream << "mov eax, offset " << right.c_str() << "\n";  
                        else           *out.stream << "mov eax, " << right.c_str() << "\n";  

                        *out.stream << "mov " << left.c_str() << ", eax\n";  
                    }
                }
                continue;
            }

            //  8. Бинарные операции 
            if (strchr("+-*/%><v^!&|qwxy", lexem) || lexem == LEX_EQUAL || lexem == LEX_NOTEQUAL ||
                lexem == LEX_MOREEQUAL || lexem == LEX_LESSEQUAL) {

                if (operandStack.size() >= 2) {
                    std::string right = operandStack.top(); operandStack.pop();
                    std::string left = operandStack.top(); operandStack.pop();

                    if (lexem == LEX_DIVIDE || lexem == LEX_MOD || lexem == '/' || lexem == '%') {
                        *out.stream << "\n; Safe Division Check\n";
                        *out.stream << "mov ebx, " << right.c_str() << "\n";
                        *out.stream << "cmp ebx, 0\n";

                        std::string continueLabel = "div_ok_" + std::to_string(divChecks);
                        *out.stream << "jne " << continueLabel.c_str() << "\n";
                        *out.stream << "call F_DivisionByZero\n";
                        *out.stream << continueLabel.c_str() << ":\n";
                        divChecks++;

                        *out.stream << "mov eax, " << left.c_str() << "\n";
                        *out.stream << "cdq\n";
                        *out.stream << "idiv ebx\n";

                        if (lexem == LEX_MOD || lexem == '%') *out.stream << "mov eax, edx\n";
                    }
                    else {
                        if (lexem == LEX_PLUS || lexem == '+') {
                            if (right == "eax") {
                                // Если правый операнд уже в eax, сохраняем его в ebx
                                *out.stream << "mov ebx, eax\n";
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "add eax, ebx\n";
                            }
                            else if (left == "eax") {
                                // Если левый операнд уже в eax
                                *out.stream << "add eax, " << right.c_str() << "\n";
                            }
                            else {
                                // Оба операнда не в eax
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "add eax, " << right.c_str() << "\n";
                            }
                            AddOverflowCheck(out, "addition");
                        }
                        else if (lexem == LEX_MINUS || lexem == '-') {
                            if (right == "eax") {
                                // Если правый операнд уже в eax
                                *out.stream << "mov ebx, eax\n";
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "sub eax, ebx\n";
                            }
                            else if (left == "eax") {
                                *out.stream << "sub eax, " << right.c_str() << "\n";
                            }
                            else {
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "sub eax, " << right.c_str() << "\n";
                            }
                            AddOverflowCheck(out, "subtraction");
                        }
                        else if (lexem == LEX_MULTIPLY || lexem == '*') {
                            if (right == "eax") {
                                // Если правый операнд уже в eax
                                *out.stream << "mov ebx, eax\n";
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "imul eax, ebx\n";
                            }
                            else if (left == "eax") {
                                *out.stream << "imul eax, " << right.c_str() << "\n";
                            }
                            else {
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "imul eax, " << right.c_str() << "\n";
                            }
                            AddOverflowCheck(out, "multiplication");
                        }
                        else {
                            // Для сравнений
                            if (right == "eax") {
                                *out.stream << "mov ebx, eax\n";
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "cmp eax, ebx\n";
                            }
                            else if (left == "eax") {
                                *out.stream << "cmp eax, " << right.c_str() << "\n";
                            }
                            else {
                                *out.stream << "mov eax, " << left.c_str() << "\n";
                                *out.stream << "cmp eax, " << right.c_str() << "\n";
                            }

                            *out.stream << "mov ecx, 0\n";
                            if (lexem == LEX_MORE || lexem == '>') *out.stream << "setg cl\n";
                            else if (lexem == LEX_LESS || lexem == '<') *out.stream << "setl cl\n";
                            else if (lexem == LEX_EQUAL || lexem == 'q') *out.stream << "sete cl\n";
                            else if (lexem == LEX_NOTEQUAL || lexem == 'w') *out.stream << "setne cl\n";
                            else if (lexem == LEX_MOREEQUAL || lexem == 'x') *out.stream << "setge cl\n";
                            else if (lexem == LEX_LESSEQUAL || lexem == 'y') *out.stream << "setle cl\n";
                            *out.stream << "mov eax, ecx\n";
                        }
                    }
                    operandStack.push("eax");
                }
                continue;
            }

            //  9. COUT 
            if (lexem == LEX_COUT) {
                int j = i + 2;
                int balance = 1;
                while (j < endPos && balance > 0) {
                    if (lex.lextable.table[j].lexema == LEX_LEFTTHESIS) balance++;
                    if (lex.lextable.table[j].lexema == LEX_RIGHTTHESIS) balance--;
                    if (balance > 0) j++;
                }
                int endExpr = j;

                bool handled = false;
                if (endExpr - (i + 2) == 1) {
                    int idx = lex.lextable.table[i + 2].idxTI;
                    if (idx != LT_TI_NULLIDX && idx < lex.idtable.size) {
                        IT::IDDATATYPE type = lex.idtable.table[idx].iddatatype;
                        if (type == IT::DT_STR) {
                            if (lex.idtable.table[idx].idtype == IT::LITERAL)
                                *out.stream << "push offset " << lex.idtable.table[idx].id << "\n";
                            else
                                *out.stream << "push " << lex.idtable.table[idx].id << "\n";
                            *out.stream << "call prints\n";
                            handled = true;
                        }
                        else if (type == IT::DT_CHAR) {
                            *out.stream << "push offset " << lex.idtable.table[idx].id << "\n";
                            *out.stream << "call prints\n";
                            handled = true;
                        }
                        else if (type == IT::DT_BOOL) {
                            *out.stream << "push " << lex.idtable.table[idx].id << "\n";
                            *out.stream << "call printb\n";
                            handled = true;
                        }
                    }
                }

                if (!handled) {
                    bool usePrintS = false;
                    for (int k = i + 2; k < endExpr; k++) {
                        if (lex.lextable.table[k].lexema == LEX_ID) {
                            int idx = lex.lextable.table[k].idxTI;
                            if (idx != LT_TI_NULLIDX && idx < lex.idtable.size) {
                                if (strcmp(lex.idtable.table[idx].id, "to_str") == 0) usePrintS = true;
                            }
                        }
                    }
                    Expression(out, lex, i + 2, endExpr);
                    *out.stream << "push eax\n";
                    if (usePrintS) *out.stream << "call prints\n";
                    else          *out.stream << "call printi\n";
                }
                while (!operandStack.empty()) operandStack.pop();
                i = endExpr;
                continue;
            }
        }

        // Финал 
        if (!operandStack.empty()) {
            std::string result = operandStack.top();
            if (result != "eax") {
                bool useOffset = false;
                int rIdx = IT::IsId(lex.idtable, (char*)result.c_str());
                if (rIdx != TI_NULLIDX) {
                    if (lex.idtable.table[rIdx].iddatatype == IT::DT_STR &&
                        lex.idtable.table[rIdx].idtype == IT::LITERAL) useOffset = true;
                }
                if (useOffset) *out.stream << "mov eax, offset " << result.c_str() << "\n"; 
                else           *out.stream << "mov eax, " << result.c_str() << "\n";  
            }
        }
    }

    void Functions(Out::OUT out, MFST::LEX lex) {
        for (int i = 0; i < lex.idtable.size; i++) {
            if (lex.idtable.table[i].idtype == IT::FUNCTION) {
                *out.stream << "\nF" << lex.idtable.table[i].id << " PROC uses ebx ecx edi esi";
                int startIdx = lex.idtable.table[i].idxfirstLE;
                int cur = 1;

                while ((startIdx + cur < lex.lextable.size) &&
                    lex.lextable.table[startIdx + cur].lexema != LEX_RIGHTTHESIS) {
                    int idxTI = lex.lextable.table[startIdx + cur].idxTI;
                    if (lex.lextable.table[startIdx + cur].lexema == LEX_ID &&
                        idxTI != LT_TI_NULLIDX && idxTI < lex.idtable.size &&
                        lex.idtable.table[idxTI].idtype == IT::PARAMETER) {
                        *out.stream << ", " << lex.idtable.table[idxTI].id << " : DWORD";
                    }
                    cur++;
                }
                *out.stream << "\n";

                // Сбрасываем счетчики проверок для каждой функции
                int savedDivChecks = divChecks;
                int savedOverflowChecks = overflowChecks;
                int savedLoops = loops;
                divChecks = 1;
                overflowChecks = 1;
                loops = 1;

                while ((startIdx + cur < lex.lextable.size) && lex.lextable.table[startIdx + cur].lexema != '{') cur++;
                if (startIdx + cur >= lex.lextable.size) {
                    *out.stream << "ret\nF" << lex.idtable.table[i].id << " ENDP\n";
                    divChecks = savedDivChecks;
                    overflowChecks = savedOverflowChecks;
                    loops = savedLoops;
                    continue;
                }

                int braceIdx = startIdx + cur;
                int endBody = findMatchingRightBrace(lex.lextable, braceIdx);
                if (endBody == -1) endBody = lex.lextable.size;

                if (endBody > braceIdx + 1) {
                    Expression(out, lex, braceIdx + 1, endBody);
                }
                *out.stream << "ret\n";
                *out.stream << "F" << lex.idtable.table[i].id << " ENDP\n";

                // Восстанавливаем счетчики
                divChecks = savedDivChecks;
                overflowChecks = savedOverflowChecks;
                loops = savedLoops;
            }
        }
    }

    void Code(Out::OUT out, MFST::LEX lex) {
        *out.stream << ".code\n";
        Functions(out, lex);

        *out.stream << "\nF_DivisionByZero PROC\n";
        *out.stream << "push offset DIVBYZERO\n";
        *out.stream << "call prints\n";
        *out.stream << "push -1\n";
        *out.stream << "call ExitProcess\n";
        *out.stream << "F_DivisionByZero ENDP\n\n";

        AddIntegerOverflowProcedure(out);

        *out.stream << "\nmain PROC\n";
        *out.stream << "Invoke SetConsoleCP, 1251\n";
        *out.stream << "Invoke SetConsoleOutputCP, 1251\n";

        int mainPos = 0;
        for (int i = 0; i < lex.lextable.size; i++) {
            if (lex.lextable.table[i].lexema == LEX_MAIN) { mainPos = i; break; }
        }
        if (lex.lextable.size > 2) {
            int startMain = mainPos + 2;
            int endMain = lex.lextable.size - 1;
            Expression(out, lex, startMain, endMain);
        }

        *out.stream << "push -1\n";
        *out.stream << "call ExitProcess\n";
        *out.stream << "main ENDP\n";
        *out.stream << "end main\n";
    }

    void GenerateCode(MFST::LEX lex, Out::OUT out) {
        if (!out.stream || !out.stream->is_open()) return;
        Head(out);
        Const(out, lex);
        Data(out, lex);
        Code(out, lex);
        out.stream->flush();
    }
    void GenerateCodeToASMFile(MFST::LEX lex) {
        std::string asmFolder = "../TEA-2025Asm/";
        std::string fullPath = asmFolder + "TEA-2025.asm";

        Out::OUT out;
        std::wofstream* asmFile = new std::wofstream(fullPath);
        if (!asmFile->is_open()) {
            fullPath = "TEA-2025.asm";
            asmFile->open(fullPath);
            if (!asmFile->is_open()) {
                throw Error::ERROR(Error::geterror(302)); 
            }
        }
        out.stream = asmFile;

        GenerateCode(lex, out);
        asmFile->close();
        delete asmFile;


    }
}
