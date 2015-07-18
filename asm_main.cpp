#include <iostream>
#include "Assembler.hpp"

using namespace std;

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        cout << "Pass a file to assemble" << endl;
        return 2;
    }

    Assembler ass(argv[1]);
    const vector<char>& bin = ass.Bin();

    ofstream fout("out.bin");

    for (vector<char>::const_iterator it = bin.begin(); it != bin.end(); ++it)
    {
        fout << *it;
    }

    fout.close();

    ofstream dbg("out.dbg");

    const map<int, map<string, int> >::const_iterator end = ass.debugInfo().end();
    map<int, map<string, int> >::const_iterator it = ass.debugInfo().begin();

    for (; it != end; ++it)
    {
        const map<string, int>::const_iterator end2 = it->second.end();
        map<string, int>::const_iterator it2 = it->second.begin();
        for (; it2 != end2; ++it2)
        {
            dbg << it->first << ' ' << it2->first << ' ' << it2->second << '\n';
        }
    }

    dbg.close();

    return 0; 
}

