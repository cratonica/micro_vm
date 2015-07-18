#include <fstream>
#include <iostream>
#include <vector>
#include "CPU.hpp"
#include "CPU_Fault.hpp"

using namespace std;

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        cout << "Pass a program to run" << endl;
        return 2;
    }

    ifstream fin(argv[1]);

    vector<byte> program;

    while (!fin.eof())
    {
        char c;
        fin.read(&c, 1);
        program.push_back(c);
    }

    fin.close();

    CPU cpu(program);

    /*
    try
    {
        cpu.Run();
    }
    catch (const CPU::Fault& fault)
    {
        cout << endl << fault.what() << endl;
        return 1;
    }
    */
    cpu.Run();
    return 0; 
}
