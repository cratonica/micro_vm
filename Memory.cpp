#include <cassert>
#include "Memory.hpp"

using namespace std;

Memory::Memory(unsigned size)
{
    _mem.resize(size);
}

byte Memory::Read8(unsigned offset) const
{
    return _mem.at(offset);
}

uint16_t Memory::Read16(unsigned offset) const
{
    return (0xFFFF & ((uint16_t)(_mem.at(offset)) << 8)) | _mem.at(offset+1);
}

void Memory::Write8(byte val, unsigned offset)
{
    _delta.push_back(PrevVal(offset, _mem.at(offset)));
    _mem.at(offset) = val;
}

void Memory::Write16(uint16_t val, unsigned offset)
{
    _delta.push_back(PrevVal(offset, _mem.at(offset)));
    _delta.push_back(PrevVal(offset+1, _mem.at(offset+1)));
    _mem.at(offset) = (byte)(val >> 8);
    _mem.at(offset+1) = (byte)(val);
}

void Memory::TakeSnapshot()
{
    _history.push(_delta);
    _delta.clear();
}

void Memory::Rewind()
{
    assert(_delta.empty()); // Shouldn't be in the middle of a state
    Delta& changes = _history.top(); 
    for (Delta::const_iterator it = changes.begin(); it != changes.end(); ++it)
    {
        _mem.at(it->Offset) = it->Value; 
    }
    _history.pop(); // Pop off current state
}

