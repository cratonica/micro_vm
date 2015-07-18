build = Environment(
                    CXXFLAGS = '-g'
                   )

# Create instructions definitions
build.Command(['Instructions.hpp', 'Instructions.cpp'], 'make_instructions.py', 'python make_instructions.py')

# Create CPU emulator
build.Program('cpu', ['Memory.cpp', 'CPU.cpp', 'CPU_Fault.cpp', 'cpu_main.cpp', 'Instructions.cpp', 'Instructions.hpp'])

# Create debugger
build.Program('debug', ['debug_main.cpp', 'Memory', 'CPU', 'CPU_Fault', 'Instructions', 'Instructions.hpp'], LIBS='ncurses')

# Create assembler
build.Program('asm', ['Instructions.cpp', 'Instructions.hpp', 'Assembler.cpp', 'asm_main.cpp'])

# Create disassembler
build.Program('disasm', ['Instructions.cpp', 'Instructions.hpp', 'Disassembler.cpp', 'disasm_main.cpp'])

# Assemble the test program
build.Command(['out.bin', 'out.dbg'], ['asm', 'test.asm', 'lib.asm'], './asm test.asm')
