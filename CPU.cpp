#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "CPU.hpp"
#include "CPU_Fault.hpp"

using namespace std;

const unsigned CPU::STACK_BOT = 60000;

CPU::CPU(const vector<byte>& program)
    : _regs(STACK_BOT)
    , _inst(Inst::BAD)
    , _mem(0xFFFF)
    , _output(NULL)
    , _input(NULL)
{
    for (unsigned i = 0; i < program.size(); i++)
    {
        _mem.Write8(program.at(i), i);
    }
    takeSnapshot();
}

void CPU::Run()
{
    while (Tick())
        ;
}

bool CPU::Tick()
{
    _inst = Inst::Enum(_mem.Read8(_regs.IP++));
    switch (_inst)
    {
        case Inst::NOP:
            break;
        case Inst::ADD:
            {
                unsigned full = _regs.A + _mem.Read8(_regs.I) + _regs.Flags.Carry;
                _regs.A = (byte)(full);
                _regs.Flags.Carry = (full >> 8) & 1;
                _regs.Flags.Zero = _regs.A == 0;
            }
            break;
        case Inst::ADD1:
            {
                unsigned full = _regs.A + _mem.Read8(_regs.IP++) + _regs.Flags.Carry;
                _regs.A = (byte)(full);
                _regs.Flags.Carry = (full >> 8) & 1;
                _regs.Flags.Zero = _regs.A == 0;
            }
            break;
        case Inst::DIV:
            {
                unsigned mod = _regs.I % _regs.A;
                unsigned div = _regs.I / _regs.A;
                _regs.A = mod;
                _regs.I = div;
                _regs.Flags.Zero = _regs.A == 0;
            }
            break;
        case Inst::MUL:
            _regs.I = _regs.A * _regs.Y;
            break;
        case Inst::CMPL:
            _regs.A = ~_regs.A;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::CMP:
            _regs.Flags.Zero = _regs.A == _mem.Read8(_regs.I);
            _regs.Flags.Negative = _regs.A < _mem.Read8(_regs.I);
            break;
        case Inst::CMP1:
            _regs.Flags.Zero = (_regs.A == _mem.Read8(_regs.IP));
            _regs.Flags.Negative = (_regs.A < _mem.Read8(_regs.IP));
            ++_regs.IP;
            break;
        case Inst::LDI2:
            _regs.I = _mem.Read16(_regs.IP);
            _regs.IP += 2;
            break;
        case Inst::PSHA:
            push8(_regs.A);
            break;
        case Inst::PSHX:
            push8(_regs.X);
            break;
        case Inst::PSHY:
            push8(_regs.Y);
            break;
        case Inst::PSH1:
            push8(_mem.Read8(_regs.IP++));
            break;
        case Inst::PSH2:
            push16(_mem.Read16(_regs.IP));
            _regs.IP += 2;
            break;
        case Inst::LDA:
            _regs.A = _mem.Read8(_regs.I);
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::LDA1:
            _regs.A = _mem.Read8(_regs.IP++);
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::LDX1:
            _regs.X = _mem.Read8(_regs.IP++);
            break;
        case Inst::LDY1:
            _regs.Y = _mem.Read8(_regs.IP++);
            break;
        case Inst::LIS1:
            _regs.I = _mem.Read16(_regs.FP + _regs.SP + (signed char)(_mem.Read8(_regs.IP++)));
            break;
        case Inst::LAS1:
            _regs.A = _mem.Read8(_regs.FP + _regs.SP + (signed char)(_mem.Read8(_regs.IP++)));
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::DREF:
            _regs.I = _mem.Read16(_regs.I);
            break;
        case Inst::TXA:
            _regs.A = _regs.X;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::TYA:
            _regs.A = _regs.Y;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::TAX:
            _regs.X = _regs.A;
            break;
        case Inst::TAY:
            _regs.Y = _regs.A;
            break;
        case Inst::TSPI:
            _regs.I = _regs.FP + _regs.SP;
            break;
        case Inst::LAL1:
            _regs.A = _mem.Read8(_regs.FP + _mem.Read8(_regs.IP++));
            _regs.Flags.Negative = 0;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::LAP1:
            _regs.A = _mem.Read8(_regs.FP - 5 - _mem.Read8(_regs.IP++));
            _regs.Flags.Negative = 0;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::LIP1:
            _regs.I = _regs.FP - 5 - _mem.Read8(_regs.IP++);
            break;
        case Inst::LIL1:
            _regs.I = _regs.FP + _mem.Read8(_regs.IP++);
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::STL1:
            _mem.Write8(_regs.A, _regs.FP + _mem.Read8(_regs.IP++));
            break;
        case Inst::POP:
            --_regs.SP; 
            break;
        case Inst::POPA:
            _regs.A = pop8();
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::POPX:
            _regs.X = pop8();
            break;
        case Inst::POPY:
            _regs.Y = pop8();
            break;
        case Inst::POP1:
            _regs.SP -= _mem.Read8(_regs.IP++);
            break;
        case Inst::JMP2:
            _regs.IP = _mem.Read16(_regs.IP);
            break;
        case Inst::JLE2:
            if (_regs.Flags.Zero || _regs.Flags.Negative)
            {
                _regs.IP = _mem.Read16(_regs.IP);
            }
            else
            {
                _regs.IP += 2;
            }
            break;

        case Inst::JE2:
            if (_regs.Flags.Zero)
            {
                _regs.IP = _mem.Read16(_regs.IP);
            }
            else
            {
                _regs.IP += 2;
            }
            break;
        case Inst::JNE2:
            if (!_regs.Flags.Zero)
            {
                _regs.IP = _mem.Read16(_regs.IP);
            }
            else
            {
                _regs.IP += 2;
            }
            break;
        case Inst::JGE2:
            if (_regs.Flags.Zero || !_regs.Flags.Negative)
            {
                _regs.IP = _mem.Read16(_regs.IP);
            }
            else
            {
                _regs.IP += 2;
            }
            break;
        case Inst::JG2:
            if (_regs.Flags.Negative)
            {
                _regs.IP = _mem.Read16(_regs.IP);
            }
            else
            {
                _regs.IP += 2;
            }
            break;
        case Inst::JC2:
            if (_regs.Flags.Carry)
            {
                _regs.IP = _mem.Read16(_regs.IP);
            }
            else
            {
                _regs.IP += 2;
            }
            break;
        case Inst::CAL2:
            _mem.Write16(_regs.FP, _regs.FP + _regs.SP);
            _mem.Write8(_regs.SP, _regs.FP + _regs.SP + 2);
            _mem.Write16(_regs.IP + 2, _regs.FP + _regs.SP + 3);
            _regs.FP += _regs.SP + 5;
            _regs.SP = 0;
            _regs.IP = _mem.Read16(_regs.IP);
            break;
        case Inst::INCA:
            ++_regs.A;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::INCX:
            ++_regs.X;
            break;
        case Inst::INCY:
            ++_regs.Y;
            break;
        case Inst::INCI:
            ++_regs.I;
            break;
        case Inst::DECA:
            --_regs.A;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::DECX:
            --_regs.X;
            break;
        case Inst::DECY:
            --_regs.Y;
            break;
        case Inst::DECI:
            --_regs.I;
            break;
        case Inst::RET:
            if (_regs.FP == STACK_BOT)
            {
                return false;
            }
            _regs.IP = _mem.Read16(_regs.FP-2);
            _regs.SP = _mem.Read8(_regs.FP-3);
            _regs.FP = _mem.Read16(_regs.FP-5);
            break;
        case Inst::PRNT:
            if (_output)
            {
                _output(_regs.A);
            }
            else
            {
                cout << _regs.A;
            }
            break;
        case Inst::KEYB:
            if (_input)
            {
                _regs.A = _input();   
            }
            else
            {
                _regs.A = cin.get();
                if (_regs.A == '\n')
                {
                    _regs.A = 0;
                }
            }
            _regs.Flags.Negative = 0;
            _regs.Flags.Zero = _regs.A == 0;
            break;
        case Inst::CC:
            _regs.Flags.Carry = 0;
            break;
        case Inst::BRK:
            break;
        case Inst::BAD:
            throw Fault(*this, "Attempted to execute 'BAD' instruction");
        default:
            throw Fault(*this, "Invalid opcode");
            break;
    }
    takeSnapshot();
    return true;
}

void CPU::Rewind()
{
    if (CanRewind())
    {
        _history.pop(); // Pop current state
        _regs = _history.top();
        _mem.Rewind();
    }
}

void CPU::push8(byte val)
{
    _mem.Write8(val, _regs.FP + _regs.SP);
    ++_regs.SP;
}

void CPU::push16(uint16_t val)
{
    _mem.Write16(val, _regs.FP + _regs.SP);
    _regs.SP += 2;
}

byte CPU::pop8()
{
    --_regs.SP;
    return _mem.Read8(_regs.FP + _regs.SP);
}

uint16_t CPU::pop16()
{
    _regs.SP -= 2;
    uint16_t result = _mem.Read16(_regs.FP + _regs.SP);
    return result;
}

void CPU::takeSnapshot()
{
    _history.push(_regs); 
    _mem.TakeSnapshot();
}

