#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <list>
#include <sstream>
#include <vector>
#include <ncurses.h>
#include <unistd.h>
#include "CPU.hpp"
#include "CPU_Fault.hpp"

using namespace std;

// TODO: Ability to reload program mid-execution to allow for JIT
// debugging. This will likely crash the program at some point
// due to return addresses changing, but will be useful for
// intra-function debugging

// TODO: Support for me to be editing asm file in vim, and have a script
// that assembles it and launches the debugger so Regs().I can see the program state
// up to the end of the program (or HALT or BRK instruction)


// I have no idea why this isn't just a class.

static map<int, pair<string, int> > dbgMap;
static map<string, std::vector<string> > srcFiles;
static list<string> exec;
static WINDOW* lblWin;
static WINDOW* regWin;
static WINDOW* execWin;
static WINDOW* codeWin;
static WINDOW* stackWin;
static WINDOW* ioWin;
static bool hexDisplay;

static string outputStr;

static void setupWindows();
static void cleanupWindows();
static void refreshWindows();
static void clearWindow(WINDOW*);
static void updateExec(CPU&);
static void updateCode(CPU&);
static void updateStack(CPU&);
static void updateRegs(CPU&);
static void updateOutput();
static void handleOutput(byte);
static byte handleInput();

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        cout << "Pass a program to run" << endl;
        return 2;
    }
    int delayMs = -1;

    for (int i = 1; i < argc; i++)
    {
        string arg(argv[i]);
        if (arg == "--delay" && argc > i)
        {
            stringstream ss;
            ss << argv[i+1];
            ss >> delayMs;
            ++i;
        }
        else if (arg == "--help" || arg == "-h")
        {
            cout << "Usage: debug program_without_extension [--delay MS]"
                 << endl;
            return 0;
        }
        else if (i > 1)
        {
            cout << "Unknown argument: " << argv[i] << endl;
            return 2;
        }
    }

    string binName = string(argv[1]) + ".bin";
    string dbgName = string(argv[1]) + ".dbg";

    ifstream bin(binName.c_str());

    if (!bin.is_open())
    {
        cout << "Failure opening file " << binName << endl;
        return 1;
    }

    vector<byte> program;

    while (!bin.eof())
    {
        char c;
        bin.read(reinterpret_cast<char*>(&c), 1);
        program.push_back(c);
    }

    bin.close();

    ifstream dbg(dbgName.c_str());

    if (!dbg.is_open())
    {
        cout << "Failure opening file " << dbgName << endl;
        return 1;
    }

    int addr;
    dbg >> addr;
    while (!dbg.eof())
    {
        // addr being set at bottom of loop
        // so we can trip EOF
        string srcName;
        int line;
        dbg >> srcName;
        dbg >> line;

        if (srcFiles.find(srcName) == srcFiles.end())
        {
            vector<string> lines;
            ifstream src(srcName.c_str());        
            string line;
            while (getline(src, line))
            {
                lines.push_back(line);
            }
            src.close();
            srcFiles[srcName] = lines;
        }
        dbgMap[addr] = make_pair(srcName, line);
        dbg >> addr;
    }

    dbg.close();

    CPU cpu(program);

    cpu.setOutputHandler(&handleOutput);
    cpu.setInputHandler(&handleInput);

    setupWindows();
    try {
        do
        {
top:            
            updateExec(cpu);
            updateCode(cpu);
            updateRegs(cpu);
            updateStack(cpu);
            updateOutput();
            refreshWindows();

            if (cpu.CurrentInstruction() == Inst::BRK)
            {
                getch();
            }

            if (delayMs == -1)
            {
tryagain:
                switch(getch())
                {
                    case 'r':
                        if (cpu.CanRewind())
                        {
                            cpu.Rewind();
                            exec.pop_front();
                            clearWindow(execWin);
                        }
                        exec.pop_front(); 
                        goto top;
                    case 'x':
                        hexDisplay = !hexDisplay;
                        updateRegs(cpu);
                        refreshWindows();
                        goto tryagain;
                    case '\n':
                        break;
                    default:
                        goto tryagain;
                }
            }
            else
            {
                usleep(delayMs * 1000);
            }

        } while (cpu.Tick());
    }
    catch (const CPU::Fault& e)
    {
        mvwprintw(ioWin, 1, 1, "%s", e.what());
        refreshWindows();
        getch();
    }



    cleanupWindows();

    return 0;
}

static void clearWindow(WINDOW* window)
{
    wclear(window);
    wborder(window, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void setupWindows()
{
    initscr();
    noecho();

    // Shouldn't ever have to refresh this window
    lblWin = newwin(12, 4, 6, 0);
    box(lblWin, 0, 0);
    wborder(lblWin, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(lblWin, 1, 1, "%s", "A");
    mvwprintw(lblWin, 2, 1, "%s", "X");
    mvwprintw(lblWin, 3, 1, "%s", "Y");
    mvwprintw(lblWin, 4, 1, "%s", "I");
    wattron(lblWin, A_REVERSE);
    mvwprintw(lblWin, 5, 1, "%s", "FP");
    wattroff(lblWin, A_REVERSE);
    mvwprintw(lblWin, 6, 1, "%s", "SP");
    mvwprintw(lblWin, 7, 1, "%s", "IP");
    mvwprintw(lblWin, 8, 1, "%s", "Z");
    mvwprintw(lblWin, 9, 1, "%s", "C");
    mvwprintw(lblWin, 10, 1, "%s", "N");
    refresh();
    wrefresh(lblWin);

    regWin = newwin(12, 8, 6, 4);
    box(regWin, 0, 0);
    wborder(regWin, 0, 0, 0, 0, 0, 0, 0, 0);

    ioWin = newwin(6, 0, 0, 0);
    box(ioWin, 0, 0);
    wborder(ioWin, 0, 0, 0, 0, 0, 0, 0, 0);

    // 2 modes we can toggle: Instruction Exec and Context
    int maxx, maxy;
    getmaxyx(stdscr, maxy, maxx);
    codeWin = newwin(0, (maxx-12) / 2, 6, 12);
    box(codeWin, 0, 0);
    wborder(codeWin, 0, 0, 0, 0, 0, 0, 0, 0);

    execWin = newwin(0, 0, 6, 12 + (maxx-12) / 2);
    box(execWin, 0, 0);
    wborder(execWin, 0, 0, 0, 0, 0, 0, 0, 0);

    stackWin = newwin(0, 12, 18, 0);
    box(stackWin, 0, 0);
    wborder(stackWin, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cleanupWindows()
{
    delwin(execWin);
    delwin(codeWin);
    delwin(regWin);
    delwin(lblWin);
    delwin(ioWin);
    delwin(stackWin);
    endwin();
}

static void refreshWindows()
{
    refresh();
    wrefresh(execWin);
    wrefresh(ioWin);
    wrefresh(regWin);
    wrefresh(stackWin);
    wrefresh(codeWin);
}

static void updateCode(CPU& cpu)
{
    map<int, pair<string, int> >::const_iterator it = 
        dbgMap.upper_bound(cpu.Regs().IP);
    --it; // Got upper bound, need one below

    int maxx, maxy;
    getmaxyx(execWin, maxy, maxx);
    const string& fileName = it->second.first;
    const std::vector<string>& file = srcFiles.find(fileName)->second;
    int lineIndex = it->second.second-1;

    char fmt[32];
    sprintf(fmt, "%%-%ds", maxx - 2);
    wattron(codeWin, A_REVERSE);
    mvwprintw(codeWin, 1, 1, fmt, ""); // Blank out line
    mvwprintw(codeWin, 1, maxx / 2 - fileName.size() / 2, "%s", fileName.c_str());
    wattroff(codeWin, A_REVERSE);

    sprintf(fmt, "%%-%ds", maxx - 2);
    for (int row = 2; row <= maxy - 2; ++row)
    {
        int curLine = lineIndex - (maxy-2)/2 + row - 1;
        string line = curLine < 0 || curLine >= file.size() ? "" : file[curLine];
        if (line.size() > maxx - 2)
        {
            line[maxx-2] = '\0';
        }
        if (curLine == lineIndex)
        {
            wattron(codeWin, A_BLINK);
        }
        mvwprintw(codeWin, row, 1, fmt, line.c_str());
        if (curLine == lineIndex)
        {
            wattroff(codeWin, A_BLINK);
        }
    }
}

static void updateExec(CPU& cpu)
{
    // TODO: Context mode support
    map<int, pair<string, int> >::const_iterator it = 
        dbgMap.upper_bound(cpu.Regs().IP);
    --it; // Got upper bound, need one below

    int maxx, maxy;
    getmaxyx(execWin, maxy, maxx);
    string line = srcFiles[it->second.first][it->second.second-1];
    exec.push_front(line);

    char fmt[32];
    sprintf(fmt, "%%-%ds", maxx - 2);
    wattron(execWin, A_REVERSE);
    mvwprintw(execWin, 1, 1, fmt, ""); // Blank out line
    const char* title = "Execution Path";
    mvwprintw(execWin, 1, maxx / 2 - strlen(title) / 2, "%s", title);
    wattroff(execWin, A_REVERSE);

    sprintf(fmt, "%%-%ds", maxx - 2);
    list<string>::const_iterator listIt = exec.begin();
    const list<string>::const_iterator listEnd = exec.end();
    for (int row = maxy - 2; row > 2 && listIt != listEnd; ++listIt, --row)
    {
        const char* start = listIt->c_str();
        while (isspace(*start))
        {
            ++start;
        }
        line = start;
        if (line.size() > maxx - 2)
        {
            line[maxx-2] = '\0';
        }
        mvwprintw(execWin, row, 1, fmt, line.c_str());
    }

}

static void updateStack(CPU& cpu)
{
    int maxx, maxy;
    getmaxyx(stackWin, maxy, maxx);
    char fmt[32];
    sprintf(fmt, "%%-%ds", maxx - 2);
    wattron(stackWin, A_REVERSE);
    mvwprintw(stackWin, 1, 1, fmt, ""); // Blank out line
    const char* title = "Stack";
    mvwprintw(stackWin, 1, maxx / 2 - strlen(title) / 2, "%s", title);
    wattroff(stackWin, A_REVERSE);
    int addr = cpu.Regs().FP + cpu.Regs().SP - 1;
    int row = 2;
    for (row = 2; row <= maxy - 2; ++row, --addr)
    {
        char val = cpu.Mem().Read8(addr);
        if (row == maxy - 2 && addr >= cpu.Regs().FP)
        {
            mvwprintw(stackWin, row, 1, "(%3d more)", addr - CPU::STACK_BOT + 1);
        }
        else if (addr >= cpu.Regs().FP)
        {
            mvwprintw(stackWin, row, 1, "0x%02X   % 3u", val & 0xFF, val & 0xFF);
        }
        else
        {
            mvwprintw(stackWin, row, 1, "%s", "          ");
        }   

    }
}

static void updateRegs(CPU& cpu)
{
    if (hexDisplay)
    {
        mvwprintw(regWin, 1, 1, "0x%02X", cpu.Regs().A & 0xFF);
        mvwprintw(regWin, 2, 1, "0x%02X", cpu.Regs().X & 0xFF);
        mvwprintw(regWin, 3, 1, "0x%02X", cpu.Regs().Y & 0xFF);
        mvwprintw(regWin, 4, 1, "0x%04hX", cpu.Regs().I);
        mvwprintw(regWin, 5, 1, "0x%04hX", cpu.Regs().FP);
        mvwprintw(regWin, 6, 1, "0x%02X", cpu.Regs().SP & 0xFF);
        mvwprintw(regWin, 7, 1, "0x%04hX", cpu.Regs().IP);
        mvwprintw(regWin, 8, 1, "%u", cpu.Regs().Flags.Zero);
        mvwprintw(regWin, 9, 1, "%u", cpu.Regs().Flags.Carry);
        mvwprintw(regWin, 10, 1, "%u", cpu.Regs().Flags.Negative);
    }
    else
    {
        mvwprintw(regWin, 1, 1, " % 3u", cpu.Regs().A & 0xFF);
        mvwprintw(regWin, 2, 1, " % 3u", cpu.Regs().X & 0xFF);
        mvwprintw(regWin, 3, 1, " % 3u", cpu.Regs().Y & 0xFF);
        mvwprintw(regWin, 4, 1, " %05hu", cpu.Regs().I);
        mvwprintw(regWin, 5, 1, " %05hu", cpu.Regs().FP);
        mvwprintw(regWin, 6, 1, " % 3u", cpu.Regs().SP & 0xFF);
        mvwprintw(regWin, 7, 1, " %05hu", cpu.Regs().IP);
        mvwprintw(regWin, 8, 1, "%u", cpu.Regs().Flags.Zero);
        mvwprintw(regWin, 9, 1, "%u", cpu.Regs().Flags.Carry);
        mvwprintw(regWin, 10, 1, "%u", cpu.Regs().Flags.Negative);
    }
}

static void updateOutput()
{
    mvwprintw(ioWin, 1, 1, "%s", outputStr.c_str());
}

static void handleOutput(byte c)
{
    outputStr += c;
}

static byte handleInput()
{
    static list<byte> inbuf;

    if (inbuf.empty())
    {
        // TODO Prompt for input
        mvwprintw(ioWin, 1, 1, "%s", "Program requesting input: ");
        refreshWindows();
        string in;
        cin >> in;
        copy(in.begin(), in.end(), back_inserter(inbuf));
        inbuf.push_back(0); // End of input
    }

    byte result = inbuf.front();
    inbuf.pop_front();
    return result;
}


