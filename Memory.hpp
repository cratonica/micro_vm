#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <stack>
#include <vector>
#include "Primitives.hpp"

class Memory
{
    public: // Methods
        Memory(unsigned size);
        byte Read8(unsigned offset) const;
        uint16_t Read16(unsigned offset) const;
        void Write8(byte, unsigned offset);
        void Write16(uint16_t, unsigned offset);
        void TakeSnapshot();
        void Rewind();
    private: // Types
        typedef std::vector<byte> Block;
        struct PrevVal
        {
            unsigned Offset;
            byte Value;
            PrevVal(unsigned offset, byte value)
                : Offset(offset)
                , Value(value) {}
        };
        typedef std::vector<PrevVal> Delta;
        typedef std::stack<Delta> History;
    private: // Fields
        Block _mem;
        History _history;
        Delta _delta;
};

#endif // MEMORY_HPP

