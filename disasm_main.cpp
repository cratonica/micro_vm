#include <iostream>
#include <fstream>
#include "Disassembler.hpp"

using namespace std;

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        cout << "Pass a program to disassemble" << endl;
        return 2;
    }

    ifstream fin(argv[1]);

    vector<char> program;

    while (!fin.eof())
    {
        char c;
        fin.read(reinterpret_cast<char*>(&c), 1);
        if (!fin.eof())
        {
            program.push_back(c);
        }
    }

    fin.close();

    Disassembler disasm(program);
    disasm.Print();

    return 0; 
}

