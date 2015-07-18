#!/usr/bin/python

# Registers (16-bit): 
    # A (8 bit) 
    # X = I[High] 
    # Y = I[Low] 
    # FP (16-bit) 
    # SP (8-bit offset from FP)
    # IP (16-bit) 
    # F(lags)
# Flags:
    # Zero
    # Carry
    # Sign

# SP points to next available element in stack (uninitialized)
#   Therefore, it should never be accessed
    

instructions = [
    # Operations
    'BAD',  # Invalid
    'NOP',  # Nothing
    'ADD',  # A = A + *X:Y
    'ADD1', # A = A + Literal
    'AND',  # A = A & *X:Y
    'AND1', # A = A & Literal
    'OR',   # A = A | *X:Y
    'ORL',  # A = A | Literal
    'CMPL', # A = ~A
    'INCA', # ++A
    'INCX', # ++X
    'INCY', # ++Y
    'INCI', # ++I
    'DECA', # --A
    'DECX', # --X
    'DECY', # --Y
    'DECI', # --I
    'DIV',  # I = I / A; A = I % A;
    'MUL',  # I = A * Y (8-bit operands, 16-bit result)

    # Compare
    'CMP',  # Flags = A Compared to *X:Y
    'CMP1', # Flags = A Compared to Literal

    # Memory
    'LDA',  # A = *X:Y
    'LDA1', # A = Literal
    'LDX1', # X = Literal
    'LDY1', # Y = Literal
    'DREF', # I = *I
    'LAS1', # A = SP[Literal]
    'LIS1', # I = SP[Literal]
    'STOR', # *I = A
    'TXA',  # A = X
    'TYA',  # A = Y
    'TAX',  # X = A
    'TAY',  # Y = A
    'TSPI', # I = FP + SP
    'LDI2', # I = Literal

    # Stack manipulation
    'PSHA', # *SP = A; ++SP
    'PSHX', # *SP = A; ++SP
    'PSHY', # *SP = A; ++SP
    'PSH1', # *SP = Lit8; ++SP
    'PSH2', # *SP = Lit16; SP += 2
    'POPA', # --SP; A = *(FP + SP);
    'POPX', # --SP; X = *(FP + SP);
    'POPY', # --SP; Y = *(FP + SP);
    'POP1', # SP -= Lit
    'POP',  # --SP
    'LAL1', # A = FP[Literal] (Load A from local)
    'LAP1', # A = FP[-5 - Literal] (Load A from parameter)
    'LIP1', # I = FP -5 - Literal (Load I from parameter address)
    'LIL1', # I = FP + Literal (Load I from local address)
    'STL1', # FP[Literal] = A

    # Flow Control
    'JMP2', # IP = Literal
    'JGE2', # If F[Zero] || !F[Sign], IP = Literal
    'JG2',  # If !F[Sign], IP = Literal
    'JL2',  # If F[Sign], IP = Literal
    'JLE2', # If F[Zero] || F[Sign], IP = Literal
    'JE2',  # If F[Zero], IP = Literal
    'JNE2', # If !F[Zero], IP = Literal
    'JC2',  # If F[Carry], IP = Literal
    'CAL2', # *(FP+SP)=FP; *(FP+SP+2)=SP; *(FP+SP+3)=IP; 
            # FP+=SP+5; SP = 0; IP = Literal
    'RET',  # IP = FP[-2]; SP = FP[-3]; FP = FP[-5];

    # I/O
    'PRNT', # Print A as ASCII byte
    'KEYB', # Read key into A. \0 is end of input

    # Flags
    'CC',   # Clear Carry

    # Debug
    'BRK'   # Debug break (NOP)
    ]

with open('Instructions.hpp', 'w') as f:
    print >> f, '#ifndef INSTRUCTIONS_HPP'
    print >> f, '#define INSTRUCTIONS_HPP'
    print >> f, '#define NUM_INSTRUCTIONS %d' % len(instructions)
    print >> f, 'namespace Inst { enum Enum'
    print >> f, '{'
    for inst in instructions[0:-1]:
        print >> f, '    %s,' % inst    
    print >> f, '    %s' % instructions[-1]
    print >> f, '};}'
    print >> f, ''
    print >> f, 'extern const char* InstStrings[NUM_INSTRUCTIONS];'
    print >> f, '#endif //INSTRUCTIONS_HPP'

with open('Instructions.cpp', 'w') as f:
    print >> f, '#include "Instructions.hpp"'
    print >> f, 'const char* InstStrings[NUM_INSTRUCTIONS] ='
    print >> f, '{'
    for inst in instructions[0:-1]:
        print >> f, '    "%s",' % inst
    print >> f, '    "%s"' % instructions[-1]
    print >> f, '};'
    print >> f, ''     

