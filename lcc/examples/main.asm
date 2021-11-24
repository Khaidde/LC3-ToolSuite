.orig x3000
	LEA R0, testStr_arr
	ST R0, testStr
	LD R6, STACK
	JSR main
	LDR R0, R6, 0
	TRAP x21
	ADD R6, R6, 1
	HALT
STACK	.fill xF000
addIfGreater
	ADD R6, R6, -4
	STR R7, R6, 2
	STR R5, R6, 1
	ADD R5, R6, 0
	ADD R6, R6, -6
	STR R0, R6, 4
	STR R1, R6, 3
	STR R2, R6, 2
	STR R3, R6, 1
	STR R4, R6, 0

	;; Begin Function
	;; If statement
	LDR R0, R5, 4	;; Load 'a' into R0
	LDR R1, R5, 5	;; Load 'b' into R1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	BRnz elif_addIfGreater_0
	LDR R0, R5, 4	;; Load 'a' into R0
	LDR R1, R5, 5	;; Load 'b' into R1
	ADD R0, R0, R1	;; R0 = R0 + R1
	STR R0, R5, 3	;; Set return value to value in R0
	BR addIfGreaterEnd
	BR endif_addIfGreater_0
elif_addIfGreater_0
	AND R0, R0, 0	;; Clear R0
	STR R0, R5, 3	;; Set return value to value in R0
	BR addIfGreaterEnd
endif_addIfGreater_0

addIfGreaterEnd
	LDR R4, R6, 0
	LDR R3, R6, 1
	LDR R2, R6, 2
	LDR R1, R6, 3
	LDR R0, R6, 4
	ADD R6, R5, 0
	LDR R5, R6, 1
	LDR R7, R6, 2
	ADD R6, R6, 3
	RET
mult
	ADD R6, R6, -4
	STR R7, R6, 2
	STR R5, R6, 1
	ADD R5, R6, 0
	ADD R6, R6, -7
	STR R0, R6, 4
	STR R1, R6, 3
	STR R2, R6, 2
	STR R3, R6, 1
	STR R4, R6, 0

	;; Begin Function
	AND R0, R0, 0	;; Clear R0
	STR R0, R5, -2	;; Store R0 in 'res'

while_mult_0
	LDR R0, R5, 4	;; Load 'a' into R0
	ADD R0, R0, xFFFF	;; R0--
	STR R0, R5, 4	;; a = R0
	ADD R0, R0, 1
	BRz endwhile_mult_0
	LDR R0, R5, -2	;; Load 'res' into R0
	LDR R1, R5, 5	;; Load 'b' into R1
	ADD R0, R0, R1	;; R0 = R0 + R1
	STR R0, R5, -2	;; res = R0
	BR while_mult_0
endwhile_mult_0

	LDR R0, R5, -2	;; Load 'res' into R0
	STR R0, R5, 3	;; Set return value to value in R0
	BR multEnd
multEnd
	LDR R4, R6, 0
	LDR R3, R6, 1
	LDR R2, R6, 2
	LDR R1, R6, 3
	LDR R0, R6, 4
	ADD R6, R5, 0
	LDR R5, R6, 1
	LDR R7, R6, 2
	ADD R6, R6, 3
	RET
fact
	ADD R6, R6, -4
	STR R7, R6, 2
	STR R5, R6, 1
	ADD R5, R6, 0
	ADD R6, R6, -5
	STR R0, R6, 4
	STR R1, R6, 3
	STR R2, R6, 2
	STR R3, R6, 1
	STR R4, R6, 0

	;; Begin Function
	;; If statement
	LDR R0, R5, 4	;; Load 'n' into R0
	AND R1, R1, 0	;; Set R1 to 1
	ADD R1, R1, 1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	BRp endif_fact_0
	AND R0, R0, 0	;; Set R0 to 1
	ADD R0, R0, 1
	STR R0, R5, 3	;; Set return value to value in R0
	BR factEnd
endif_fact_0

	;; Call mult
	;; Call fact
	LDR R0, R5, 4	;; Load 'n' into R0
	AND R1, R1, 0	;; Set R1 to 1
	ADD R1, R1, 1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR fact
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 2
	ADD R6, R6, -1	;; Push arg 1
	STR R0, R6, 0
	LDR R0, R5, 4	;; Load 'n' into R0
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR mult
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 3
	STR R0, R5, 3	;; Set return value to value in R0
	BR factEnd
factEnd
	LDR R4, R6, 0
	LDR R3, R6, 1
	LDR R2, R6, 2
	LDR R1, R6, 3
	LDR R0, R6, 4
	ADD R6, R5, 0
	LDR R5, R6, 1
	LDR R7, R6, 2
	ADD R6, R6, 3
	RET
strcpy
	ADD R6, R6, -4
	STR R7, R6, 2
	STR R5, R6, 1
	ADD R5, R6, 0
	ADD R6, R6, -6
	STR R0, R6, 4
	STR R1, R6, 3
	STR R2, R6, 2
	STR R3, R6, 1
	STR R4, R6, 0

	;; Begin Function
while_strcpy_0
	LDR R0, R5, 5	;; Load 'src' into R0
	LDR R0, R0, 0	;; R0 = *R0
	BRz endwhile_strcpy_0
	LDR R0, R5, 5	;; Load 'src' into R0
	LDR R0, R0, 0	;; R0 = *R0
	LDR R1, R5, 4	;; Load 'dest' into R1
	STR R0, R1, 0
	LDR R0, R5, 5	;; Load 'src' into R0
	ADD R0, R0, 1	;; R0++
	STR R0, R5, 5	;; src = R0
	ADD R0, R0, xFFFF
	LDR R0, R5, 4	;; Load 'dest' into R0
	ADD R0, R0, 1	;; R0++
	STR R0, R5, 4	;; dest = R0
	ADD R0, R0, xFFFF
	BR while_strcpy_0
endwhile_strcpy_0

strcpyEnd
	LDR R4, R6, 0
	LDR R3, R6, 1
	LDR R2, R6, 2
	LDR R1, R6, 3
	LDR R0, R6, 4
	ADD R6, R5, 0
	LDR R5, R6, 1
	LDR R7, R6, 2
	ADD R6, R6, 3
	RET
staticAdr	.fill 16384
testStr	.blkw 1
testStr_arr	.stringz "Hello World"
main
	ADD R6, R6, -4
	STR R7, R6, 2
	STR R5, R6, 1
	ADD R5, R6, 0
	ADD R6, R6, -8
	STR R0, R6, 4
	STR R1, R6, 3
	STR R2, R6, 2
	STR R3, R6, 1
	STR R4, R6, 0

	;; Begin Function
	;; Call strcpy
	LD R0, testStr	;; Load 'testStr' into R0
	ADD R6, R6, -1	;; Push arg 1
	STR R0, R6, 0
	LD R0, staticAdr	;; Load 'staticAdr' into R0
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR strcpy
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 3
	LD R0, c_main_0	;; Load 16384 into R0
	STR R0, R5, 0	;; Store R0 in 'fourK'

	LDR R0, R5, 0	;; Load 'fourK' into R0
	AND R1, R1, 0	;; Set R1 to 5
	ADD R1, R1, 5
	ADD R0, R0, R1	;; R0 = R0 + R1
	LDR R0, R0, 0	;; R0 = *R0
	STR R0, R5, -1	;; Store R0 in 'ptrArithmetic'

	;; Call fact
	AND R0, R0, 0	;; Set R0 to 7
	ADD R0, R0, 7
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR fact
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 2
	LDR R1, R5, -1	;; Load 'ptrArithmetic' into R1
	ADD R0, R0, R1	;; R0 = R0 + R1
	STR R0, R5, -2	;; Store R0 in 'f'

	LD R0, testStr	;; Load 'testStr' into R0
	AND R1, R1, 0	;; Set R1 to 7
	ADD R1, R1, 7
	ADD R0, R0, R1	;; Load R0 at offset R1
	LDR R0, R0, 0
	STR R0, R5, -3	;; Store R0 in 'arrayIndex'

	LDR R0, R5, -2	;; Load 'f' into R0
	LDR R1, R5, -3	;; Load 'arrayIndex' into R1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	STR R0, R5, -2	;; f = R0
	;; Call addIfGreater
	LD R0, c_main_1	;; Load 29 into R0
	ADD R6, R6, -1	;; Push arg 1
	STR R0, R6, 0
	LDR R0, R5, -2	;; Load 'f' into R0
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR addIfGreater
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 3
	STR R0, R5, 3	;; Set return value to value in R0
	BR mainEnd
mainEnd
	LDR R4, R6, 0
	LDR R3, R6, 1
	LDR R2, R6, 2
	LDR R1, R6, 3
	LDR R0, R6, 4
	ADD R6, R5, 0
	LDR R5, R6, 1
	LDR R7, R6, 2
	ADD R6, R6, 3
	RET
c_main_0	.fill 16384
c_main_1	.fill 29
.end
