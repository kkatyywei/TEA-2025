.586P
.model flat, stdcall
includelib libucrt.lib
includelib kernel32.lib
includelib ./MathOperations.lib
ExitProcess PROTO : DWORD

SetConsoleCP PROTO : DWORD

SetConsoleOutputCP PROTO : DWORD

Pow PROTO : BYTE, : BYTE 

Sum PROTO : BYTE, : BYTE 

StrCmp PROTO : DWORD, : DWORD 

writestr PROTO : DWORD 

writeint PROTO : BYTE 

writebool PROTO : BYTE 

writechar PROTO : BYTE 

.stack 4096

.const
LIT1 DB "Hello, World!", 0 ; text
LIT2 DB "Hello, World!", 0 ; text
LIT3 BYTE 1 ; boolean
LIT4 SBYTE 1 ; byte
LIT5 SBYTE 3 ; byte
LIT6 BYTE 0 ; boolean
LIT7 DB "Hello, my name is Vlad!", 0 ; text
LIT8 DB "19.12.2023", 0 ; text
LIT9 SBYTE 120 ; byte
LIT10 SBYTE 127 ; byte
LIT11 SBYTE 120 ; byte
LIT12 BYTE 71 ; symbol
LIT13 SBYTE 9 ; byte
LIT14 SBYTE 2 ; byte
LIT15 SBYTE 120 ; byte
LIT16 SBYTE -100 ; byte
LIT17 BYTE 1 ; boolean
LIT18 DB "Bool is true", 0 ; text

.data
setBool$b BYTE 0 ; boolean
sayHello$todaysData DWORD 0 ; text
main$number SBYTE 0 ; byte
main$sym BYTE 0 ; symbol
main$txt DWORD 0 ; text
main$bool BYTE 0 ; boolean

.code

FgetNum PROC uses ebx ecx edi esi, getNum$n : SBYTE
;return
movsx eax, getNum$n
mov getNum$n, al
ret
FgetNum ENDP


FsetHelloText PROC uses ebx ecx edi esi
push offset LIT1
CALL writestr

;return
mov eax, OFFSET LIT2
ret
FsetHelloText ENDP


FsetSym PROC uses ebx ecx edi esi, setSym$a : BYTE
;return
movzx eax, setSym$a
ret
FsetSym ENDP


FsetBool PROC uses ebx ecx edi esi
; string #15 : i:l
mov al, LIT3
mov setBool$b, al 

If_Begin1: 
movsx eax, LIT4
movsx ebx, LIT5
cmp eax, ebx
jle If_End1
jmp If_End2
If_End1:

; string #17 : i:l
mov al, LIT6
mov setBool$b, al 
If_End2:

;return
movzx eax, setBool$b
ret
FsetBool ENDP


FsayHello PROC uses ebx ecx edi esi
push offset LIT7
CALL writestr

; string #24 : i:l
push offset LIT8
pop eax
mov sayHello$todaysData, eax 

push sayHello$todaysData
CALL writestr
ret
FsayHello ENDP

main PROC
Invoke SetConsoleCP, 1251
Invoke SetConsoleOutputCP, 1251

; string #29 : i:l
mov al, LIT9
mov main$number, al 

If_Begin4: 
movzx eax, setBool
mov ebx, 1
cmp eax, ebx
je If_End4
jmp If_End5
movzx eax, setBool
movsx ebx, LIT10
cmp eax, ebx
jle If_End4
jmp If_End5
If_End4:

If_Begin7: 
movsx eax, main$number
movsx ebx, LIT10
cmp eax, ebx
jle If_End7
jmp If_End8
If_End7:

; string #35 : i:i@0
CALL FsetHelloText
mov main$txt, eax

push main$txt
CALL writestr
If_End8:
If_End5:

If_Begin10: 
movsx eax, main$number
movsx ebx, LIT11
cmp eax, ebx
je If_End10
jmp If_End11
If_End10:

; string #39 : i:l
mov al, LIT12
mov main$sym, al 
push eax
movzx eax, main$sym
push eax
CALL writechar
pop eax

If_End11:

; string #42 : i:ill@2#
movsx eax, LIT14
push eax
movsx eax, LIT13
push eax
CALL Pow
mov main$number, al

movsx eax, main$number
push eax
CALL writeint

; string #44 : i:l
mov al, LIT15
mov main$number, al 

; string #45 : i:iil@2#
movsx eax, LIT16
push eax
movsx eax, main$number
push eax
CALL Sum
mov main$number, al

movsx eax, main$number
push eax
CALL writeint

; string #47 : i:l
mov al, LIT17
mov main$bool, al 

If_Begin13: 
movzx eax, main$bool
mov ebx, 1
cmp eax, ebx
je If_End13
jmp If_End14
If_End13:

push offset LIT18
CALL writestr
If_End14:
CALL FsayHello
push -1
call ExitProcess
main ENDP
end main
