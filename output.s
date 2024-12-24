.section .data
.section .text
.global _start
_start:
x: .word 0
MOV R0, #5
STR R0, [x]
LDR R1, [x]
MOV R2, #5
CMP R1, R2
MOV R3, #0
MOVEQ R3, #1
CMP R3, #1
BNE L0
LDR R4, [x]
MOV R5, #1
ADD R6, R4, R5
STR R6, [x]
L0:
MOV R7, #1
MOV R0, #0
SWI 0
