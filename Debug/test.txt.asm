.586P
.model flat, stdcall
includelib msvcrtd.lib
includelib vcruntimed.lib
includelib ucrtd.lib
includelib legacy_stdio_definitions.lib
includelib kernel32.lib
includelib ../Debug/TEA-2025Lib.lib
ExitProcess PROTO : DWORD

SetConsoleCP PROTO : DWORD

SetConsoleOutputCP PROTO : DWORD

power_of PROTO : DWORD, : DWORD 

to_str PROTO : DWORD 

prints PROTO : DWORD 

printi PROTO : DWORD 

printb PROTO : DWORD 

.stack 4096

.const
DIVBYZERO DB "Runtime Error: Division by zero", 0
OVERFLOW_ERROR DB "Runtime Error: Integer overflow", 0
LIT1 DWORD 1 ; int
LIT2 DWORD 2 ; int
LIT3 DWORD 3 ; int
LIT4 DWORD 106 ; char
LIT5 DWORD 5 ; int
LIT6 DB "hi", 0 ; string
LIT7 DB "hi1", 0 ; string
LIT8 DWORD 1 ; boolean
LIT9 DB "<<< TEA-2025 OPERATIONS DEMO >>>", 0 ; string
LIT10 DB " ", 0 ; string
LIT11 DWORD 3 ; int
LIT12 DWORD 2 ; int
LIT13 DB "1. Arithmetic Operations:", 0 ; string
LIT14 DB "Square (2 * 2): ", 0 ; string
LIT15 DB "Power Lib (2^5): ", 0 ; string
LIT16 DB " ", 0 ; string
LIT17 DB "2. Data Types:", 0 ; string
LIT18 DB "Char output: ", 0 ; string
LIT19 DB "String output: ", 0 ; string
LIT20 DB " ", 0 ; string
LIT21 DB "3. Function Calls:", 0 ; string
LIT22 DB "Sum function (10 + 5): ", 0 ; string
LIT23 DWORD 10 ; int
LIT24 DWORD 5 ; int
LIT25 DB "Cycle function (2^3): ", 0 ; string
LIT26 DWORD 3 ; int
LIT27 DB " ", 0 ; string
LIT28 DB "4. Loops and Logic:", 0 ; string
LIT29 DWORD 2 ; int
LIT30 DWORD 6 ; int
LIT31 DWORD 1 ; int
LIT32 DB "Cycle result (start -2, add 1 six times): ", 0 ; string
LIT33 DB "Comparison result (cyc != col): ", 0 ; string
LIT34 DB " ", 0 ; string
LIT35 DB "5. Unary Operations:", 0 ; string
LIT36 DWORD 2 ; int
LIT37 DB "Postfix increment/decrement (2++ then --): ", 0 ; string
LIT38 DWORD 10 ; int
LIT39 DWORD 5 ; int
LIT40 DB "Unary minus assignment (x = -5): ", 0 ; string
LIT41 DB "Complex unary minus (z = -(x+y)): ", 0 ; string
LIT42 DB " ", 0 ; string
LIT43 DB "6. Boolean Logic:", 0 ; string
LIT44 DWORD 1 ; boolean
LIT45 DWORD 0 ; boolean
LIT46 DB "Original True: ", 0 ; string
LIT47 DB "Inverted True (!t): ", 0 ; string
LIT48 DB "Original False: ", 0 ; string
LIT49 DB "Inverted False (!f): ", 0 ; string
LIT50 DB "Comparison expression (x > z): ", 0 ; string
LIT51 DB "Inverted comparison result: ", 0 ; string
LIT52 DWORD 3 ; int
LIT53 DWORD 4 ; int
LIT54 DB "Boolean function result (4 > 3): ", 0 ; string
LIT55 DB " ", 0 ; string
LIT56 DB "7. Modulo Operation:", 0 ; string
LIT57 DWORD 6 ; int
LIT58 DWORD 4 ; int
LIT59 DWORD 2 ; int
LIT60 DB "6 % 4 result: ", 0 ; string
LIT61 DWORD 5 ; int
LIT62 DWORD 2 ; int
LIT63 DWORD 3 ; int
LIT64 DWORD 10 ; int
LIT65 DWORD 10 ; int
LIT66 DWORD 2 ; int
LIT67 DWORD 8 ; int
LIT68 DWORD 2 ; int
LIT69 DWORD 33 ; int

.data
sum$tmp DWORD 0
fact$acc DWORD 0
bool$res DWORD 0
test_fi$res DWORD 0
main$counter DWORD 0
main$result DWORD 0
main$symbol DWORD 0
main$base DWORD 0
main$power DWORD 0
main$st DWORD 0
main$st2 DWORD 0
main$acc DWORD 0
main$bl DWORD 0
main$rSum DWORD 0
test_fi$resfact DWORD 0
test_fi$cyc DWORD 0
test_fi$col DWORD 0
test_fi$b_res DWORD 0
test_fi$count DWORD 0
test_fi$x DWORD 0
test_fi$y DWORD 0
test_fi$z DWORD 0
test_fi$t DWORD 0
test_fi$f DWORD 0
test_fi$nt DWORD 0
test_fi$nf DWORD 0
test_fi$r DWORD 0
test_fi$rr DWORD 0
test_fi$v DWORD 0
test_fi$div DWORD 0
test_fi$test DWORD 0
test_fi$an_test DWORD 0
test_fi$test_one DWORD 0
test_fi$test_two DWORD 0
test_fi$test_three DWORD 0
test_fi$fi_test DWORD 0
test_fi$over_test DWORD 0

.code

Fsum PROC uses ebx ecx edi esi, sum$x : DWORD, sum$y : DWORD

; Init sum$tmp
mov eax, sum$x
add eax, sum$y

; Overflow check for addition
jo overflow_error_1
jmp overflow_check_1
overflow_error_1:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_1:
mov sum$tmp, eax

; Return
mov eax, sum$tmp
ret
ret
Fsum ENDP

Ffact PROC uses ebx ecx edi esi, fact$n : DWORD

; Assign: fact$acc = LIT1
mov eax, LIT1
mov fact$acc, eax

; --- Cycle ---
mov eax, fact$n
mov ecx, eax
cmp ecx, 0
jle cycle_label_1_end
cycle_label_1:
push ecx
mov eax, fact$acc
imul eax, LIT2

; Overflow check for multiplication
jo overflow_error_1
jmp overflow_check_1
overflow_error_1:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_1:

; Assign: fact$acc = eax
mov fact$acc, eax
pop ecx
dec ecx
cmp ecx, 0
jg cycle_label_1
cycle_label_1_end:

; Return
mov eax, fact$acc
ret
ret
Ffact ENDP

Fbool PROC uses ebx ecx edi esi, bool$w1 : DWORD, bool$w2 : DWORD
mov eax, bool$w2
cmp eax, bool$w1
mov ecx, 0
setg cl
mov eax, ecx

; Assign: bool$res = eax
mov bool$res, eax

; Return
mov eax, bool$res
ret
ret
Fbool ENDP

Ftest_fi PROC uses ebx ecx edi esi, test_fi$w1 : DWORD, test_fi$w2 : DWORD

; Init test_fi$res
push LIT3
push test_fi$w2
call power_of
mov ebx, eax
mov eax, test_fi$w1
add eax, ebx

; Overflow check for addition
jo overflow_error_1
jmp overflow_check_1
overflow_error_1:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_1:
mov test_fi$res, eax

; Return
mov eax, test_fi$res
ret
ret
Ftest_fi ENDP

F_DivisionByZero PROC
push offset DIVBYZERO
call prints
push -1
call ExitProcess
F_DivisionByZero ENDP


F_IntegerOverflow PROC
push ebp
mov ebp, esp

; Выводим сообщение об ошибке
push offset OVERFLOW_ERROR
call prints

; Завершаем программу с кодом ошибки
push -1
call ExitProcess
mov esp, ebp
pop ebp
ret 4
F_IntegerOverflow ENDP

main PROC
Invoke SetConsoleCP, 1251
Invoke SetConsoleOutputCP, 1251

; Init main$symbol
mov eax, LIT4
mov main$symbol, eax

; Init main$power
mov eax, LIT5
mov main$power, eax

; Init main$st
mov eax, offset LIT6
mov main$st, eax

; Init main$st2
mov eax, offset LIT7
mov main$st2, eax

; Init main$bl
mov eax, LIT8
mov main$bl, eax
push offset LIT9
call prints
push offset LIT10
call prints

; Assign: main$counter = LIT11
mov eax, LIT11
mov main$counter, eax

; Assign: main$base = LIT12
mov eax, LIT12
mov main$base, eax
push offset LIT13
call prints
mov eax, main$base
imul eax, main$base

; Overflow check for multiplication
jo overflow_error_1
jmp overflow_check_1
overflow_error_1:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_1:

; Assign: main$result = eax
mov main$result, eax
push offset LIT14
call prints
push main$result
call to_str
push eax
call prints
push main$power
push main$base
call power_of

; Assign: main$result = eax
mov main$result, eax
push offset LIT15
call prints
push main$result
call to_str
push eax
call prints
push offset LIT16
call prints
push offset LIT17
call prints
push offset LIT18
call prints
push offset main$symbol
call prints
push offset LIT19
call prints
push main$st
call prints
push offset LIT20
call prints
push offset LIT21
call prints
push offset LIT22
call prints
push LIT24
push LIT23
call Fsum
push eax
call printi
push offset LIT25
call prints

; Init test_fi$resfact
push LIT26
call Ffact
mov test_fi$resfact, eax
mov eax, test_fi$resfact
push eax
call printi
push offset LIT27
call prints
push offset LIT28
call prints

; Init test_fi$cyc
mov eax, LIT29
neg eax

; Overflow check for negation
jo overflow_error_2
jmp overflow_check_2
overflow_error_2:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_2:
mov test_fi$cyc, eax

; Init test_fi$col
mov eax, LIT30
mov test_fi$col, eax

; --- Cycle ---
mov eax, test_fi$col
mov ecx, eax
cmp ecx, 0
jle cycle_label_1_end
cycle_label_1:
push ecx
mov eax, test_fi$cyc
add eax, LIT31

; Overflow check for addition
jo overflow_error_3
jmp overflow_check_3
overflow_error_3:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_3:

; Assign: test_fi$cyc = eax
mov test_fi$cyc, eax
pop ecx
dec ecx
cmp ecx, 0
jg cycle_label_1
cycle_label_1_end:
push offset LIT32
call prints
mov eax, test_fi$cyc
push eax
call printi
mov eax, test_fi$cyc
cmp eax, test_fi$col
mov ecx, 0
setne cl
mov eax, ecx

; Assign: test_fi$b_res = eax
mov test_fi$b_res, eax
push offset LIT33
call prints
push test_fi$b_res
call printb
push offset LIT34
call prints
push offset LIT35
call prints

; Init test_fi$count
mov eax, LIT36
mov test_fi$count, eax

; Postfix
mov eax, test_fi$count
push eax
inc test_fi$count
pop eax

; Assign: test_fi$count = eax
mov test_fi$count, eax

; Postfix
mov eax, test_fi$count
push eax
dec test_fi$count
pop eax

; Assign: test_fi$count = eax
mov test_fi$count, eax
push offset LIT37
call prints
mov eax, test_fi$count
push eax
call printi

; Init test_fi$y
mov eax, LIT38
mov test_fi$y, eax
mov eax, LIT39
neg eax

; Overflow check for negation
jo overflow_error_4
jmp overflow_check_4
overflow_error_4:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_4:

; Assign: test_fi$x = eax
mov test_fi$x, eax
mov eax, test_fi$y
neg eax

; Overflow check for negation
jo overflow_error_5
jmp overflow_check_5
overflow_error_5:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_5:

; Assign: test_fi$z = eax
mov test_fi$z, eax
mov eax, test_fi$x
add eax, test_fi$y

; Overflow check for addition
jo overflow_error_6
jmp overflow_check_6
overflow_error_6:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_6:
neg eax

; Overflow check for negation
jo overflow_error_7
jmp overflow_check_7
overflow_error_7:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_7:

; Assign: test_fi$z = eax
mov test_fi$z, eax
push offset LIT40
call prints
mov eax, test_fi$x
push eax
call printi
push offset LIT41
call prints
mov eax, test_fi$z
push eax
call printi
push offset LIT42
call prints
push offset LIT43
call prints

; Init test_fi$t
mov eax, LIT44
mov test_fi$t, eax

; Init test_fi$f
mov eax, LIT45
mov test_fi$f, eax

; Init test_fi$nt
mov eax, test_fi$t
cmp eax, 0
sete al
movzx eax, al
mov test_fi$nt, eax

; Init test_fi$nf
mov eax, test_fi$f
cmp eax, 0
sete al
movzx eax, al
mov test_fi$nf, eax
push offset LIT46
call prints
push test_fi$t
call printb
push offset LIT47
call prints
push test_fi$nt
call printb
push offset LIT48
call prints
push test_fi$f
call printb
push offset LIT49
call prints
push test_fi$nf
call printb
mov eax, test_fi$x
cmp eax, test_fi$z
mov ecx, 0
setg cl
mov eax, ecx

; Assign: test_fi$r = eax
mov test_fi$r, eax
push offset LIT50
call prints
push test_fi$r
call printb
mov eax, test_fi$r
cmp eax, 0
sete al
movzx eax, al

; Assign: test_fi$rr = eax
mov test_fi$rr, eax
push offset LIT51
call prints
push test_fi$rr
call printb
push LIT53
push LIT52
call Fbool

; Assign: test_fi$v = eax
mov test_fi$v, eax
push offset LIT54
call prints
push test_fi$v
call printb
push offset LIT55
call prints
push offset LIT56
call prints

; Init test_fi$div
mov eax, LIT57
mov test_fi$div, eax

; Safe Division Check
mov ebx, LIT59
cmp ebx, 0
jne div_ok_1
call F_DivisionByZero
div_ok_1:
mov eax, LIT58
cdq
idiv ebx
mov ebx, eax
mov eax, test_fi$div
add eax, ebx

; Overflow check for addition
jo overflow_error_8
jmp overflow_check_8
overflow_error_8:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_8:

; Assign: test_fi$div = eax
mov test_fi$div, eax
push offset LIT60
call prints
mov eax, test_fi$div
push eax
call printi

; Init test_fi$test
mov eax, LIT61
mov test_fi$test, eax
mov eax, test_fi$test
push eax
call printi
mov eax, test_fi$test
not eax

; Assign: test_fi$test = eax
mov test_fi$test, eax
mov eax, test_fi$test
push eax
call printi
push LIT63
push LIT62
call power_of
mov ebx, eax
mov eax, test_fi$test
add eax, ebx

; Overflow check for addition
jo overflow_error_9
jmp overflow_check_9
overflow_error_9:
push 131  ; ERROR_THROW(131) - Integer overflow
call F_IntegerOverflow
overflow_check_9:

; Assign: test_fi$test = eax
mov test_fi$test, eax
mov eax, test_fi$test
push eax
call printi

; Init test_fi$an_test
mov eax, LIT64
not eax
mov test_fi$an_test, eax
mov eax, test_fi$an_test
push eax
call printi

; Init test_fi$test_one
mov eax, LIT65
mov test_fi$test_one, eax

; Init test_fi$test_two
mov eax, LIT66
mov test_fi$test_two, eax

; Init test_fi$test_three
mov eax, LIT67
mov test_fi$test_three, eax

; Init test_fi$fi_test
push test_fi$test_two
push test_fi$test_one
call Ftest_fi
mov test_fi$fi_test, eax
mov eax, test_fi$fi_test
push eax
call printi

; Init test_fi$over_test
push LIT69
push LIT68
call power_of
mov test_fi$over_test, eax
mov eax, test_fi$over_test
push eax
call printi
push -1
call ExitProcess
main ENDP
end main
