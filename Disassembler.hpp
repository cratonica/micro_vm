#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

#include <vector>

class Disassembler
{
    public:
        Disassembler(const std::vector<char>& program);
        void Print() const;
    private:
        const std::vector<char>& _program;
};

#endif // DISASSEMBLER_HPP

