JMP2 main # Allow us to include this at the top of the file

endl:
    D1 0x0A 0

puts:
    LIP1 2
    DREF
.loop:
    LDA
    JE2  .done
    PRNT
    INCI
    JMP2 .loop
.done:
    RET

printint16:
    LIP1 2 
    DREF
    PSHX    # 0-1: Unpushed number
    PSHY
    PSH1 0  # 2: Number of digits to print
            # 3-: Digits to print
.loop:
    LIL1 0  # Remaining to process
    DREF    # Dereference I pointer
    LDA1 10 # Divisor
    DIV
    PSHA    # Remainder = digit to print
    TXA
    STL1 0
    TYA
    STL1 1
    LAL1 2
    INCA
    STL1 2
    LAL1 0
    JNE2 .loop # First byte zero?
    LAL1 1
    JNE2 .loop # second byte zero?
.print:
    # Change: Parameter 0 is now total printed
    LAL1 2
    JE2 .done
    DECA
    STL1 2
    POPA
    JE2 .iszero
.doprint:
    ADD1 '0'
    PRNT
    LAL1 0
    INCA
    STL1 0
    JMP2 .print
.iszero:
    TAX
    LAL1 0
    JE2  .print # Don't print leading zeros
    TXA
    JMP2 .doprint
.done:
    LAL1 0
    JE2 .doprint # No digits printed. Print a zero (already now in A).
    RET


printint8: # Written without use of FP
    CC
    LAP1 1
    PSH1 0 # Digit count
    PSHA
.loop:
    LDX1 0
    POPY # Remainder
    LDA1 10
    DIV
    POPX # Pull digit count
    PSHA # Digit itself
    TXA  # Digit count
    INCA # +1
    PSHA # Digit count on top
    TYA
    JE2   .done
    PSHA # Remainder
    JMP2  .loop
.done:
    POPX # X = Digit Count
.printloop:
    TXA
    JE2 .printdone
    DECA
    TAX
    POPA
    ADD1 '0'
    PRNT
    JMP2 .printloop
.printdone:
    RET
    TAY

digin:
    LDA1 '0'
    CMPL # Step 1
    INCA # Step 2 = 2s complement
    TSPI # I = future position of '0'
    PSHA
    KEYB
    JE2  .gotnull
    CC
    ADD    
    POP
    RET
.gotnull:
    POP
    LDA1 0xFF
    RET

int8in:
    PSH1 0
    LDX1 0
.loop:
    CAL2 digin
    CMP1 0xFF
    JE2  .done
    POPY
    PSHA
    LDA1 10 
    MUL
    TYA
    TSPI
    DECI
    CC
    ADD
    POP
    PSHA
    JMP2 .loop
.done:
    POPA
    RET

# Uses sum of consecutive odd numbers rule
sqrt8:
    CC
    LAP1 1
    JE2  .done
    PSH1 0 # 0: result
    TSPI   # Sum pointed to by I
    PSH1 1 # 1: sum
    PSH1 1 # 2: current odd number
    PSHA   # 3: Argument
.loop:
    LAL1 0
    INCA 
    STL1 0
    LAL1 2
    ADD1 2
    STL1 2
    LIL1 1
    ADD  # Sum += odd number
    JC2 .done # Overflow
    STL1 1
    TSPI
    DECI
    CMP
    JLE2 .loop
.done:
    LAL1 0
    RET
    
