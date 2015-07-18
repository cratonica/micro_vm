#ifndef REGISTERS_HPP
#define REGISTERS_HPP


class Registers
{
    public:
        Registers(unsigned sp)
            : A(0)
            , I(0)
            , FP(sp)
            , SP(0)
            , IP(0) {}
        byte A;
        union
        {
            unsigned short I;
            struct
            {
                // Little-endian
                byte Y;
                byte X;
            };
        };
        unsigned short FP;
        byte  SP;
        unsigned short IP;
        struct Flags
        {
            Flags()
                : Zero(0)
                , Carry(0) 
                , Negative(0)
            {

            }
            
            unsigned Zero : 1;
            unsigned Carry : 1;
            unsigned Negative : 1;
        } Flags;
};

#endif // REGISTERS_HPP

