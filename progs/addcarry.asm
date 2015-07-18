%file lib.asm

main:
    # Add with carry test
    PSH1 0 # High bits to add
    CAL2 int8in
    PSHA # low bits
    CAL2 int8in
    LIL1 1
    CC
    ADD  # Add low bits
    POP
    PSHA
    LIL1 0
    LDA1 0
    ADD  # High bits + Carry
    STL1 0
    CAL2 printint16
    LDA1 0x0A
    PRNT
    RET
