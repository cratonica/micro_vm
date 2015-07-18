#ifndef CPU_HPP
#define CPU_HPP

#include <vector>
#include "Instructions.hpp"
#include "Memory.hpp"
#include "Registers.hpp"

class CPU
{
    public:
        class Fault;
    public:
        CPU(const std::vector<byte>& program);
        void Run();
        bool Tick();
        void Rewind();
        bool CanRewind() const
        {
            return _history.size() > 1;
        }
        void setOutputHandler(void (*handler)(byte))
        {
            _output = handler;
        }
        void setInputHandler(byte (*handler)())
        {
            _input = handler;
        }
        Inst::Enum CurrentInstruction() const
        {
            return _inst;
        }
        const Memory& Mem() const
        {
            return _mem;
        }
        const Registers& Regs() const
        {
            return _regs;
        }
    private:
        void push8(byte val);
        void push16(uint16_t val);
        byte pop8();
        uint16_t pop16();
        void takeSnapshot();
    private:
        Memory _mem;
        Registers _regs;
        std::stack<Registers> _history;
        Inst::Enum _inst;
        void (*_output)(byte);
        byte (*_input)();
    public:
        static const unsigned STACK_BOT;
};

#endif // CPU_HPP

