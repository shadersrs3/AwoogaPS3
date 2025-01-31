#ifndef _CELLBE_POWERPROCESSOR_H
#define _CELLBE_POWERPROCESSOR_H

#include <string>
#include <limits>
#include <cmath>

#include <Common/Types.h>

// Forward declaration
struct Memory;

// Type punning...
union TypePunnedFloatInt32 {
    float F;
    uint32_t V;
};

union TypePunnedFloatInt64 {
    double F;
    uint64_t V;
};

typedef union _PPUVectorRegister {
    uint64_t data[2];

    void debug() {
        printf("%016llX %016llX\n", data[0], data[1]);
    }

    uint64_t getVPValue(int bitIndex) {
        uint64_t a = data[0], b = data[1];
        if (bitIndex < 64) {
            uint64_t c = a << bitIndex;
            uint64_t d = b >> (64 - bitIndex);
            return c;
        }

        if (bitIndex < 128) {
            bitIndex -= 64;
            uint64_t c = (b << bitIndex) >> bitIndex;
            return c;
        }
        return 0;
    }

    void setVPValue(int bitIndex, const uint8_t& value) {
        if (bitIndex < 60) {
            int index = 63 - bitIndex;
        }

        if (bitIndex < 64) {
            
        }

        if (bitIndex < 128) {
            
        }
    }
} PPUVectorRegister_t;

struct Thread {
    char name[64];
    uint32_t CR; // Condition register
    uint32_t FPSCR; // Floating point status and control register

    // Special purpose registers
    uint64_t XER; // Fixed point exception register
    uint64_t LR; // Link register
    uint64_t CTR; // Count register
    //
    uint64_t GPR[32]; // General purpose registers
    double FPR[32]; // Floating point registers
    PPUVectorRegister_t VR[32]; // Vector registers
    uint64_t PC, prevPC;
    uint64_t TBR; // Time base register
    int64_t delayDowncount;
};

struct PowerProcessor {
private:
    // Anonymize the unused fields afterwards
    union InstructionFormat {
        // simd
        // there's a little of VA forms
        struct {
            uint32_t xo : 6;
            uint32_t vc : 5;
            uint32_t vb : 5;
            uint32_t va : 5;
            uint32_t vd : 5;
        } va0;

        struct {
            uint32_t xo : 6;
            uint32_t sh : 4;
            uint32_t unused : 1;
            uint32_t vb : 5;
            uint32_t va : 5;
            uint32_t vd : 5;
        } va1;

        struct {
            uint32_t xo : 11;
            uint32_t vb : 5;
            uint32_t va : 5;
            uint32_t vd : 5;
        } va2;

        struct {
            uint32_t xo : 11;
            uint32_t vb : 5;
            uint32_t va : 5;
            uint32_t vd : 5;
        } vx;

        struct {
            uint32_t xo : 11;
            uint32_t vb : 5;
            uint32_t uimm : 2;
            uint32_t unused : 3;
            uint32_t vd : 5;
        } vx2;

        struct {
            uint32_t si : 16;
            uint32_t ra : 5;
            uint32_t rt : 5;
        } d;

        struct {
            uint32_t si : 16;
            uint32_t ra : 5;
            uint32_t l : 1;
            uint32_t unused : 1;
            uint32_t bf : 3;
        } d2;

        struct {
            uint32_t xo : 2;
            uint32_t ds : 14;
            uint32_t ra : 5;
            uint32_t rt : 5;
        } ds;

        struct {
            uint32_t lk : 1;
            uint32_t aa : 1;
            uint32_t li : 24;
        } i;

        struct {
            uint32_t rc : 1;
            uint32_t xo : 9;
            uint32_t oe : 1;
            uint32_t rb : 5;
            uint32_t ra : 5;
            uint32_t rt : 5;
        } xo;

        struct {
            uint32_t _unused : 1;
            uint32_t xo : 10;
            uint32_t _sprtbr : 10;
            uint32_t rt : 5;
        } xfx1;

        struct {
            uint32_t _unused : 1;
            uint32_t xo : 10;
            uint32_t _unused2 : 1;
            uint32_t fxm : 8;
            uint32_t op : 1;
            uint32_t rs : 5;
        } xfx2;

        struct {
            uint32_t rc : 1;
            uint32_t xo : 10;
            uint32_t rb : 5;
            uint32_t ra : 5;
            uint32_t rt : 5;
        } x;

        struct {
            uint32_t _unused : 1;
            uint32_t xo : 10;
            uint32_t rb : 5;
            uint32_t ra : 5;
            uint32_t l : 1;
            uint32_t _anotherunused : 1;
            uint32_t bf : 3;
        } x_2;

        struct {
            uint32_t rc : 1;
            uint32_t xo : 5;
            uint32_t frc : 5;
            uint32_t frb : 5;
            uint32_t fra : 5;
            uint32_t frt : 5;
        } x_3;

        struct {
            uint32_t rc : 1;
            uint32_t sh5 : 1;
            uint32_t op : 3;
            uint32_t mb : 6;
            uint32_t sh : 5;
            uint32_t ra : 5;
            uint32_t rs : 5;
        } md;

        struct {
            uint32_t lk : 1;
            uint32_t aa : 1;
            uint32_t bd : 14;
            uint32_t bi : 5;
            uint32_t bo : 5;
        } b;

        struct {
            uint32_t lk : 1;
            uint32_t xo : 10;
            uint32_t bh : 2;
            uint32_t unused : 3;
            uint32_t bi : 5;
            uint32_t bo : 5;
        } xl;

        struct {
            uint32_t rc : 1;
            uint32_t me : 5;
            uint32_t mb : 5;
            uint32_t sh : 5;
            uint32_t ra : 5;
            uint32_t rs : 5;
        } m;

        struct {
            uint32_t rc : 1;
            uint32_t xo : 5;
            uint32_t frc : 5;
            uint32_t frb : 5;
            uint32_t fra : 5;
            uint32_t frt : 5;
        } a;

        struct {
            uint32_t reserved : 1;
            uint32_t xo : 10;
            uint32_t tbr : 10;
            uint32_t rd : 5;
        } tbr;

        struct {
            uint32_t _unused1 : (32-6);
            uint32_t opcd : 6;
        };
        uint32_t instruction;
    };

    uint64_t clockCyclesElapsed;
    Thread *bank;
    Memory *memory;
private:
    template<typename T> static std::string toBinary(const T& v, int n) {
        std::string binary;
        for (int i = n; i-- > 0;)
            binary += (v & (1 << i)) != 0 ? "1" : "0";
        return binary;
    }

    inline int64_t signExtend64Format(int v, int bitToExtend) { 
        if ((v & (1 << bitToExtend)) != 0) {
            v = 0xFFFFFFFF & ~((1 << bitToExtend) - 1) | v;
        }
        return (int64_t)v;
    }

    inline void setOverflow(bool condition) {
        bank->XER |= (uint64_t)condition << 32;
        bank->XER = (bank->XER & ~(1uLL << 33)) | (uint64_t)condition << 33;
    }

    inline void setCarry(bool condition) {
        bank->XER = (bank->XER & ~(1uLL << 34)) | (uint64_t)condition << 34;
    }

    void setFI(double a, double b) {
        bank->FPSCR &= ~(1 << 14);
        if (a != b)
            bank->FPSCR |= 1 << 14;
    }

    void setFR(double a, double b) {
        bank->FPSCR &= ~(1 << 13);
        if (b > a)
            bank->FPSCR |= 1 << 13;
    }

    void __setFPRF(int condition) {
        bank->FPSCR = (bank->FPSCR & ~0xF8000) | condition << 15;
    }

    void setFPRF(double a) {
        int condition = 0;
        constexpr int QuietNaN = 0b10001;
        constexpr int NegativeInfinity = 0b01001;
        constexpr int NegativeNormalizedNumber = 0b01000;
        constexpr int NegativeDenormalizedNumber = 0b11000;
        constexpr int NegativeZero = 0b10010;
        constexpr int PositiveZero = 0b00010;
        constexpr int DenormalizedNumber = 0b10100;
        constexpr int NormalizedNumber = 0b00100;
        constexpr int Infinity = 0b00101;

        bank->FPSCR = bank->FPSCR & ~0xF8000;
        if (std::isnan(a)) {
            __setFPRF(QuietNaN);
            return;
        }

        if (a == std::numeric_limits<double>::infinity()) {
            __setFPRF(Infinity);
            return;
        }

        if (a == -std::numeric_limits<double>::infinity()) {
            __setFPRF(NegativeInfinity);
            return;
        }

        int x = std::fpclassify(a);

        if (x == FP_NORMAL) {
            __setFPRF(a < 0 ? NegativeNormalizedNumber : NormalizedNumber);
        } else if (x == FP_SUBNORMAL) {
            __setFPRF(a < 0 ? NegativeDenormalizedNumber : DenormalizedNumber);
        }

        if (a == 0.0 && std::signbit(a)) {
            __setFPRF(NegativeZero);
            return;
        } else if (a == 0.0) {        
            __setFPRF(PositiveZero);
            return;
        }
    }


    void setXFormatCR1(double result) {
        int bits = 0;
        if (result < 0)
            bits |= 1;
        if (result > 0)
            bits |= 2;
        if (result == 0)
            bits |= 4;

        bits |= ((bank->XER >> 32) & 1) << 3;
        bank->CR = (bank->CR & ~0xF) | bits;
    }

    void setXFormatCR1(uint64_t result) {
        int bits = 0;
        if ((int64_t) result < 0)
            bits |= 1;
        if ((int64_t) result > 0)
            bits |= 2;
        if (result == 0)
            bits |= 4;

        bits |= ((bank->XER >> 32) & 1) << 3;
        bank->CR = (bank->CR & ~0xF) | bits;
    }

    void setXFormatCR0(uint64_t result) {
        int bits = 0;
        if ((int64_t) result < 0)
            bits |= 1;
        if ((int64_t) result > 0)
            bits |= 2;
        if (result == 0)
            bits |= 4;

        bits |= ((bank->XER >> 32) & 1) << 3;
        bank->CR = (bank->CR & ~0xF) | bits;
    }
public:
    PowerProcessor();
    inline uint64_t& getTBR() { return bank->TBR; }
    inline Memory *getMemory() { return memory; }
    inline uint64_t readXER() { return bank->XER; }
    inline uint64_t readLR() { return bank->LR; }
    inline uint64_t readCTR() { return bank->CTR; }
    inline uint32_t readCR() { return bank->CR; }
    inline void setXER(uint64_t value) { bank->XER = value; }
    inline void setLR(uint64_t value) { bank->LR = value; }
    inline void setCTR(uint64_t value) { bank->CTR = value; }
    inline uint64_t& getPC() { return bank->PC; }
    inline uint64_t& getPrevPC() { return bank->prevPC; }
    inline void setCR(uint32_t value) {
        bank->CR = value;
    }
    
    void setCR(int index, int cond) {
        /*
        * This code generates it looks like a straight line diagonally when generated
        *  for (int i = 0; i <= 7; i++) {
        *      int index = i;
        *      uint32_t mask;
        *      if (index == 7)
        *          mask = 0xF0000000;
        *      else
        *          mask = ((1 << ((index + 1) << 2)) - 1) & ~((1 << (index << 2)) - 1);
        *  }
        */

        static constexpr uint32_t controlRegisterMaskTable[8] = {
            0xfffffff0,
            0xffffff0f,
            0xfffff0ff,
            0xffff0fff,
            0xfff0ffff,
            0xff0fffff,
            0xf0ffffff,
            0x0fffffff,
        };

        bank->CR = (bank->CR & controlRegisterMaskTable[index]) | (cond & 0xF) << (index << 2);
    }

    inline uint64_t readGPR(int index) { return bank->GPR[index]; }
    inline void setGPR(int index, uint64_t value) { bank->GPR[index] = value; }
    inline void setVR(int index, const PPUVectorRegister_t& vr) { bank->VR[index] = vr; }
    inline PPUVectorRegister_t& getVR(int index) { return bank->VR[index]; }
    inline void setFPR(int index, double value) { bank->FPR[index] = value; }
    double readFPR(int index) { return bank->FPR[index]; }

    void dumpRegisters() {
        printf("PC: %08llX", bank->PC);
        printf("\nGPR\n");
        for (int i = 0; i < 32; i++)
            printf("r%d: %016llX\n", i, readGPR(i));
        printf("LR: %llX\n", readLR());
        printf("XER: %llX\n", readXER());
        printf("CTR: %llX\n", readCTR());
    }
    inline Thread *getThread() { return bank; }
    void setThread(Thread *t) { bank = t; }
    void reset();
    void setMemory(Memory *memory);
    uint64_t executeInterpreterSingleStep(uint64_t address);
};


#endif