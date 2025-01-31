#include <bit>

#include <Cell/PowerProcessor.h>
#include <Memory/Memory.h>
#include <HLE/HLE.h>

#include <Common/Logger.h>

// FIX THE FLOATING POINT OPERATIONS SOON

#define LOAD_128_BE(dest, vD) \
        dest[0].V = vD.data[0] >> 32; \
        dest[1].V = (uint32_t)vD.data[0]; \
        dest[2].V = vD.data[1] >> 32; \
        dest[3].V = (uint32_t)vD.data[1]

#define STORE_128_BE(dest, vD)  \
        vD.data[0] = (uint64_t)dest[0].V << 32 | dest[1].V; \
        vD.data[1] = (uint64_t)dest[2].V << 32 | dest[3].V

namespace std { void e(int) { } }

#define LOG_ERROR(...)
#define exit(n) e(n); return ~0uLL;
/*
    Using this documentation
    PowerPC User Instruction Set Architecture
    Book I
    Version 2.02
    January 28, 2005
    and the SIMD instruction set
*/

PowerProcessor::PowerProcessor() {
    reset();
}

void PowerProcessor::reset() {
    memory = nullptr;
}

void PowerProcessor::setMemory(Memory *memory) {
    this->memory = memory;
}

#define MASK32_ALLSET 0xFFFFFFFF
#define MASK64_ALLSET 0xFFFFFFFFFFFFFFFF

// https://github.com/Goatman13/pypyc2c/blob/main/ppc2c.py
// Generates a 32-bit mask between MaskBegin (MB) and MaskEnd (ME) inclusive
uint32_t GenerateMask32(int MB, int ME) {
    if (MB < 0 || ME < 0 || MB > 31 || ME > 31) {
        printf("Error with parameters GenerateMask32(%d, %d)\n", MB, ME);
        return 0;
    }

    uint32_t mask = 0;
    if (MB <= ME) {
        // Normal mask
        while (MB <= ME) {
            mask |= (1U << (31 - MB));
            MB++;
        }
    } else if (MB == ME + 1) {
        // All mask bits set
        mask = MASK32_ALLSET;
    } else {
        // Split mask
        uint32_t mask_lo = GenerateMask32(0, ME);
        uint32_t mask_hi = GenerateMask32(MB, 31);
        mask = mask_lo | mask_hi;
    }

    return mask;
}

// Generates a 64-bit mask between MaskBegin (MB) and MaskEnd (ME) inclusive
uint64_t GenerateMask64(int MB, int ME) {
    if (MB < 0 || ME < 0 || MB > 63 || ME > 63) {
        printf("Error with parameters GenerateMask64(%d, %d)\n", MB, ME);
        return 0;
    }

    uint64_t mask = 0;
    if (MB <= ME) {
        // Normal mask
        while (MB <= ME) {
            mask |= (1ULL << (63 - MB));
            MB++;
        }
    } else if (MB == ME + 1) {
        // All mask bits set
        mask = MASK64_ALLSET;
    } else {
        // Split mask
        uint64_t mask_lo = GenerateMask64(0, ME);
        uint64_t mask_hi = GenerateMask64(MB, 63);
        mask = mask_lo | mask_hi;
    }

    return mask;
}

typedef struct {
    uint64_t low;
    int64_t high;
} int128_t;

int128_t mul64to128(int64_t a, int64_t b) {
    int128_t result;

    // Extract absolute values and track sign
    uint64_t ua = (a < 0) ? -a : a;
    uint64_t ub = (b < 0) ? -b : b;

    // Split into 32-bit parts
    uint64_t aL = ua & 0xFFFFFFFF;
    uint64_t aH = ua >> 32;
    uint64_t bL = ub & 0xFFFFFFFF;
    uint64_t bH = ub >> 32;

    // Perform partial products
    uint64_t albl = aL * bL;
    uint64_t albh = aL * bH;
    uint64_t ahbl = aH * bL;
    uint64_t ahbh = aH * bH;

    // Combine results
    uint64_t low = albl + ((albh & 0xFFFFFFFF) << 32) + ((ahbl & 0xFFFFFFFF) << 32);
    int64_t high = (ahbh + (albh >> 32) + (ahbl >> 32) + (low < albl));

    // Apply sign correction
    if ((a < 0) ^ (b < 0)) {
        high = ~high + (low == 0); // Two’s complement negate
        low = -low;
    }

    result.low = low;
    result.high = high;
    return result;
}

uint64_t PowerProcessor::executeInterpreterSingleStep(uint64_t address) {
    InstructionFormat i;

    address &= ~3;
    if (auto func = __getRegisteredPPUFunction(address)) {
        if (!func->exec) {
            dumpRegisters();
            printf("Unknown module call (%s) call 0x%08llX NID 0x%08X prev PC %08llX thread %s", func->name, address, func->nid, bank->PC, bank->name);
            std::exit(0);
        }

        func->exec(this);
        return readLR();
    }

    if (!memory->read32((address), &i.instruction)) { // Always word aligned 2 bits ignored and zero
        LOG_ERROR("Unknown read address 0x%08llX prev 0x%08llX thread %s", address, bank->prevPC, bank->name);
        std::exit(0);
        return readLR();
    }

    bank->prevPC = address;
    bank->PC = address;
    switch (i.opcd) {
    case 4: // vector instructions
        switch (i.va0.xo) {
        case 46: // vmaddfp
        {
            auto& vD = getVR(i.va0.vd);
            auto& vA = getVR(i.va0.va);
            auto& vC = getVR(i.va0.vc);
            auto& vB = getVR(i.va0.vb);

            TypePunnedFloatInt32 dest[4], a[4], b[4], c[4];

            LOAD_128_BE(a, vA);
            LOAD_128_BE(b, vB);
            LOAD_128_BE(c, vC);

            for (int i = 0; i < 4; i++)
                dest[i].F = std::round(a[i].F * c[i].F + b[i].F);

            STORE_128_BE(dest, vD);
            return address + 4;
        }
        case 42: // vsel
        {
            auto& vD = getVR(i.va0.vd);
            auto& vA = getVR(i.va0.va);
            auto& vC = getVR(i.va0.vc);
            auto& vB = getVR(i.va0.vb);

            TypePunnedFloatInt32 dest[4], a[4], b[4], c[4];

            LOAD_128_BE(a, vA);
            LOAD_128_BE(b, vB);
            LOAD_128_BE(c, vC);

            dest[0] = dest[1] = dest[2] = dest[3] = TypePunnedFloatInt32 {};
            for (int i = 0; i < 4; i++) {
                const auto& _a = a[i];
                const auto& _b = b[i];
                const auto& _c = c[i];
                uint32_t data = 0;
                for (int j = 32; j-- > 0; ) {
                    if ((_c.V & (1 << j)) != 0)
                        data |= (_a.V & (1 << j));
                    else
                        data |= (_b.V & (1 << j));
                }

                dest[i].V = data;
            }

            STORE_128_BE(dest, vD);
            return address + 4;
        }
        case 47: // vnmsubfp
        {
            auto& vD = getVR(i.va0.vd);
            auto& vA = getVR(i.va0.va);
            auto& vC = getVR(i.va0.vc);
            auto& vB = getVR(i.va0.vb);

            TypePunnedFloatInt32 dest[4], a[4], b[4], c[4];

            LOAD_128_BE(a, vA);
            LOAD_128_BE(b, vB);
            LOAD_128_BE(c, vC);

            for (int i = 0; i < 4; i++)
                dest[i].F = std::round(a[i].F * c[i].F - b[i].F);

            STORE_128_BE(dest, vD);
            return address + 4;
        }
        case 32:
        case 33:
        case 34:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 43:
        case 44:
            LOG_ERROR("UNKNOWN VECTOR 4 LOWER OPCODE %d PC %llX", i.va0.xo, address);
            std::exit(0);
            break;
        default:
            switch (i.va2.xo) {
            case 198: // vcmpeqfpx (HANDLE RC 1222)
            {
                auto& vD = getVR(i.va2.vd);
                auto& vA = getVR(i.va2.va);
                auto& vB = getVR(i.va2.vb);

                TypePunnedFloatInt32 vd[4], va[4], vb[4];

                LOAD_128_BE(va, vA);
                LOAD_128_BE(vb, vB);

                for (int i = 0; i < 4; i++)
                    vd[i].V = va[i].F == vb[i].F ? 0xFFFFFFFF : 0;

                STORE_128_BE(vd, vD);
                return address + 4;
            }
            case 780: // vspltisb
            {
                auto& vD = getVR(i.vx.vd);
                TypePunnedFloatInt32 vd[4] {};
                auto simm = (uint8_t)signExtend64Format(i.vx.va, 4);
                vd[0].V = simm | simm << 8 | simm << 16 | simm << 24;
                vd[3].V = vd[2].V = vd[1].V = vd[0].V;
                STORE_128_BE(vd, vD);
                return address + 4;
            }
            case 908: // vspltisw
            {
                auto& vr = getVR(i.va2.vd);
                uint32_t simm = (uint32_t)signExtend64Format(i.va2.va, 4);
                vr.data[0] = (uint64_t)simm << 32 | simm;
                vr.data[1] = (uint64_t)simm << 32 | simm;
                return address + 4;
            }
            case 1030: // vcmpequb.
            {
                auto& vB = getVR(i.vx.vb);
                auto& vA = getVR(i.vx.va);
                auto& vD = getVR(i.vx.vd);
                TypePunnedFloatInt32 va[4], vb[4], vd[4];
                LOAD_128_BE(va, vA);
                LOAD_128_BE(vb, vB);
                
                vd[0] = vd[1] = vd[2] = vd[3] = {};
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        int mask = (0xFF << (j * 8));
                        bool comparisionSet = (va[i].V & mask) == (vb[i].V & mask);

                        if (comparisionSet) {
                            int conditionSet = 1 << (j * 8);
                            vd[i].V = (vd[i].V & ~mask) | conditionSet;
                        } else {
                            vd[i].V = vd[i].V & ~mask;
                        }
                    }
                }

                STORE_128_BE(vd, vD);
                bool t = vd[0].V == 0xFFFF'FFFF && vd[1].V == 0xFFFF'FFFF && vd[2].V == 0xFFFF'FFFF && vd[3].V == 0xFFFF'FFFF;
                bool f = !vd[0].V && !vd[1].V && !vd[2].V && !vd[3].V;
                int cond = t << 3 | f << 1;
                setCR(6, cond);
                return address + 4;
            }
            case 1220: // vxor
            {
                auto& vD = getVR(i.va2.vd);
                auto& vA = getVR(i.va2.va);
                auto& vB = getVR(i.va2.vb);

                for (int i = 0; i < 2; i++) {
                    vD.data[i] = vA.data[i] ^ vB.data[i];
                }
                return address + 4;
            }
            case 652: // vspltw
            {
                const auto& vb = getVR(i.vx2.vb);
                auto& vd = getVR(i.vx2.vd);

                uint32_t word; // I think you're a little too hard on the beaver, so is Eddie Haskell, Wally and Mrs. Cleaver
                switch (i.vx2.uimm) {
                case 0:
                    word = (uint32_t)(vb.data[0] >> 32);
                    break;
                case 1:
                    word = (uint32_t)(vb.data[0]);
                    break;
                case 2:
                    word = (uint32_t)(vb.data[1] >> 32);
                    break;
                case 3:
                    word = (uint32_t)(vb.data[1]);
                    break;
                default: word = 0;
                }

                vd.data[0] = (uint64_t)word << 32 | word;
                vd.data[1] = (uint64_t)word << 32 | word;
                return address + 4;
            }
            case 330: // vrsqrtefp
            {
                auto& vb = getVR(i.vx.vb);
                auto& vd = getVR(i.vx.vd);
                TypePunnedFloatInt32 data[4];

                LOAD_128_BE(data, vb);

                for (int i = 0; i < 4; i++)
                    data[i].F = 1.f / std::sqrt((float) data[i].F);

                STORE_128_BE(data, vd);
                return address + 4;
            }
            default:
                LOG_ERROR("UNKNOWN VECTOR 4 UPPER INSTRUCTION %d PC %llX", i.va2.xo, address);
                std::exit(0);
            }

        }
        break;
    case 7: // mulli RT, RA, SI (FIXME)
    {
        uint64_t result = (int64_t) readGPR(i.d.ra) * (int16_t) i.d.si;
        setGPR(i.d.rt, result);
        return address + 4;
    }
    case 8: // subfc RT, RA, RB
    {
        int64_t RA, RB, RT;

        RA = readGPR(i.d.ra);
        RB = (int16_t)i.d.si;
        RT = ~RA + RB + 1;
        setGPR(i.xo.rt, RT);
        setCarry(RT < ~RA);
        return address + 4;
    }
    case 10: // cmpli BF, L, RA, SI
    {
        uint64_t a, b;
        int c = 0;

        if (i.d2.l == 0) { // cmplwi
            a = (uint32_t)readGPR(i.d2.ra);
        } else { // cmpldi
            a = readGPR(i.d2.ra);
        }

        b = (uint16_t) i.d2.si;

        c = a < b ? 0b001 : (a > b ? 0b010 : 0b100);
        c |= ((readXER() >> 32) & 1) << 3;
        setCR(i.d2.bf, c);
        return address + 4;
    }
    case 11: // cmpi BF, L, RA, SI
    {
        int64_t a, b;
        int c = 0;

        if (i.d2.l == 0) { // cmpwi
            a = (int64_t)(int32_t)readGPR(i.d2.ra);
        } else { // cmpdi
            a = readGPR(i.d2.ra);
        }

        b = (int64_t)(int16_t) i.d2.si;

        c = a < b ? 0b001 : (a > b ? 0b010 : 0b100);
        c |= ((readXER() >> 32) & 1) << 3;
        setCR(i.d2.bf, c);
        return address + 4;
    }
    case 12: // addic RT, RA, SI
    {
        int64_t RA, SI, result;

        RA = readGPR(i.d.ra);
        SI = (int64_t)(int16_t) i.d.si;
        result = RA + SI;
        setGPR(i.d.rt, result);
        setCarry(result < RA);
        return address + 4;
    }
    case 14: // addi RT, RA, SI
        if (i.d.ra == 0)
            setGPR(i.d.rt, (int16_t) i.d.si);
        else
            setGPR(i.d.rt, readGPR(i.d.ra) + (int16_t) i.d.si);
        return address + 4;
    case 15: // addis RT, RA, SI
        if (i.d.ra == 0)
            setGPR(i.d.rt, (uint64_t)((int16_t) i.d.si) << 16);
        else
            setGPR(i.d.rt, readGPR(i.d.ra) + ((uint64_t)((int16_t) i.d.si) << 16));
        return address + 4;
    case 16: // bc/bca/bcl/bcla
    {
        uint64_t nia;
        uint64_t branchOffset;
        bool ctr_ok, cond_ok;
        branchOffset = signExtend64Format(i.b.bd, 13) << 2;
        bool bo0 = (i.b.bo & 16) != 0;
        bool bo1 = (i.b.bo & 8) != 0;
        bool bo2 = (i.b.bo & 4) != 0;
        bool bo3 = (i.b.bo & 2) != 0;
        bool crSet = ((readCR() >> i.b.bi) & 1) != 0;
        if (!bo2)
            setCTR(readCTR() - 1);

        ctr_ok = bo2 || ((readCTR() != 0) ^ bo3);
        cond_ok = bo0 || crSet == bo1;
        if (ctr_ok && cond_ok) nia = i.b.aa ? branchOffset : address + branchOffset;
        else nia = address + 4;
        
        if (i.b.lk) setLR(address + 4);
        return nia;
    }
    case 17: // sc (system call)
    {
        auto syscallId = readGPR(11);

        if (!__hleExecuteSyscall(this, syscallId)) {
            printf("BAD SYSTEM CALL ID 0x%llX", syscallId);
            return ~0uLL;
            std::exit(0);
        }
        return address + 4;
    }
    case 18: // b addr
    {
        uint64_t nia;
        uint64_t branchOffset;
        branchOffset = signExtend64Format(i.i.li, 23) << 2;
        nia = i.i.aa ? branchOffset /* ba */ : address + branchOffset /* b */;
        if (i.i.lk) setLR(address + 4); /* bl(a) */
        return nia;
    }
    case 19:
    {
        switch (auto opcode = i.xl.xo) {
        case 16: // bclr/bclrl
        {
            uint64_t nia;
            bool ctr_ok, cond_ok;
            bool bo0 = (i.xl.bo & 16) != 0;
            bool bo1 = (i.xl.bo & 8) != 0;
            bool bo2 = (i.xl.bo & 4) != 0;
            bool bo3 = (i.xl.bo & 2) != 0;
            bool crSet = ((readCR() >> i.xl.bi) & 1) != 0;
            if (!bo2)
                setCTR(readCTR() - 1);

            ctr_ok = bo2 || ((readCTR() != 0) ^ bo3);
            cond_ok = bo0 || crSet == bo1;
            if (ctr_ok && cond_ok) nia = readLR() & ~3;
            else nia = address + 4;
            if (i.xl.lk) setLR(address + 4);
            return nia;
        }
        case 33: // crnor
        {
            auto cr = readCR();
            bool crba = (bool)((cr & (1 << i.x.ra)) >> i.x.ra);
            bool crbb = (bool)((cr & (1 << i.x.rb)) >> i.x.rb);
            bool result = true ^ (crba | crbb);
            cr = (cr & ~(1 << i.x.rt)) | (result << i.x.rt);
            setCR(cr);
            return address + 4;
        }
        case 225: // crnand
        {
            auto cr = readCR();
            bool crba = (bool)((cr & (1 << i.x.ra)) >> i.x.ra);
            bool crbb = (bool)((cr & (1 << i.x.rb)) >> i.x.rb);
            bool result = true ^ (crba & crbb);
            cr = (cr & ~(1 << i.x.rt)) | (result << i.x.rt);
            setCR(cr);
            return address + 4;
        }
        case 417: // crorc
        {
            auto cr = readCR();
            bool crba = (bool)((cr & (1 << i.x.ra)) >> i.x.ra);
            bool crbb = (bool)((cr & (1 << i.x.rb)) >> i.x.rb) ^ true;
            cr = (cr & ~(1 << i.x.rt)) | ((crba | crbb) << i.x.rt);
            setCR(cr);
            return address + 4;
        }
        case 449: // cror
        {
            auto cr = readCR();
            bool crba = (bool)((cr & (1 << i.x.ra)) >> i.x.ra);
            bool crbb = (bool)((cr & (1 << i.x.rb)) >> i.x.rb);
            cr = (cr & ~(1 << i.x.rt)) | ((crba | crbb) << i.x.rt);
            setCR(cr);
            return address + 4;
        }
        case 528: // bcctr/bcctrl
        {
            uint64_t nia;
            bool cond_ok;
            bool bo0 = (i.xl.bo & 16) != 0;
            bool bo1 = (i.xl.bo & 8) != 0;
            bool crSet = ((readCR() >> i.xl.bi) & 1) != 0;
            cond_ok = bo0 || crSet == bo1;
            if (cond_ok) nia = readCTR() & ~3;
            else nia = address + 4;
            if (i.xl.lk) setLR(address + 4);
            return nia;
        }
        default:
            LOG_ERROR("UNIMPLEMENTED OPCODE 19 EXTENDED OPCODE %d PC 0x%08llX", opcode, getPC());
            std::exit(0);
        }
        break;
    }
    case 20: // rlwimi RA, RS, SH, MB, ME
    {
        int n = i.m.sh;
        uint64_t r, m, result;

        r = std::rotl<uint32_t>((uint32_t)readGPR(i.m.rs), n);
        m = GenerateMask64(i.m.mb + 32, i.m.me + 32);
        result = (r & m) | (readGPR(i.m.ra) & ~m);
        setGPR(i.m.ra, result);
        if (i.m.rc) setXFormatCR0(result);
        return address + 4;
    }
    case 21: // rlwinm RA, RS, SH, MB, ME
    {
        int n = i.m.sh;
        uint64_t r, m, result;
        r = std::rotl<uint32_t>((uint32_t)readGPR(i.m.rs), n);
        m = GenerateMask64(i.m.mb + 32, i.m.me + 32);
        result = r & m;
        setGPR(i.m.ra, result);
        if (i.m.rc) setXFormatCR0(result);
        return address + 4;
    }
    case 24: // ori RA, RS, UI
        setGPR(i.d.ra, readGPR(i.d.rt) | (uint64_t)i.d.si);
        return address + 4;
    case 25: // oris RA, RS, UI
        setGPR(i.d.ra, readGPR(i.d.rt) | ((uint64_t)i.d.si << 16));
        return address + 4;
    case 26: // ori RA, RS, UI
        setGPR(i.d.ra, readGPR(i.d.rt) ^ (uint64_t)i.d.si);
        return address + 4;
    case 30: // rotate
        switch (i.md.op) {
        case 0: // rldicl RA, RS, SH, MB
        {
            uint64_t result;
            int n, b;
            uint64_t r, m;

            n = i.md.sh5 << 5 | i.md.sh;
            r = std::rotl(readGPR(i.md.rs), n);
            b = (i.md.mb & 1) << 5 | (i.md.mb >> 1);

            m = GenerateMask64(b, 63);
            result = r & m;
            setGPR(i.md.ra, result);
            if (i.md.rc) setXFormatCR0(result);
            return address + 4;
        }
        case 1: // rldicr RA, RS, SH, MB            
        {
            uint64_t result;
            int n, b;
            uint64_t r, m;

            n = (i.md.sh5 << 5) | i.md.sh;
            r = std::rotl(readGPR(i.md.rs), n);
            b = ((i.md.mb & 1) << 5) | (i.md.mb >> 1);

            m = GenerateMask64(0, b);
            result = r & m;
            setGPR(i.md.ra, result);
            if (i.md.rc) setXFormatCR0(result);
            return address + 4;
        }
        case 2: // rldic RA, RS, SH, MB
        {
            uint64_t result;
            int n, b;
            uint64_t r, m;

            n = (i.md.sh5 << 5) | i.md.sh;
            r = std::rotl(readGPR(i.md.rs), n);
            b = ((i.md.mb & 1) << 5) | (i.md.mb >> 1);

            m = GenerateMask64(b, n ^ 0x3F);
            result = r & m;
            setGPR(i.md.ra, result);
            if (i.md.rc) setXFormatCR0(result);
            return address + 4;
        }
        case 3: // rldimi RA, RS, SH, MB
        {
            uint64_t result;
            int n, b;
            uint64_t r, m;

            n = i.md.sh5 << 5 | i.md.sh;
            r = std::rotl(readGPR(i.md.rs), n);
            b = (i.md.mb & 1) << 5 | (i.md.mb >> 1);

            m = GenerateMask64(b, 63 ^ n);
            result = (r & m) | (readGPR(i.md.ra) & ~m);
            setGPR(i.md.ra, result);
            if (i.md.rc) setXFormatCR0(result);
            return address + 4;
        }
        default:
            LOG_ERROR("Unimplemented OP 30 MD OP %d PC 0x%08llX", i.md.op, address);
            std::exit(0);
        }
        break;
    case 31:
        switch (i.xfx1.xo) {
        case 0:
        {
            int64_t a, b;
            int c = 0;

            if (i.x_2.l) { // cmpd
                a = (int64_t)readGPR(i.x_2.ra);
                b = (int64_t)readGPR(i.x_2.rb);
            } else { // cmpw
                a = (int32_t)readGPR(i.x_2.ra);
                b = (int32_t)readGPR(i.x_2.rb);
            }

            c = a < b ? 0b001 : (a > b ? 0b010 : 0b100);
            c |= ((readXER() >> 32) & 1) << 3;
            setCR(i.x_2.bf, c);
            return address + 4;
        }
        case 8: // subfc RT, RA, RB
        {
            int64_t RA, RB, RT;

            RA = readGPR(i.xo.ra);
            RB = readGPR(i.xo.rb);
            RT = ~RA + RB + 1;
            setGPR(i.xo.rt, RT);
            setCarry(RT < ~RA);
            if (i.xo.oe) setOverflow(((~RA >> 63) == (RB >> 63)) && ((~RA >> 63) != (RT >> 63)));
            if (i.xo.rc) setXFormatCR0(RT);
            return address + 4;
        }
        case 10: // addc RT, RA, RB
        {
            int64_t RA, RB, RT;

            RA = readGPR(i.xo.ra);
            RB = readGPR(i.xo.rb);
            RT = RA + RB;
            setGPR(i.xo.rt, RT);
            setCarry(RT < RA);
            if (i.xo.oe) setOverflow(((RA >> 63) == (RB >> 63)) && ((RA >> 63) != (RT >> 63)));
            if (i.xo.rc) setXFormatCR0(RT);
            return address + 4;
        }
        case 11: // mulhwu
        {
            auto a = (uint32_t)(readGPR(i.x.ra) >> 32);
            auto b = (uint32_t)(readGPR(i.x.rb) >> 32);
            uint64_t result;

            result = (uint64_t)a * (uint64_t)b;
            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 75: // mulhw
        {
            auto a = (int32_t)(readGPR(i.x.ra) >> 32);
            auto b = (int32_t)(readGPR(i.x.rb) >> 32);
            uint64_t result;

            result = (int64_t)a * (int64_t)b;
            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 19: // mfcr RT
        {
            setGPR(i.x.rt, readCR());
            return address + 4;
        }
        case 20: // lwarx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            uint32_t value;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->read32(effectiveAddress, &value)) {
                LOG_ERROR("BAD LWARX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }

            setGPR(i.x.rt, value);
            return address + 4;
        }
        case 21: // ldx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            uint64_t value;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->read64(effectiveAddress, &value)) {
                LOG_ERROR("BAD LWARX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }

            setGPR(i.x.rt, value);
            return address + 4;
        }
        case 23: // lwzx RT, RA, RB
        {
            uint32_t value;
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + (int32_t)readGPR(i.x.rb);

            if (!memory->read32(effectiveAddress, &value)) {
                LOG_ERROR("BAD LWZX [%08llX] PC 0x%08X", effectiveAddress, getPC());
                std::exit(0);
            }

            setGPR(i.x.rt, (uint64_t) value);
            return address + 4;
        }
        case 24: // slw(.) RA, RS, RB
        {
            uint32_t rb = (uint32_t) readGPR(i.x.rb);
            int n = rb & 0x1F;
            uint32_t rs = (uint32_t) readGPR(i.x.rt);
            uint32_t result;
            result = rs << n;

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 26: // cntlzw RA, RS
        {
            uint32_t result;

            result = std::countl_zero((uint32_t)readGPR(i.x.rt));
            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 28: // and RA, RS, RB
        {
            uint64_t result;

            result = readGPR(i.x.rt) & readGPR(i.x.rb);

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 32: // cmpl BF, L, RA, RB
        {
            uint64_t a, b;
            int c = 0;

            if (i.x_2.l) { // cmpld
                a = readGPR(i.x_2.ra);
                b = readGPR(i.x_2.rb);
            } else { // cmplw
                a = (uint32_t)readGPR(i.x_2.ra);
                b = (uint32_t)readGPR(i.x_2.rb);
            }

            c = a < b ? 0b001 : (a > b ? 0b010 : 0b100);
            c |= ((readXER() >> 32) & 1) << 3;
            setCR(i.x_2.bf, c);
            return address + 4;
        }
        case 40: // subf RT, RA, RB
        {
            uint64_t RA, RB, RT;

            RA = readGPR(i.xo.ra);
            RB = readGPR(i.xo.rb);
            RT = ~RA + RB + 1;
            setGPR(i.xo.rt, RT);
            if (i.xo.oe) setOverflow(((~RA >> 63) == (RB >> 63)) && ((~RA >> 63) != (RT >> 63)));
            if (i.xo.rc) setXFormatCR0(RT);
            return address + 4;
        }
        case 58: // cntlzd RA, RS
        {
            uint64_t result;

            result = std::countl_zero(readGPR(i.x.rt));
            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 60: // andc(.) RA, RS, RB
        {
            uint64_t rs = readGPR(i.x.rt);
            uint64_t rb = readGPR(i.x.rb);
            uint64_t result = rs & ~rb;

            setGPR(i.x.ra, result);
            return address + 4;
        }

        case 87: // lbzx RT, RA, RB
        {
            uint8_t value;
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->read8(effectiveAddress, &value)) {
                LOG_ERROR("BAD LWZX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, address);
                std::exit(0);
            }

            setGPR(i.x.rt, (uint8_t) value);
            return address + 4;
        }
        case 104: // neg RT, RA
        {
            uint64_t result = ~readGPR(i.xo.ra) + 1;

            setGPR(i.xo.rt, result);
            if (i.xo.rc)
                setXFormatCR0(result);
            if (i.xo.oe)
                setOverflow((result & 0x8000000000000000uLL) != 0);
            return address + 4;
        }
        case 124: // nor RA, RS, RB
        {
            uint64_t result;

            result = ~(readGPR(i.x.rt) | readGPR(i.x.rb));

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 144:
        {
            alignas(4) static const uint8_t s_table[16][4] = {
                {0, 0, 0, 0},
                {0, 0, 0, 1},
                {0, 0, 1, 0},
                {0, 0, 1, 1},
                {0, 1, 0, 0},
                {0, 1, 0, 1},
                {0, 1, 1, 0},
                {0, 1, 1, 1},
                {1, 0, 0, 0},
                {1, 0, 0, 1},
                {1, 0, 1, 0},
                {1, 0, 1, 1},
                {1, 1, 0, 0},
                {1, 1, 0, 1},
                {1, 1, 1, 0},
                {1, 1, 1, 1},
            };

            uint64_t s = readGPR(i.xfx2.rs);

            if (i.xfx2.op) { // MTOCRF
                uint32_t n = std::countl_zero<uint32_t>(i.xfx2.fxm) & 7;
                uint64_t v = (s >> ((n * 4) ^ 0x1c)) & 0xf;
                uint32_t mask = 1 << (n * 4) | 1 << (n * 4 + 1) | 1 << (n * 4 + 2) | 1 << (n * 4 + 3);
                
                setCR((readCR() & ~mask) | *reinterpret_cast<const u32*>(s_table + v) << n);
                return address + 4;
            }
            else { // mtcrf
                for (u32 x = 0; x < 8; x++) {
                    if (i.xfx2.fxm & (128 >> x))
                    {
                        const u64 v = (s >> ((x * 4) ^ 0x1c)) & 0xf;
                        // ppu.cr.fields[i] = *reinterpret_cast<const u32*>(s_table + v);
                    }
                }
                LOG_ERROR("UNIMPLEMENTED MTCRF");
                std::exit(0);
            }
            break;
        }
        case 149: // stdx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);
            if (!memory->write64(effectiveAddress, readGPR(i.x.rt))) {
                LOG_ERROR("BAD STDX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }
            return address + 4;
        }
        case 150: // stwcx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            uint32_t val = (uint32_t) readGPR(i.x.rt);
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->write32(effectiveAddress, val)) {
                LOG_ERROR("BAD STWCX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }

            setCR(0, 0b100); // Always unreserved from the reservation station
            return address + 4;
        }
        case 151: // stwx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->write32(effectiveAddress, (uint32_t) readGPR(i.x.rt))) {
                LOG_ERROR("BAD STWX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }
            return address + 4;
        }
        case 215: // stbx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->write8(effectiveAddress, (uint8_t) readGPR(i.x.rt))) {
                LOG_ERROR("BAD STBX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }
            return address + 4;
        }
        case 266: // add RT, RA, RB
        {
            uint64_t RA, RB, RT;

            RA = readGPR(i.xo.ra);
            RB = readGPR(i.xo.rb);
            RT = RA + RB;
            setGPR(i.xo.rt, RT);
            if (i.xo.oe) setOverflow(((RA >> 63) == (RB >> 63)) && ((RA >> 63) != (RT >> 63)));
            if (i.xo.rc) setXFormatCR0(RT);
            return address + 4;
        }
        case 278: // DCBT (Data cache block touch)
        {
            uint64_t effectiveAddress;
            if (i.xo.ra == 0)
                effectiveAddress = 0;
            else
                effectiveAddress = readGPR(i.xo.ra);

            effectiveAddress += readGPR(i.xo.rb);
            return address + 4;
        }
        case 316: // xor RA, RS, RB
        {
            uint64_t result;

            result = readGPR(i.x.rt) ^ readGPR(i.x.rb);

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 339: // mfspr RT, SPR
            switch (int spr = ((i.xfx1._sprtbr >> 5) & 0x1F) | (i.xfx1._sprtbr & 0x1F)) {
            case 8:
                setGPR(i.xfx1.rt, readLR());
                return address + 4;
            default:
                LOG_ERROR("UNKNOWN MFSPR SPR %d PC 0x%08llX", spr, address);
                std::exit(0);
            }
            break;
        case 444: // or RA, RS, RB
        {
            uint64_t result;

            result = readGPR(i.x.rt) | readGPR(i.x.rb);

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 233: // muldw
        {
            auto a = readGPR(i.x.ra);
            auto b = readGPR(i.x.rb);
            uint64_t result;

            result = (int64_t)a * (int64_t)b;
            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 235: // mullw
        {
            auto a = (int32_t)readGPR(i.x.ra);
            auto b = (int32_t)readGPR(i.x.rb);
            uint64_t result;

            result = (int64_t)a * (int64_t)b;
            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 371: // mftb rD, TBR
        {
            int tbr = (i.tbr.tbr & 0x1F) << 5 | (i.tbr.tbr >> 5);
            switch (tbr) {
            case 268:
                setGPR(i.tbr.rd, bank->TBR);
                return address + 4;
            default:
                LOG_ERROR("UNKNOWN TBR INDEX %d PC 0x%08llx", tbr, address);
                std::exit(0);
            }
            break;
        }
        case 407: // sthx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            if (!memory->write16(effectiveAddress, (uint16_t) readGPR(i.x.rt))) {
                LOG_ERROR("BAD STBX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }
            return address + 4;
        }
        case 457: // divdu(o.)
        {
            auto dividend = readGPR(i.x.ra);
            auto divisor = readGPR(i.x.rb);
            uint64_t result;
            if (divisor == 0)
                result = ~0uLL; // Undefined like that?
            else
                result = dividend / divisor;

            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 459: // divwu(o.)
        {
            auto dividend = (uint32_t)readGPR(i.x.ra);
            auto divisor = (uint32_t)readGPR(i.x.rb);
            uint64_t result;
            if (divisor == 0)
                result = ~0uLL; // Undefined like that?
            else
                result = dividend / divisor;

            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 489: // divd(o.)
        {
            auto dividend = (int64_t)readGPR(i.x.ra);
            auto divisor = (int64_t)readGPR(i.x.rb);
            uint64_t result;
            if (divisor == 0)
                result = ~0uLL; // Undefined like that?
            else
                result = dividend / divisor;

            setGPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 467: // mtspr SPR, RS
            switch (int spr = ((i.xfx1._sprtbr >> 5) & 0x1F) | (i.xfx1._sprtbr & 0x1F)) {
            case 8:
                setLR(readGPR(i.xfx1.rt));
                return address + 4;
            case 9:
                setCTR(readGPR(i.xfx1.rt));
                return address + 4;
            default:
                LOG_ERROR("UNKNOWN MTSPR SPR %d PC 0x%08llX", spr, address);
                std::exit(0);
            }
            break;
        case 536: // srw(.) RA, RS, RB
        {
            uint32_t rb = (uint32_t) readGPR(i.x.rb);
            int n = rb & 0x1F;
            uint32_t rs = (uint32_t) readGPR(i.x.rt);
            uint32_t result;
            result = rs >> n;

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 539: // srd(.) RA, RS, RB
        {
            uint64_t rb = readGPR(i.x.rb);
            int n = rb & 0x1F;
            uint64_t rs = readGPR(i.x.rt);
            uint64_t result;
            result = rs >> n;

            setGPR(i.x.ra, result);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 663: // stfsx RS, RA, RB
        {
            TypePunnedFloatInt32 value;
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            value.F = (float) readFPR(i.x.rt);
            if (!memory->write32(effectiveAddress, value.V)) {
                LOG_ERROR("BAD STFSX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }
            return address + 4;
        }
        case 824: // srawi RA, RS, RB (FIXME?)
        {
            int n = i.x.rb;
            uint64_t rs = readGPR(i.x.rt);
            uint32_t r = std::rotl((uint32_t) rs, 64 - n);
            uint64_t m;
            uint64_t result;

            uint64_t replicatedBits;

            m = GenerateMask64(n + 32, 63);
            if ((rs >> 31) != 0)
                replicatedBits = -1LL;
            else
                replicatedBits = 0;

            result = (r & m) | (replicatedBits & ~m);
            setGPR(i.x.ra, result);
            setCarry(replicatedBits && ((r & ~m) & 0xFFFFFFFF) != 0);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 792: // sraw RA, RS, RB (FIXME?)
        {
            uint64_t rb = readGPR(i.x.rb);
            uint64_t rs = readGPR(i.x.rt);
            int n = rb & 0x1F;
            uint32_t r = std::rotl((uint32_t) rs, 64 - n);
            uint64_t m;
            uint64_t result;


            if ((rb & (1 << 6)) == 0) {
                m = GenerateMask64(n + 32, 63);
            } else
                m = 0;

            uint64_t replicatedBits;

            if ((rs >> 31) != 0)
                replicatedBits = -1LL;
            else
                replicatedBits = 0;

            result = (r & m) | (replicatedBits & ~m);

            setGPR(i.x.ra, result);
            setCarry(replicatedBits && ((r & ~m) & 0xFFFFFFFF) != 0);
            if (i.x.rc)
                setXFormatCR0(result);
            return address + 4;
        }
        case 922: // extsh(.) RA, RS
        {
            uint64_t result = (int16_t)readGPR(i.x.rt);

            setGPR(i.x.ra, result);
            if (i.x.rc) setXFormatCR0(result);
            return address + 4;
        }
        case 954: // extsb(.) RA, RS
        {
            uint64_t result = (int8_t)readGPR(i.x.rt);

            setGPR(i.x.ra, result);
            if (i.x.rc) setXFormatCR0(result);
            return address + 4;
        }
        case 983: // stfiwx RS, RA, RB
        {
            uint64_t b, effectiveAddress;
            if (i.x.ra == 0) b = 0;
            else b = readGPR(i.x.ra);
            effectiveAddress = b + readGPR(i.x.rb);

            auto frs = readFPR(i.x.rt);

            TypePunnedFloatInt64 result;
            result.F = frs;

            if (!memory->write32(effectiveAddress, result.V & 0xFFFFFFFF)) {
                LOG_ERROR("BAD STFIWX [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
                std::exit(0);
            }
            return address + 4;
        }
        case 986: // extsw(.) RA, RS
        {
            uint64_t result = (int32_t)readGPR(i.x.rt);

            setGPR(i.x.ra, result);
            if (i.x.rc) setXFormatCR0(result);
            return address + 4;
        }
        
        // SIMD instructions
        case 103: // lvx VD, RA, RB
        {
            uint64_t effectiveAddress, b;
            if (i.x.ra == 0)
                b = 0;
            else
                b = readGPR(i.x.ra);

            effectiveAddress = (b + readGPR(i.x.rb)) & 0xFFFFFFFFFFFFFFF0;
            auto& vr = getVR(i.x.rt);

            if (!memory->read64(effectiveAddress, &vr.data[0])) {
                LOG_ERROR("BAD LVX ADDRESS 0x%08llX", effectiveAddress);
                std::exit(0);
            }

            if (!memory->read64(effectiveAddress + 0x8, &vr.data[1])) {
                LOG_ERROR("BAD LVX ADDRESS 0x%08llX", effectiveAddress + 0x8);
                std::exit(0);
            }
            return address + 4;
        }
        case 231: // stvx VS, RA, RB
        {
            uint64_t effectiveAddress, b;
            if (i.x.ra == 0)
                b = 0;
            else
                b = readGPR(i.x.ra);

            effectiveAddress = (b + readGPR(i.x.rb)) & 0xFFFFFFFFFFFFFFF0;
            auto& vr = getVR(i.x.rt);

            if (!memory->write64(effectiveAddress, vr.data[0])) {
                LOG_ERROR("BAD STVX ADDRESS 0x%08llX", effectiveAddress);
                std::exit(0);
            }

            if (!memory->write64(effectiveAddress + 0x8, vr.data[1])) {
                LOG_ERROR("BAD STVX ADDRESS 0x%08llX", effectiveAddress + 0x8);
                std::exit(0);
            }
            return address + 4;
        }
        case 519: // lvlx vD, rA, rB
        {
            uint64_t base, effectiveAddress;
            auto& vD = getVR(i.x.rt);
            int eb;
            if (i.x.ra == 0)
                base = 0;
            else
                base = readGPR(i.x.ra);

            effectiveAddress = base + readGPR(i.x.rb);
            eb = effectiveAddress & 0xF;
            uint8_t data[16];

            memory->readBytes(effectiveAddress, &data[0], 16 - eb);
            uint64_t upper = ((u64_be *)&data[0])->getValue();
            uint64_t lower = ((u64_be *)&data[8])->getValue();

            eb <<= 3;
            
            upper <<= eb;
            lower <<= eb;

            vD.data[0] = upper;
            vD.data[1] = lower;
            return address + 4;
        }
        default:
            dumpRegisters();
            LOG_ERROR("UNIMPLEMENTED OP 31 EXTENDED OPCODE %d PC 0x%08llX", i.xfx1.xo, address);
            std::exit(0);
        }
        break;
    case 32: // lwz RT, D(RA)
    {
        uint64_t b, effectiveAddress;
        uint32_t val;
        if (i.d.ra == 0)
            b = 0;
        else
            b = readGPR(i.d.ra);

        effectiveAddress = b + (int16_t) i.d.si;
        if (!memory->read32(effectiveAddress, &val)) {
            LOG_ERROR("BAD LWZ EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }
        setGPR(i.d.rt, (uint64_t) val);
        return address + 4;
    }
    case 33: // lwzu RT, D(RA)
    {
        uint64_t effectiveAddress;
        uint32_t val;

        effectiveAddress = readGPR(i.d.ra) + (int16_t) i.d.si;
        if (!memory->read32(effectiveAddress, &val)) {
            LOG_ERROR("BAD LWZ EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }
        setGPR(i.d.rt, (uint64_t) val);
        setGPR(i.d.ra, effectiveAddress);
        return address + 4;
    }
    case 34: // lbz RT, D(RA)
    {
        uint64_t b, effectiveAddress;
        uint8_t val;
        if (i.d.ra == 0)
            b = 0;
        else
            b = readGPR(i.d.ra);

        effectiveAddress = b + (int16_t) i.d.si;
        if (!memory->read8(effectiveAddress, &val)) {
            LOG_ERROR("BAD LBZ EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }

        setGPR(i.d.rt, (uint8_t) val);
        return address + 4;
    }
    case 35: // lbzu RT, D(RA)
    {
        uint64_t effectiveAddress;
        uint8_t val;
        effectiveAddress = readGPR(i.d.ra) + (int16_t) i.d.si;
        if (!memory->read8(effectiveAddress, &val)) {
            LOG_ERROR("BAD LBZ EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }

        setGPR(i.d.rt, (uint8_t) val);
        setGPR(i.d.ra, effectiveAddress);
        return address + 4;
    }
    case 36: // stw RS, D(RA)
    {
        uint64_t effectiveAddress;
        uint64_t b;

        b = i.d.ra == 0 ? 0 : readGPR(i.d.ra);
        effectiveAddress = b + (int16_t) i.d.si;

        if (!memory->write32(effectiveAddress, (uint32_t)readGPR(i.d.rt))) {
            LOG_ERROR("BAD STW [%08llX] = 0x%08X PC 0x%08llX LR 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address, readLR());
            std::exit(0);
        }
        return address + 4;
    }
    case 38: // stb RS, D(RA)
    {
        uint64_t effectiveAddress;
        uint64_t b;

        b = i.d.ra == 0 ? 0 : readGPR(i.d.ra);
        effectiveAddress = b + (int16_t) i.d.si;
        if (!memory->write8(effectiveAddress, (uint8_t)readGPR(i.d.rt))) {
            LOG_ERROR("BAD STB [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
            std::exit(0);
        }
        return address + 4;
    }
    case 39: // stbu RS, D(RA)
    {
        uint64_t effectiveAddress;
        effectiveAddress = readGPR(i.d.ra) + (int16_t) i.d.si;
        if (!memory->write8(effectiveAddress, (uint8_t)readGPR(i.d.rt))) {
            LOG_ERROR("BAD STB [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
            std::exit(0);
        }

        setGPR(i.d.ra, effectiveAddress);
        return address + 4;
    }
    case 40: // lhz RT, D(RA)
    {
        uint64_t b, effectiveAddress;
        uint16_t val;
        if (i.d.ra == 0)
            b = 0;
        else
            b = readGPR(i.d.ra);

        effectiveAddress = b + (int16_t) i.d.si;

        if (!memory->read16(effectiveAddress, &val)) {
            LOG_ERROR("BAD LHZ EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }

        setGPR(i.d.rt, (uint16_t) val);
        return address + 4;
    }
    case 44: // sth RS, D(RA)
    {
        uint64_t effectiveAddress;
        uint64_t b;

        b = i.d.ra == 0 ? 0 : readGPR(i.d.ra);
        effectiveAddress = b + (int16_t) i.d.si;

        if (!memory->write16(effectiveAddress, (uint16_t)readGPR(i.d.rt))) {
            LOG_ERROR("BAD STH [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, (uint32_t)readGPR(i.d.rt), address);
            std::exit(0);
        }
        return address + 4;
    }
    case 48: // lfs FRT, D(RA)
    {
        uint64_t b, effectiveAddress;
        TypePunnedFloatInt32 val;
        if (i.d.ra == 0)
            b = 0;
        else
            b = readGPR(i.d.ra);

        effectiveAddress = b + (int16_t) i.d.si;
        if (!memory->read32(effectiveAddress, &val.V)) {
            LOG_ERROR("BAD LFS EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }

        setFPR(i.d.rt, (double)val.F);
        return address + 4;
    }
    case 50: // lfd FRT, D(RA)
    {
        uint64_t b, effectiveAddress;
        TypePunnedFloatInt64 val;
        if (i.d.ra == 0)
            b = 0;
        else
            b = readGPR(i.d.ra);

        effectiveAddress = b + (int16_t) i.d.si;
        if (!memory->read64(effectiveAddress, &val.V)) {
            LOG_ERROR("BAD LFD EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
            std::exit(0);
        }

        setFPR(i.d.rt, val.F);
        return address + 4;
    }
    case 52: // stfs RS, D(RA)
    {
        uint64_t effectiveAddress;
        uint64_t b;

        TypePunnedFloatInt32 value;
        b = i.d.ra == 0 ? 0 : readGPR(i.d.ra);
        effectiveAddress = b + (int16_t) i.d.si;
        value.F = (float)readFPR(i.d.rt);
        if (!memory->write32(effectiveAddress, value.V)) {
            LOG_ERROR("BAD STFS [%08llX] = 0x%08X PC 0x%08llX", effectiveAddress, value.V, address);
            std::exit(0);
        }
        return address + 4;
    }
    case 54: // stfd RS, D(RA)
    {
        uint64_t effectiveAddress;
        uint64_t b;
        TypePunnedFloatInt64 data;

        b = i.d.ra == 0 ? 0 : readGPR(i.d.ra);
        effectiveAddress = b + (int16_t) i.d.si;

        data.F = readFPR(i.d.rt);
        if (!memory->write64(effectiveAddress, data.V)) {
            LOG_ERROR("BAD STFD [%08llX] = 0x%08llX PC 0x%08llX", effectiveAddress, data.V, address);
            std::exit(0);
        }
        return address + 4;
    }
    case 58:
    {
        switch (i.ds.xo) {
        case 0: // ld RT, DS(RA)
        {
            uint64_t effectiveAddress, b;
            uint64_t RT;
            int64_t off = signExtend64Format(i.ds.ds, 13) << 2;
            if (i.ds.ra == 0)
                b = 0;
            else
                b = readGPR(i.ds.ra);

            effectiveAddress = b + off;
            if (!memory->read64(effectiveAddress, &RT)) {
                LOG_ERROR("BAD LD [%08llX] PC 0x%08llX", effectiveAddress, address);
                std::exit(0);
            }

            setGPR(i.ds.rt, RT);
            return address + 4;
        }
        case 1: // ldu RT, DS(RA)
        {
            uint64_t effectiveAddress;
            uint64_t RT;
            int64_t off = signExtend64Format(i.ds.ds, 13) << 2;

            effectiveAddress = readGPR(i.ds.ra) + off;
            if (!memory->read64(effectiveAddress, &RT)) {
                LOG_ERROR("BAD LD [%08llX] PC 0x%08llX", effectiveAddress, address);
                std::exit(0);
            }

            setGPR(i.ds.rt, RT);
            setGPR(i.ds.ra, effectiveAddress);
            return address + 4;
        }
        default:
            LOG_ERROR("UNKNOWN LOAD DOUBLEWORD XO %d PC 0x%08llX", i.ds.xo, getPC());
            std::exit(0);
        }
        break;
    }
    case 59:
    {
        switch (i.a.xo) {
        case 21: // fadds(.) FRT, FRA, FRB
        {
            auto fra = readFPR(i.a.fra);
            auto frc = readFPR(i.a.frb);
            float result = (float)(fra + frc);
            setFPR(i.a.frt, result);
            setFPRF(result);
            if (i.a.rc)
                setXFormatCR1((double)result);
            return address + 4;
        }
        case 25: // fmuls(.) FRT, FRA, FRC
        {
            auto fra = readFPR(i.a.fra);
            auto frc = readFPR(i.a.frc);
            float result = (float)(fra * frc);
            setFPR(i.a.frt, result);
            setFPRF(result);
            if (i.a.rc)
                setXFormatCR1((double)result);
            return address + 4;
        }
        case 29:
        {
            auto fra = (float)readFPR(i.a.fra);
            auto frb = (float)readFPR(i.a.fra);
            auto frc = (float)readFPR(i.a.frc);
            auto result = fra * frc + frb;

            setFPR(i.a.frt, result);
            setFPRF(result);
            if (i.a.rc)
                setXFormatCR1((double)result);
            return address + 4;
        }
        default:
            LOG_ERROR("UNIMPLEMENTED OP 59 EXTENDED OPCODE %d PC 0x%08llX", i.a.xo, address);
            std::exit(0);
        }
        break;
    }
    case 62:
    {
        uint64_t effectiveAddress;
        int64_t off = signExtend64Format(i.ds.ds, 13) << 2;
        uint64_t b;

        switch (i.ds.xo) {
        case 0: // std RS, DS(RA)
            b = i.ds.ra == 0 ? 0 : readGPR(i.ds.ra);
            effectiveAddress = b + off;
            if (!memory->write64(effectiveAddress, readGPR(i.ds.rt))) {
                LOG_ERROR("INVALID STD EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
                std::exit(0);
            }
            break;
        case 1: // stdu RS, DS(RA)
            effectiveAddress = readGPR(i.ds.ra) + off;
            if (!memory->write64(effectiveAddress, readGPR(i.ds.rt))) {
                LOG_ERROR("INVALID STDU EFFECTIVE ADDRESS 0x%08llX PC 0x%08llX", effectiveAddress, address);
                std::exit(0);
            }

            setGPR(i.ds.ra, effectiveAddress);
            break;
        default:
            LOG_ERROR("UNIMPLEMENTED XO %d DSFORM FOR OPCODE 62 STORE DOUBLEWORD PC 0x%08llX", i.ds.xo, address);
            break;
        }
        return address + 4;
    }
    case 63: // FIXME
    {
        switch (i.x.xo) {
        case 0: // fcmpu BF, FRA, FRB
        {
            auto fra = readFPR(i.x.ra);
            auto frb = readFPR(i.x.rb);
            int c = 0;

            if (std::isnan(fra) || std::isnan(frb)) {
                c |= 0b1000;
            } else if (fra < frb) {
                c |= 0b0001;
            } else if (fra > frb) {
                c |= 0b0010;
            } else {
                c |= 0b0100;
            }

            setCR(i.x.rt >> 2, c);
            return address + 4;
        }
        case 12: // frsp FRT, FRB
        {

            auto operand = std::round((float)(readFPR(i.x.rb)));

            setFPR(i.x.rt, operand);
            setFPRF(operand);
            setFR(0.f, (double) operand);
            if (i.x.rc)
                setXFormatCR1(operand);
            return address + 4;
        }
        case 15: // fctiwz FRT, FRB (FIXME)
        {
            auto frb = readFPR(i.x.rb);
            auto operand = (uint64_t)std::round((double)frb);
            if (frb > 2147483647.) {
                operand |= 0x7FFFFFFFuLL << 32;
            } else if (frb < -2147483648.) {
                operand |= 0x80000000uLL << 32;
            }
            setFPR(i.x.rt, (double) operand);
            setFPRF((double) operand);
            setFR(0.f, (double) operand);
            if (i.x.rc)
                setXFormatCR1((double) operand);
            return address + 4;
        }
        case 815: // fctidz FRT, FRB (FIXME)
        {
            auto frb = readFPR(i.x.rb);
            auto operand = (uint64_t)std::round((double)frb);
            setFPR(i.x.rt, (double) operand);
            setFPRF((double) operand);
            setFR(0.f, (double) operand);
            if (i.x.rc)
                setXFormatCR1((double) operand);
            return address + 4;
        }
        case 18: // fdiv FRT, FRA, FRB
        {
            auto fra = (float)readFPR(i.x.ra);
            auto frb = (float)readFPR(i.x.rb);
            auto result = fra / frb;

            setFPR(i.x.rt, (double) result);
            setFPRF((double) result);
            setFR(0.f, (double) result);
            if (i.x.rc)
                setXFormatCR1((double)result);
            return address + 4;
        }
        case 40: // fneg FRT, FRB
        {
            auto result = -readFPR(i.x.rb);

            setFPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR1(result);
            return address + 4;
        }
        case 846: // fcfid FRT, FRB (FIXME)
        {
            TypePunnedFloatInt64 value;
            value.F = readFPR(i.x.rb);
            auto result = (double) value.V;
            setFPR(i.x.rt, result);
            setFPRF(result);
            setFR(0., result);
            if (i.x.rc)
                setXFormatCR1(result);
            return address + 4;
        }
        case 72: // fmr FRT, FRB
        {
            auto result = readFPR(i.x.rb);
            setFPR(i.x.rt, result);
            if (i.x.rc)
                setXFormatCR1(result);
            return address + 4;
        }
        default:
            switch (i.x_3.xo) {
            case 20: // fsub
            {
                auto fra = (float)readFPR(i.x_3.fra);
                auto frb = (float)readFPR(i.x_3.frb);
                auto result = fra - frb;

                setFPR(i.x_3.frt, (double) result);
                setFPRF((double) result);
                setFR(0.f, (double) result);
                if (i.x_3.rc)
                    setXFormatCR1((double)result);
                return address + 4;
            }
            case 25: // fmul
            {
                auto fra = (float)readFPR(i.x_3.fra);
                auto frc = (float)readFPR(i.x_3.frc);
                auto result = fra * frc;

                setFPR(i.x_3.frt, (double) result);
                setFPRF((double) result);
                setFR(0.f, (double) result);
                if (i.x_3.rc)
                    setXFormatCR1((double)result);
                return address + 4;
            }
            default:
                LOG_ERROR("(MAYBE) UNIMPLEMENTED OP 63 EXTENDED OPCODE %d PC 0x%08llX", i.x_3.xo, address);
            }
            LOG_ERROR("UNIMPLEMENTED OP 63 EXTENDED OPCODE %d PC 0x%08llX", i.x.xo, address);
            std::exit(0);
        }
        break;
    }
    default:
        LOG_ERROR("UNIMPLEMENTED OPCODE %d PC 0x%08llX", i.opcd, address);
        dumpRegisters();
    }
    return ~0uLL;
}