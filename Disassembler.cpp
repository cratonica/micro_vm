#include <iomanip>
#include <iostream>
#include "Disassembler.hpp"
#include "Instructions.hpp"

using namespace std;

Disassembler::Disassembler(const vector<char>& program)
    : _program(program)
{

}

void Disassembler::Print() const
{
    for (vector<char>::const_iterator it = _program.begin(); it != _program.end(); ++it)
    {
        cout << hex << setfill('0') << setw(4) << uppercase 
             << distance(_program.begin(), it) << ":\t";
        if (*it < NUM_INSTRUCTIONS)
        {
            string inst = InstStrings[*it];
            cout << inst;
            char suffix = inst[inst.size()-1];
            if (suffix == '1')
            {
                cout << "\t0x" << hex << setfill('0') << setw(2) 
                     << unsigned(*(++it));
            }
            else if (suffix == '2')
            {
                cout << "\t0x" << hex << setw(4) << setfill('0')
                     << ((unsigned(*(++it) << 8) | unsigned(*(++it))));
            }

        }            
        else
        {
            cout << "0x" << hex << unsigned(*it);
            /*
            cout << "\t" << dec << unsigned(*it);

            if (isgraph(*it))
            {
                 cout << "\t" << *it;
            }*/
        }
        cout << '\n';
    }
}

