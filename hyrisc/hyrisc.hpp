#pragma once

#include "../log.hpp"

#include <cstdint>
#include <array>
#include <cstring>
#include <csignal>
#include <cfenv>

#include "state.hpp"
#include "types.hpp"
#include "flags.hpp"
#include "alu.hpp"
#include "fpu.hpp"

#include <iostream>

enum hyrisc_register_names_t {
    r0 , r1 , r2 , r3 ,
    r4 , r5 , r6 , r7 ,
    r8 , r9 , r10, r11,
    r12, r13, r14, r15,
    r16, r17, r18, r19,
    r20, r21, r22, r23,
    r24, r25, r26, r27,
    r28, lr , sp , pc
};

const char* hyrisc_register_names[] = {
    "r0" , "r1" , "r2" , "r3" ,
    "r4" , "r5" , "r6" , "r7" ,
    "r8" , "r9" , "r10", "r11",
    "r12", "r13", "r14", "r15",
    "r16", "r17", "r18", "r19",
    "r20", "r21", "r22", "r23",
    "r24", "r25", "r26", "r27",
    "r28", "fp" , "sp" , "pc"
};

void hyrisc_bci_update(hyrisc_t* proc) {
    if (proc->ext.bci.busreq && proc->ext.bci.busack) {
        proc->ext.bci.busreq = false;
        proc->ext.bci.busack = false;

        return;
    }

    if (!proc->ext.bci.busirq) return;

    bool open_bus = proc->ext.bci.busreq && !proc->ext.bci.busack;

    // If there was a bus error on last clock
    // or nothing was put on the bus (aka Open Bus), then:
    if (proc->ext.bci.be || open_bus) {

        // If Bus error IRQ has already been acknowledged, then
        // clear bus error and lower IRQ
        if (proc->ext.pic.irqack) {
            proc->ext.bci.be = 0x0;

            proc->ext.pic.irqack = false;
            proc->ext.pic.irq = false;

            return;
        }

        proc->ext.pic.irqack = false;
        proc->ext.pic.irq = true;

        // 256 bytes to handle each bus error mapped on f0000000-f000ffff
        proc->ext.pic.v = 0xf0000000 | (proc->ext.bci.be << 8);
    }
}

void hyrisc_set_cpuid(hyrisc_t* proc, const char* id, hyint_t core) {
    proc->id = id;
    proc->core = core;
}

void hyrisc_reset(hyrisc_t* proc) {
    std::memset(&proc->internal, 0, sizeof(proc->internal));

    proc->ext.bci.a      = 0xffffffff;
    proc->ext.bci.d      = 0xffffffff;
    proc->ext.bci.rw     = false;
    proc->ext.bci.s      = 0x3;
    proc->ext.bci.be     = 0x0;
    proc->ext.bci.busreq = false;
    proc->ext.pic.irq    = false;
    proc->ext.pic.irqack = false;
    proc->ext.bci.busirq = true;
    proc->ext.freeze     = false;

    proc->internal.instruction = 0xffffffff;

    proc->internal.r[pc] = proc->ext.pic.v;
}

bool hyrisc_handle_signals(hyrisc_t* proc) {
    // If RESET is high, then reset the CPU
    if (proc->ext.reset) {
        hyrisc_reset(proc);

        return false;
    }

    // If FREEZE is high, then idle
    if (proc->ext.freeze) {
        return false;
    }

    // If IRQ is high, then jump to the vector on V0-V31
    if (proc->ext.pic.irq) {
        proc->internal.cycle = 0;
        proc->internal.r[pc] = proc->ext.pic.v;

        proc->ext.pic.irqack = true;

        // Handling an IRQ takes 1 input clock
        return true;
    }

    // All signals handled
    return true;
}

enum hyrisc_access_size_t {
    AS_BYTE,
    AS_SHORT,
    AS_LONG,
    AS_EXECUTE
};

void hyrisc_init_read(hyrisc_t* proc, hyu32_t addr, hyint_t size = AS_LONG) {
    proc->ext.bci.a = addr;
    proc->ext.bci.s = size;

    proc->ext.bci.rw = false;
    proc->ext.bci.busreq = true;

    proc->ext.bci.be = 0x0;
}

void hyrisc_init_write(hyrisc_t* proc, hyu32_t addr, hyu32_t value, hyint_t size = AS_LONG) {
    proc->ext.bci.a = addr;
    proc->ext.bci.s = size;
    proc->ext.bci.d = value;

    proc->ext.bci.rw = true;
    proc->ext.bci.busreq = true;

    proc->ext.bci.be = 0x0;
}

bool hyrisc_execute(hyrisc_t*, hyint_t);
void hyrisc_decode(hyrisc_t*);

void hyrisc_clock(hyrisc_t* proc) {
    // Update BCI
    hyrisc_bci_update(proc);

    // Handle signals
    if (!hyrisc_handle_signals(proc)) return;

    switch (proc->internal.cycle) {
        case 0x0: {
            hyrisc_init_read(proc, proc->internal.r[pc], AS_EXECUTE);

            proc->internal.cycle++;
        } break;

        case 0x1: {
            // Copy the contents of the data bus to
            // the instruction latch for decoding
            proc->internal.instruction = proc->ext.bci.d;

            proc->internal.r[pc] += 4;
            proc->internal.cycle++;
        } break;

        case 0x2: {
            // Decode and execute are done on the same clock cycle
            hyrisc_decode(proc);

            bool done = hyrisc_execute(proc, 0);

            // If its done, then reset cycle counter
            if (done) {
                proc->internal.cycle = 0;
                proc->internal.r[r0] = 0;
            } else {
                // Instruction needs an extra cycle to wait for I/O
                proc->internal.cycle++;
            }
        } break;

        case 0x3: {
            // Instruction has to finish processing I/O on cycle 4
            hyrisc_execute(proc, 1);
    
            proc->internal.cycle = 0;
            proc->internal.r[r0] = 0;
        } break;
    }
}

/*
    lui     %gpr10,    0xaabb; gpr0 = 0xaabb0000 flags = 00000010
    or      %gpr10,    0xccdd; gpr0 = 0xaabbccdd flags = 00000010
    lui     %gpr10,    0xffff; gpr0 = 0xffff0000 flags = 00000010
    or      %gpr10,    0xffff; gpr0 = 0xffffffff flags = 00000010
    addu    %gpr10,    1;      gpr0 = 0x00000000 flags = 00001001

    gpr0 (r1) is now 0, both zero and carry flags are set
    and the negative flag is cleared
*/

// Main registers:
// Number      Mnemonic            Preserved?      Purpose
// r0       -> r0                  -               Zero register
// r1-r15   -> gpr0, gpr14         volatile        General Purpose Registers
// r16-r18  -> a0, a2              preserved       Function arguments Num. 1-3
// r19      -> rr0                 volatile        Return Register 0
// r20-r22  -> a3, a5              preserved       Function arguments Num. 4-6
// r23      -> rr2                 volatile        Return Register 2
// r24-r27  -> lr0-lr3             volatile        Link Register 0-3
// r28      -> ir                  volatile        Index register
// r29      -> br                  volatile        Base register
// r30      -> sp                  volatile        Stack pointer
// r31      -> pc                  -               Program counter

// Status register
// Flags:
// 0000cvnz
// z = Zero flag
// n = Negative flag
// c = Unsigned Carry flag
// v = Signed Carry flag

// Floating point registers:
// fp0-fp7  volatile
// fp8-fp15 non-volatile

enum hyrisc_opcodes_t {
    HY_MOV       = 0xff,
    HY_LI        = 0xfe,
    HY_LUI       = 0xfd,
    HY_LOADM     = 0xfc, // LOAD Multiply
    HY_LOADS     = 0xfb, // LOAD Shift
    HY_LOADFA    = 0xfa, // LOAD Fixed Add
    HY_LOADFS    = 0xf9, // LOAD Fixed Sub
    HY_STOREM    = 0xf8, // STORE Multiply
    HY_STORES    = 0xf7, // STORE Shift
    HY_STOREFA   = 0xf6, // STORE Fixed Add
    HY_STOREFS   = 0xf5, // STORE Fixed Sub
    HY_LEAM      = 0xf4, // LEA Multiply
    HY_LEAS      = 0xf3, // LEA Shift
    HY_LEAFA     = 0xf2, // LEA Fixed Add
    HY_LEAFS     = 0xf1, // LEA Fixed Sub
    HY_ADDR      = 0xef, // ADD Register
    HY_ADDUI8    = 0xee, // ADD Unsigned Immediate 8
    HY_ADDUI16   = 0xed, // ADD Unsigned Immediate 16
    HY_ADDSI8    = 0xec, // ADD Signed Immediate 8
    HY_ADDSI16   = 0xeb, // ADD Signed Immediate 16
    HY_SUBR      = 0xea,
    HY_SUBUI8    = 0xe9,
    HY_SUBUI16   = 0xe8,
    HY_SUBSI8    = 0xe7,
    HY_SUBSI16   = 0xe6,
    HY_MULR      = 0xe5,
    HY_MULUI8    = 0xe4,
    HY_MULUI16   = 0xe3,
    HY_MULSI8    = 0xe2,
    HY_MULSI16   = 0xe1,
    HY_DIVR      = 0xe0,
    HY_DIVUI8    = 0xdf,
    HY_DIVUI16   = 0xde,
    HY_DIVSI8    = 0xdd,
    HY_DIVSI16   = 0xdc,
    HY_CMPZ      = 0xdb, // Compare Zero
    HY_CMPR      = 0xda,
    HY_CMPI8     = 0xd9,
    HY_CMPI16    = 0xd8,
    HY_ANDR      = 0xcf,
    HY_ANDI8     = 0xce,
    HY_ANDI16    = 0xcd,
    HY_ORR       = 0xcc,
    HY_ORI8      = 0xcb,
    HY_ORI16     = 0xca,
    HY_XORR      = 0xc9,
    HY_XORI8     = 0xc8,
    HY_XORI16    = 0xc7,
    HY_NOT       = 0xc6,
    HY_NEG       = 0xc5,
    HY_SEXT      = 0xc4,
    HY_ZEXT      = 0xc3,
    HY_RSTS      = 0xc2, // RST Single
    HY_RSTM      = 0xc1, // RST Multiple
    HY_INC       = 0xc0,
    HY_DEC       = 0xbf,
    HY_TST       = 0xbe,
    HY_LSLR      = 0xbd,
    HY_LSLI16    = 0xbc,
    HY_LSRR      = 0xbb,
    HY_LSRI16    = 0xba,
    HY_ASLR      = 0xb9,
    HY_ASLI16    = 0xb8,
    HY_ASRR      = 0xb7,
    HY_ASRI16    = 0xb6,
    HY_BCCS      = 0xaf, // BCC Signed 
    HY_BCCU      = 0xae, // BCC Unsigned
    HY_JALCCI16  = 0xad, // JAL Immediate 16
    HY_JALCCM    = 0xac, // JAL Multiply
    HY_JALCCS    = 0xab, // JAL Shift
    HY_CALLCCI16 = 0xaa,
    HY_CALLCCM   = 0xa9,
    HY_CALLCCS   = 0xa8,
    HY_RTLCC     = 0xa7,
    HY_RETCC     = 0xa6,
    HY_PUSHM     = 0x9f,
    HY_POPM      = 0x9e,
    HY_PUSHS     = 0x9d,
    HY_POPS      = 0x9c,
    HY_NOP       = 0x8f
};

#define BITS(b, l) ((((hyu32_t)proc->internal.instruction >> b) & ((1 << l) - 1)))

#define CC_EQ 0
#define CC_NE 1
#define CC_CS 2
#define CC_CC 3
#define CC_MI 4
#define CC_PL 5
#define CC_VS 6
#define CC_VC 7
#define CC_HI 8
#define CC_LS 9
#define CC_GE 10
#define CC_LT 11
#define CC_GT 12
#define CC_LE 13
#define CC_AL 14

#define SET(f) (proc->internal.st & f)
#define CLEAR(f) (!SET(f))

inline bool hyrisc_test_condition(hyrisc_t* proc, int cc) {
    switch (cc) {
        case CC_EQ: { return  SET(Z); }
        case CC_NE: { return !SET(Z); }
        case CC_CS: { return  SET(C); }
        case CC_CC: { return !SET(C); }
        case CC_MI: { return  SET(N); }
        case CC_PL: { return !SET(N); }
        case CC_VS: { return  SET(V); }
        case CC_VC: { return !SET(V); }
        case CC_HI: { return SET(C) && !SET(Z); }
        case CC_LS: { return !SET(C) || SET(Z); }
        case CC_GE: { return SET(N) == SET(V); }
        case CC_LT: { return SET(N) != SET(V); }
        case CC_GT: { return !SET(Z) && (SET(N) == SET(V)); }
        case CC_LE: { return  SET(Z) && (SET(N) != SET(V)); }
        case CC_AL: { return true; }
    }

    return false;
}


#undef CC_EQ
#undef CC_NE
#undef CC_CS
#undef CC_CC
#undef CC_MI
#undef CC_PL
#undef CC_VS
#undef CC_VC
#undef CC_HI
#undef CC_LS
#undef CC_GE
#undef CC_LT
#undef CC_GT
#undef CC_LE
#undef CC_AL
#undef SET
#undef CLEAR

#define hyrisc_bus_wait if (!proc->ext.bci.busack) return false; \
\
proc->ext.bci.busack = false;

#define hyrisc_do_read(dest) dest = proc->ext.bci.d;

/*
    Encoding 4: iiiiiiii 11xxxxxy yyyyzzzz zwwwwwSS
    Encoding 3: iiiiiiii 10xxxxxy yyyyIIII IIIIssss
    Encoding 2: iiiiiiii 01xxxxxI IIIIIIII IIIIIII0
    Encoding 1: iiiiiiii 000ccccI IIIIIIII IIIIIII0 (unused, encoding 2 used instead)

    Opcode  Mnemonic        Operands         Enc Example
    0xff    mov             r0, r1           4   mov     r12, r13
    0xfe    li              r0, 0xabcd       2   li      r4, 0xdead
    0xfd    lui             r0, 0xabcd       2   lui     r5, 0xbeef
    0xfc    load.{b,s,l,x}  r0, [r1+r2*i5]   4   load.b  r3, [r6+r19*23]
    0xfb    load.{b,s,l,x}  r0, [r1+r2:i5]   4   load.b  r3, [r6+r19:23]
    0xfa    load.{b,s,l,x}  r0, [r1+i10]     4   load.b  r3, [r6+21]
    0xf9    load.{b,s,l,x}  r0, [r1-i10]     4   load.b  r3, [r6-21]
    0xf8    store.{b,s,l,x} [r1+r2*i5], r0   4   store.b [r6+r24*4], r6
    0xf7    store.{b,s,l,x} [r1+r2:i5], r0   4   store.b [r6+r24:4], r6
    0xf6    store.{b,s,l,x} [r1+i10], r0     4   store.b [r6+21], r6
    0xf5    store.{b,s,l,x} [r1-i10], r0     4   store.b [r6-21], r6
    0xf4    lea             r0, [r1+r2*i5]   4   lea     r3, [r6+r19*23]
    0xf3    lea             r0, [r1+r2:i5]   4   lea     r3, [r6+r19:23]
    0xef    addu            r0, r1, r2       4   addu    r5, r7, r12
    0xee    addu            r0, r1, i8       3   addu    r5, r7, 0xab
    0xed    addu            r0, i16          2   addu    r5, 0xdead
    0xec    adds            r0, r1, i8       3   adds    r5, r7, 0xab
    0xeb    adds            r0, i16          2   adds    r5, 0xdead
    0xea    subu            r0, r1, r2       4   subu    r5, r7, r12
    0xe9    subu            r0, r1, i8       3   subu    r5, r7, 0xab
    0xe8    subu            r0, i16          2   subu    r5, 0xdead
    0xe7    subs            r0, r1, i8       3   subs    r5, r7, 0xab
    0xe6    subs            r0, i16          2   subs    r5, 0xdead
    0xe5    mulu            r0, r1, r2       4   mulu    r5, r7, r12
    0xe4    mulu            r0, r1, i8       3   mulu    r5, r7, 0xab
    0xe3    mulu            r0, i16          2   mulu    r5, 0xdead
    0xe2    muls            r0, r1, i8       3   muls    r5, r7, 0xab
    0xe1    muls            r0, i16          2   muls    r5, 0xdead
    0xe0    divu            r0, r1, r2       4   divu    r5, r7, r12
    0xdf    divu            r0, r1, i8       3   divu    r5, r7, 0xab
    0xde    divu            r0, i16          2   divu    r5, 0xdead
    0xdd    divs            r0, r1, i8       3   divs    r5, r7, 0xab
    0xdc    divs            r0, i16          2   divs    r5, 0xdead
    0xdb    cmpz            r0               4   cmpz    r6
    0xda    cmp.{b,s,l}     r0, r1           4   cmp.l   r6, r7
    0xd9    cmp.b           r0, i8           3   cmp.b   r6, 0x9
    0xd8    cmp.s           r0, i16          2   cmp.s   r6, 0xdead
    0xcf    and             r0, r1, r2       4   and     r5, r7, r12
    0xce    and             r0, r1, i8       3   and     r5, r7, 0xab
    0xcd    and             r0, i16          2   and     5, 0xdead
    0xcc    or              r0, r1, r2       4   or      r5, r7, r12
    0xcb    or              r0, r1, i8       3   or      r5, r7, 0xab
    0xca    or              r0, i16          2   or      5, 0xdead
    0xc9    xor             r0, r1, r2       4   xor     r5, r7, r12
    0xc8    xor             r0, r1, i8       3   xor     r5, r7, 0xab
    0xc7    xor             r0, i16          2   xor     5, 0xdead
    0xc6    not             r0, r1           4   not     r5, r7
    0xc5    neg             r0, r1           4   neg     r5, r7
    0xc4    sext.{b,s,l}    r0, r1           4   sext.s  r6, r25
    0xc3    zext.{b,s,l}    r0, r1           4   zext.b  r6, r25
    0xc2    rst.{b,s,l}     r0               4   rst.l   r9
    0xc1    rst.{b,s,l}     {r0-r1}          4   rst.l   {r9, r21}
    0xc0    inc.{b,s,l,d}   r0               4   inc.d   r7
    0xbf    dec.{b,s,l,d}   r0               4   dec.b   r9
    0xbe    tst             r0, i5           4   tst     r11, 31
    0xbd    lsl             r0, r1, r2       4   lsl     r5, r7, r12
    0xbc    lsl             r0, i16          2   lsl     r5, 0xdead
    0xbb    lsr             r0, r1, r2       4   lsr     r5, r7, r12
    0xba    lsr             r0, i16          2   lsr     r5, 0xdead
    0xb9    asl             r0, r1, r2       4   asl     r5, r7, r12
    0xb8    asl             r0, i16          2   asl     r5, 0xdead
    0xb7    asr             r0, r1, r2       4   asr     r5, r7, r12
    0xb6    asr             r0, i16          2   asr     r5, 0xdead
    0xaf    bcc.s           i16              2   beq.s   0x1000
    0xae    bcc.u           i16              2   bgt.u   0x2000
    0xad    jalcc           i16              2   jaleq   0x3000
    0xac    jalcc           [r0+r1*i5]       4   jalne   [r5+r1*4]
    0xab    jalcc           [r0+r1:i5]       4   jalgt   [r6+r15:8]
    0xaa    callcc          i16              2   callhi  0x4000
    0xa9    callcc          [r0+r1*i5]       4   calleq  [r5+r1*4]
    0xa8    callcc          [r0+r1:i5]       4   callne  [r6+r15:8]
    0xa7    rtlcc                            4   rtlgt
    0xa6    retcc                            4   retne
    0x9f    push            {r0-r1}          4   push    {r6, r6}
    0x9e    pop             {r0-r1}          4   pop     {r6, r6}
    0x9d    push            r0               4   push    r7
    0x9c    pop             r0               4   pop     r21
    0x8f    nop                              4   nop
*/

void hyrisc_decode(hyrisc_t* proc) {
    std::memset(&proc->internal.decoder, 0, sizeof(proc->internal.decoder));

    proc->internal.decoder.opcode = BITS(0, 8);
    proc->internal.decoder.encoding = BITS(8, 2);

    switch (proc->internal.decoder.encoding) {
        case 3: {
            proc->internal.decoder.fieldx = BITS(10, 5);
            proc->internal.decoder.fieldy = BITS(15, 5);
            proc->internal.decoder.fieldz = BITS(20, 5);
            proc->internal.decoder.fieldw = BITS(25, 5);
            proc->internal.decoder.size   = BITS(30, 2);
        } break;

        case 2: {
            proc->internal.decoder.fieldx = BITS(10, 5);
            proc->internal.decoder.fieldy = BITS(15, 5);
            proc->internal.decoder.imm8   = BITS(20, 8);
        } break;

        case 1: {
            proc->internal.decoder.fieldx = BITS(10, 5);
            proc->internal.decoder.imm16  = BITS(15, 16);
        } break;

        // The instruction set is so coarse we don't even need encoding 0
        // for branch instructions we can use either encoding 3 or 1
        // ex.
        // beq r0+%r1:2
        // uses encoding 3, with fieldx storing condition, and fieldy
        // through fieldw storing registers and shift_mul
        // beq 0xabcd
        // uses encoding 1, with fieldx storing condition, and
        // imm16 storing the relative immediate
        // case 0: {
        //     proc->internal.cond   = BITS(10, 5);
        // }
        // for tst we can use any encoding, the second operand 
        // doesn't require more than 5 bits
        // tst r0, 0x9
    }

    // _log(debug, "instruction=%08x, fx=%02x, fy=%02x, fz=%02x, fw=%02x, i16=%04x, i8=%02x, size=%u, encoding=%u, opcode=%02x",
    //     proc->internal.instruction,
    //     proc->internal.decoder.fieldx,
    //     proc->internal.decoder.fieldy,
    //     proc->internal.decoder.fieldz,
    //     proc->internal.decoder.fieldw,
    //     proc->internal.decoder.imm16,
    //     proc->internal.decoder.imm8,
    //     proc->internal.decoder.size,
    //     proc->internal.decoder.encoding,
    //     proc->internal.decoder.opcode
    // );
}

#define REGX proc->internal.r[proc->internal.decoder.fieldx]
#define REGY proc->internal.r[proc->internal.decoder.fieldy]
#define REGZ proc->internal.r[proc->internal.decoder.fieldz]
#define REGW proc->internal.r[proc->internal.decoder.fieldw]
#define I5X  (proc->internal.decoder.fieldx)
#define I5Y  (proc->internal.decoder.fieldy)
#define I5Z  (proc->internal.decoder.fieldz)
#define I5W  (proc->internal.decoder.fieldw)
#define I8   (proc->internal.decoder.imm8)
#define I16  (proc->internal.decoder.imm16)
#define SIZE (proc->internal.decoder.size)
#define COND (proc->internal.decoder.fieldx)
#define I10              (I5Z | (I5W << 5))
#define INDEXED_MULTIPLY (REGY + (REGZ * I5W))
#define INDEXED_SHIFT    (REGY + (REGZ << I5W))

bool hyrisc_execute(hyrisc_t* proc, hyint_t cycle) {
    switch (proc->internal.decoder.opcode) {
        case HY_MOV: {
            REGX = REGY;

            return true;
        } break;

        case HY_LI: {
            REGX = I16;

            return true;
        } break;

        case HY_LUI: {
            REGX = I16 << 16;

            return true;
        } break;

        case HY_LOADM: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_read(proc, INDEXED_MULTIPLY, SIZE);
                    
                    return false;
                } break;

                case 1: {
                    hyrisc_do_read(REGX);

                    return true;
                } break;
            }
        } break;
        
        case HY_LOADS: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_read(proc, INDEXED_SHIFT, SIZE);
                    
                    return false;
                } break;

                case 1: {
                    hyrisc_do_read(REGX);

                    return true;
                } break;
            }
        } break;

        case HY_LOADFA: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_read(proc, REGY + I10, SIZE);

                    return false;
                } break;

                case 1: {
                    hyrisc_do_read(REGX);

                    return true;
                } break;
            }
        } break;

        case HY_LOADFS: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_read(proc, REGY - I10, SIZE);

                    return false;
                } break;

                case 1: {
                    hyrisc_do_read(REGX);

                    return true;
                } break;
            }
        } break;

        case HY_STOREM: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_write(proc, INDEXED_MULTIPLY, REGX, SIZE);

                    return false;
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;

        case HY_STORES: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_write(proc, INDEXED_SHIFT, REGX, SIZE);

                    return false;
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;

        case HY_STOREFA: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_write(proc, REGY + I10, REGX, SIZE);

                    return false;
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;
        
        case HY_STOREFS: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_write(proc, REGY - I10, REGX, SIZE);

                    return false;
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;

        case HY_LEAM: {
            REGX = INDEXED_MULTIPLY;

            return true;
        } break;

        case HY_LEAS: {
            REGX = INDEXED_SHIFT;

            return true;
        } break;

        case HY_LEAFA: {
            REGX = REGY + I10;

            return true;
        } break;
        
        case HY_LEAFS: {
            REGX = REGY - I10;

            return true;
        } break;

        case HY_ADDR   : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_addu); return true; } break;
        case HY_ADDUI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_addu); return true; } break;
        case HY_ADDUI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_addu); return true; } break;
        case HY_ADDSI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_adds); return true; } break;
        case HY_ADDSI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_adds); return true; } break;
        case HY_SUBR   : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_subu); return true; } break;
        case HY_SUBUI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_subu); return true; } break;
        case HY_SUBUI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_subu); return true; } break;
        case HY_SUBSI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_subs); return true; } break;
        case HY_SUBSI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_subs); return true; } break;
        case HY_MULR   : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_mulu); return true; } break;
        case HY_MULUI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_mulu); return true; } break;
        case HY_MULUI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_mulu); return true; } break;
        case HY_MULSI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_muls); return true; } break;
        case HY_MULSI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_muls); return true; } break;
        case HY_DIVR   : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_divu); return true; } break;
        case HY_DIVUI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_divu); return true; } break;
        case HY_DIVUI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_divu); return true; } break;
        case HY_DIVSI8 : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_divs); return true; } break;
        case HY_DIVSI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_divs); return true; } break;
        case HY_CMPZ   : { alu::perform_operation(proc, REGX, 0   , 0   , alu::HY_cmp ); return true; } break;
        case HY_CMPR   : { alu::perform_operation(proc, REGX, REGY, 0   , alu::HY_cmp ); return true; } break;
        case HY_CMPI8  : { alu::perform_operation(proc, REGX, I16 , 0   , alu::HY_cmpb); return true; } break;
        case HY_CMPI16 : { alu::perform_operation(proc, REGX, I16 , 0   , alu::HY_cmp ); return true; } break;
        case HY_ANDR   : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_and ); return true; } break;
        case HY_ANDI8  : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_and ); return true; } break;
        case HY_ANDI16 : { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_and ); return true; } break;
        case HY_ORR    : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_or  ); return true; } break;
        case HY_ORI8   : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_or  ); return true; } break;
        case HY_ORI16  : { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_or  ); return true; } break;
        case HY_XORR   : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_xor ); return true; } break;
        case HY_XORI8  : { alu::perform_operation(proc, REGX, REGY, I8  , alu::HY_xor ); return true; } break;
        case HY_XORI16 : { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_xor ); return true; } break;
        case HY_NOT    : { alu::perform_operation(proc, REGX, REGY, 0   , alu::HY_not ); return true; } break;
        case HY_NEG    : { alu::perform_operation(proc, REGX, REGY, 0   , alu::HY_neg ); return true; } break;
        
        case HY_SEXT: {
            uint32_t b = (1ull << (2 << (2 + SIZE))) >> 1;
            uint32_t mask = 0xffffffff - ((b - 1) | b);

            REGX = mask | REGY;

            return true;
        } break;

        case HY_RSTS: {
            REGX = 0;

            return true;
        } break;

        // This instruction is part of the I extension
        case HY_RSTM: {
            for (int i = I5X; i < I5Y; i++) {
                proc->internal.r[i] = 0;
            }

            return true;
        } break;

        case HY_INC: { alu::perform_operation(proc, REGX, 1 << SIZE, 0, alu::HY_inc); return true; } break;
        case HY_DEC: { alu::perform_operation(proc, REGX, 1 << SIZE, 0, alu::HY_dec); return true; } break;

        case HY_TST: { alu::perform_operation(proc, REGX, I5Y, 0, alu::HY_tst); return true; } break;
        
        case HY_LSLR  : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_lsl); return true; } break;
        case HY_LSLI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_lsl); return true; } break;
        case HY_LSRR  : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_lsr); return true; } break;
        case HY_LSRI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_lsr); return true; } break;
        case HY_ASLR  : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_asl); return true; } break;
        case HY_ASLI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_asl); return true; } break;
        case HY_ASRR  : { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_asr); return true; } break;
        case HY_ASRI16: { alu::perform_operation(proc, REGX, REGX, I16 , alu::HY_asr); return true; } break;
        
        case HY_BCCS: { if (hyrisc_test_condition(proc, COND)) proc->internal.r[pc] += ( int32_t)( int16_t)I16; return true; } break;
        case HY_BCCU: { if (hyrisc_test_condition(proc, COND)) proc->internal.r[pc] += (uint32_t)          I16; return true; } break;
        
        case HY_JALCCI16: {
            if (hyrisc_test_condition(proc, COND)) {
                proc->internal.r[pc] &= 0xffff0000;
                proc->internal.r[pc] |= I16;
            }

            return true;
        } break;

        case HY_JALCCM: {
            if (hyrisc_test_condition(proc, COND)) {
                proc->internal.r[lr] = proc->internal.r[pc];
                proc->internal.r[pc] = INDEXED_MULTIPLY;
            }

            return true;
        } break;

        case HY_JALCCS: {
            if (hyrisc_test_condition(proc, COND)) {
                proc->internal.r[lr] = proc->internal.r[pc];
                proc->internal.r[pc] = INDEXED_SHIFT;
            }

            return true;
        } break;

        case HY_CALLCCI16: {
            switch (cycle) {
                case 0: {
                    if (hyrisc_test_condition(proc, COND)) {
                        proc->internal.r[sp] -= 4;

                        hyrisc_init_write(proc, proc->internal.r[sp], proc->internal.r[pc], AS_LONG);
                        
                        proc->internal.r[pc] &= 0xffff0000;
                        proc->internal.r[pc] |= I16;

                        return false;
                    } else {
                        return true;
                    }
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;

        case HY_CALLCCM: {
            switch (cycle) {
                case 0: {
                    if (hyrisc_test_condition(proc, COND)) {
                        proc->internal.r[sp] -= 4;

                        hyrisc_init_write(proc, proc->internal.r[sp], proc->internal.r[pc], AS_LONG);
                        
                        proc->internal.r[pc] = INDEXED_MULTIPLY;

                        return false;
                    } else {
                        return true;
                    }
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;

        case HY_CALLCCS: {
            switch (cycle) {
                case 0: {
                    if (hyrisc_test_condition(proc, COND)) {
                        proc->internal.r[sp] -= 4;

                        hyrisc_init_write(proc, proc->internal.r[sp], proc->internal.r[pc], AS_LONG);
                        
                        proc->internal.r[pc] = INDEXED_SHIFT;

                        return false;
                    } else {
                        return true;
                    }
                } break;

                case 1: {
                    hyrisc_bus_wait;

                    return true;
                } break;
            }
        } break;

        case HY_RTLCC: {
            if (hyrisc_test_condition(proc, COND)) {
                proc->internal.r[pc] = proc->internal.r[lr];
            }

            return true;
        } break;

        case HY_RETCC: {
            switch (cycle) {
                case 0: {
                    if (hyrisc_test_condition(proc, COND)) {
                        hyrisc_init_read(proc, proc->internal.r[sp], AS_LONG);

                        proc->internal.r[sp] += 4;

                        return false;
                    } else {
                        return true;
                    }
                } break;

                case 1: {
                    hyrisc_do_read(proc->internal.r[pc]);

                    return true;
                } break;
            }

            return true;
        } break;

        // PUSHM and POPM are part of the I extension
        // and have not been implemented yet

        case HY_PUSHS: {
            switch (cycle) {
                case 0: {
                    proc->internal.r[sp] -= 4;

                    hyrisc_init_write(proc, proc->internal.r[sp], REGX, AS_LONG);

                    return false;
                } break;

                case 1: {
                    hyrisc_bus_wait;
                } break;
            }
        } break;

        case HY_POPS: {
            switch (cycle) {
                case 0: {
                    hyrisc_init_read(proc, proc->internal.r[sp], AS_LONG);

                    proc->internal.r[sp] += 4;

                    return false;
                } break;

                case 1: {
                    hyrisc_do_read(REGX);
                } break;
            }
        } break;

        case HY_NOP: {
            return true;
        } break;

        // Debug instruction!
        // Break into host
        case 0x45: {
#ifdef _WIN32
            std::raise(SIGBREAK);
#else
            std::raise(SIGINT);
#endif
        } break;

        // Any other instructions are considered illegal
        // Emulator will raise SIGILL
        default: {
            std::raise(SIGILL);
        } break;
    }

    return true;
}

#undef BITS

void hyrisc_pulse_reset(hyrisc_t* proc, hyu32_t vec) {
    proc->ext.reset = true;
    proc->ext.pic.v = vec;

    hyrisc_clock(proc);

    proc->ext.reset = false;
}