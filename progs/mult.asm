%file lib.asm

main:
    CAL2 int8in  
    PSHA
    CAL2 int8in
    LDX1 0
    POPY
    MUL
    PSHX
    PSHY
    CAL2 printint16
    PSH2 endl
    CAL2 puts
    RET
