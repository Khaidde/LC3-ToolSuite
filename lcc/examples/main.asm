.orig x3000
	LD R6, STACK
	JSR main
	LDR R0, R6, 0
	TRAP x21
	ADD R6, R6, 1
	HALT
STACK	.fill xF000
max
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
	BRnz endif_max_0
	AND R0, R0, 0	;; Clear R0
	STR R0, R5, 3	;; Set return value to value in R0
	BR maxEnd
endif_max_0

	AND R0, R0, 0	;; Set R0 to 1
	ADD R0, R0, 1
	STR R0, R5, 3	;; Set return value to value in R0
	BR maxEnd
maxEnd
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
div
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
	LDR R0, R5, 5	;; Load 'y' into R0
	LDR R1, R5, 4	;; Load 'x' into R1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	BRnz endif_div_0
	AND R0, R0, 0	;; Clear R0
	STR R0, R5, 3	;; Set return value to value in R0
	BR divEnd
endif_div_0

	;; Call div
	LDR R0, R5, 5	;; Load 'y' into R0
	ADD R6, R6, -1	;; Push arg 1
	STR R0, R6, 0
	LDR R0, R5, 4	;; Load 'x' into R0
	LDR R1, R5, 5	;; Load 'y' into R1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR div
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 3
	AND R1, R1, 0	;; Set R1 to 1
	ADD R1, R1, 1
	ADD R0, R0, R1	;; R0 = R0 + R1
	STR R0, R5, 3	;; Set return value to value in R0
	BR divEnd
divEnd
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
map
	ADD R6, R6, -4
	STR R7, R6, 2
	STR R5, R6, 1
	ADD R5, R6, 0
	ADD R6, R6, -11
	STR R0, R6, 4
	STR R1, R6, 3
	STR R2, R6, 2
	STR R3, R6, 1
	STR R4, R6, 0

	;; Begin Function
	AND R0, R0, 0	;; Clear R0
	STR R0, R5, -2	;; Store R0 in 'i'

while_map_0
	LDR R0, R5, -2	;; Load 'i' into R0
	LDR R1, R5, 5	;; Load 'length' into R1
	NOT R1, R1	;; R0 = R0 - R1
	ADD R1, R1, 1
	ADD R0, R0, R1
	BRzp endwhile_map_0
	LDR R0, R5, 4	;; Load 'array' into R0
	LDR R1, R5, -2	;; Load 'i' into R1
	ADD R0, R0, R1	;; Load R0 at offset R1
	LDR R0, R0, 0
	STR R0, R5, -3	;; first = R0
	LDR R0, R5, 4	;; Load 'array' into R0
	LDR R1, R5, -2	;; Load 'i' into R1
	AND R2, R2, 0	;; Set R2 to 1
	ADD R2, R2, 1
	ADD R1, R1, R2	;; R1 = R1 + R2
	ADD R0, R0, R1	;; Load R0 at offset R1
	LDR R0, R0, 0
	STR R0, R5, -4	;; second = R0
	;; Call div
	LDR R0, R5, -4	;; Load 'second' into R0
	ADD R6, R6, -1	;; Push arg 1
	STR R0, R6, 0
	LDR R0, R5, -3	;; Load 'first' into R0
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR div
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 3
	STR R0, R5, -5	;; d = R0
	;; Call max
	LDR R0, R5, -4	;; Load 'second' into R0
	ADD R6, R6, -1	;; Push arg 1
	STR R0, R6, 0
	LDR R0, R5, -3	;; Load 'first' into R0
	ADD R6, R6, -1	;; Push arg 0
	STR R0, R6, 0
	JSR max
	LDR R0, R6, 0	;; Store call value in R0
	ADD R6, R6, 3
	STR R0, R5, -6	;; off = R0
	LDR R0, R5, -5	;; Load 'd' into R0
	LDR R1, R5, 4	;; Load 'array' into R1
	LDR R2, R5, -2	;; Load 'i' into R2
	LDR R3, R5, -6	;; Load 'off' into R3
	ADD R2, R2, R3	;; R2 = R2 + R3
	ADD R1, R1, R2	;; Get offset of R1 and store R0
	STR R0, R1, 0

	LDR R0, R5, -2	;; Load 'i' into R0
	AND R1, R1, 0	;; Set R1 to 2
	ADD R1, R1, 2
	ADD R0, R0, R1	;; R0 = R0 + R1
	STR R0, R5, -2	;; i = R0
	BR while_map_0
endwhile_map_0

	AND R0, R0, 0	;; Clear R0
	STR R0, R5, 3	;; Set return value to value in R0
	BR mapEnd
mapEnd
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
main
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
	AND R0, R0, 0	;; Clear R0
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
.end
