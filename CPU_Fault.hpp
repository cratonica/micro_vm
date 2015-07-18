#ifndef CPU_FAULT_HPP
#define CPU_FAULT_HPP

#include <exception>
#include "CPU.hpp"

class CPU::Fault : public std::exception
{
    public:
        Fault(const CPU& cpu, const std::string& msg);
        virtual ~Fault() throw ();
        virtual const char* what() const throw ();
    private:
       std::string _what;
};

#endif // CPU_FAULT_HPP

