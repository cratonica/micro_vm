#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "CPU_Fault.hpp"
#include "Instructions.hpp"

using namespace std;

CPU::Fault::Fault(const CPU& cpu, const string& msg)
{
    stringstream ss;                 
    ss << "CPU Fault: " << msg << '\n';
    ss << "\tOpcode: ";
    if (cpu._inst < NUM_INSTRUCTIONS)
    {
        ss << InstStrings[cpu._inst] << '\n';
    }
    else
    {
        ss << "0x" << hex << unsigned(cpu._inst) << " (Unknown)\n";
    }
    ss << "\tA:\t" << "0x" << hex << unsigned(cpu._regs.A) << " (" << dec << unsigned(cpu._regs.A) << ")\n";
    ss << "\tI:\t" << "0x" << hex << unsigned(cpu._regs.I) << " (" << dec << unsigned(cpu._regs.I) << ")\n";
    ss << "\tX:\t" << "0x" << hex << unsigned(cpu._regs.X) << " (" << dec << unsigned(cpu._regs.X) << ")\n";
    ss << "\tY:\t" << "0x" << hex << unsigned(cpu._regs.Y) << " (" << dec << unsigned(cpu._regs.Y) << ")\n";
    ss << "\tFP:\t" << "0x" << hex << unsigned(cpu._regs.FP) << '\n';
    ss << "\tSP:\t" << "0x" << hex << unsigned(cpu._regs.SP) << '\n';
    ss << "\tIP*:\t" << "0x" << hex << unsigned(cpu._regs.IP) << " (";
    if (cpu._mem.Read8(cpu._regs.IP) < NUM_INSTRUCTIONS)
    {
        ss << InstStrings[cpu._mem.Read8(cpu._regs.IP)] << ")\n";
    }
    else
    {
        ss << "Bad Opcode)\n";
    }
    _what = ss.str();
}

CPU::Fault::~Fault() throw ()
{

}

const char* CPU::Fault::what() const throw ()
{
    return _what.c_str();
}


