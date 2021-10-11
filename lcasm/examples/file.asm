;; AND,R0, R1,#32766,#32767,#32768
  ;; LD LDR8
  ;; ADD LD
;; asdf:;test
.ORIG #100

ROOT ADD R7, R3, R2
  AND R0, R3, R2
a AND R3, R3, R0
  STR R3, R0, #31
  ADD R3, R3, #-16
  BRnzp a
  JSR ROOT
  ADD R3, R3, R1
  NOT R7, R0
