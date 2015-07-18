main:
    PSH1 13
    CAL2 fib
    POP
    PSHA
    CAL2 printint8
    POP
    PSH2 endl
    CAL2 puts
    POP1 2
    RET

fib:
    LAP1 1
    CMP1 1
    JG2 .recur
    RET
.recur:
    CC
    DECA
    PSHA
    CAL2 fib
    POPX
    PSHA
    TXA
    DECA
    PSHA
    CAL2 fib
    POP
    TSPI
    DECI
    ADD
    POP1 1
    RET

%file lib.asm
