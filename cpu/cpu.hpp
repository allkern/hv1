#pragma once

#include <cstdint>
#include <array>
#include <cstring>

#include "state.hpp"

#include <iostream>

#define SIG(sig, trigger) hysignal_get(sig, hysignal_t::trigger)

enum hyrisc_register_names_t {
    r0  , gp0 , gp1 , gp2 ,
    gp3 , gp4 , gp5 , gp6 ,
    gp7 , gp8 , gp9 , gp10,
    gp11, gp12, gp13, gp14,
    a0  , a1  , a2  , rr0 ,
    a3  , a4  , a5  , rr1 ,
    lr0 , lr1 , lr2 , lr3 ,
    lr  , br  , sp  , pc
};

void hyrisc_bci_update(hyrisc_t* proc) {
    // If there was a bus error on last clock, then 
    if (proc->ext.bci.be && hysignal_get(&proc->ext.bci.busirq, hysignal_t::HIGH)) {
        // If Bus error IRQ has already been acknowledged, then
        // clear bus error and lower IRQ
        if (hysignal_get(&proc->ext.pic.irqack)) {
            proc->ext.bci.be = 0x0;

            hysignal_set(&proc->ext.pic.irqack, false);
            hysignal_set(&proc->ext.pic.irq, false);

            return;
        }

        hysignal_set(&proc->ext.pic.irqack, false);
        hysignal_set(&proc->ext.pic.irq, true);

        // 256 bytes to handle each bus error mapped on f0000000-f000ffff
        proc->ext.pic.v = 0xf0000000 | (proc->ext.bci.be << 8);
    }
}

void hyrisc_reset(hyrisc_t* proc) {
    std::memset(&proc->internal, 0, sizeof(proc->internal));

    proc->internal.instruction = 0xffffffff;
}

void hyrisc_handle_signals(hyrisc_t* proc) {
    // Get signals
    bool reset  = hysignal_get(&proc->ext.reset, hysignal_t::HIGH);
    bool freeze = hysignal_get(&proc->ext.freeze, hysignal_t::HIGH);
    bool irq    = hysignal_get(&proc->ext.pic.irq, hysignal_t::RISING);
    
    // If RESET is high, then reset the CPU
    if (reset) {
        hyrisc_reset(proc);

        return;
    }

    // If FREEZE is high, then idle
    if (freeze) {
        return;
    }

    // If IRQ is rising, then jump to the vector on V0-V31
    if (irq) {
        proc->internal.r[pc] = proc->ext.pic.v;

        hysignal_set(&proc->ext.pic.irqack, true);

        // Handling an IRQ takes 1 input clock
        return;
    }
}

enum hyrisc_access_size_t {
    ACS_BYTE,
    ACS_HALF,
    ACS_WORD
};

void hyrisc_init_read(hyrisc_t* proc, hyu32_t addr, hyint_t size = ACS_WORD) {
    proc->ext.bci.a = addr;
    proc->ext.bci.s = size;

    hysignal_set(&proc->ext.bci.rw, false);

    proc->ext.bci.be = 0x0;
}

void hyrisc_init_write(hyrisc_t* proc, hyu32_t addr, hyu32_t value, hyint_t size = ACS_WORD) {
    proc->ext.bci.a = addr;
    proc->ext.bci.s = size;
    proc->ext.bci.d = value;

    hysignal_set(&proc->ext.bci.rw, true);

    proc->ext.bci.be = 0x0;
}

bool hyrisc_execute(hyrisc_t* proc, hyint_t cycle);

void hyrisc_init_clock(hyrisc_t* proc, hysignal_t* clock) {
    proc->ext.clk = clock;
}

void hyrisc_clock(hyrisc_t* proc) {
    // Processor is clocked on every input clock edge
    bool clock_input = SIG(proc->ext.clk, EDGE);

    if (clock_input && (proc->ext.vcc >= 0.5f)) {
        // Update BCI
        hyrisc_bci_update(proc);

        // Handle signals
        hyrisc_handle_signals(proc);

        switch (proc->internal.cycle) {
            case 0x0: {
                hyrisc_init_read(proc, proc->internal.r[pc], 4);

                // Set address bus to PC
                proc->ext.bci.a = proc->internal.r[pc];

                // Set RW pin to read
                hysignal_set(&proc->ext.bci.rw, false);

                // Clear bus error
                proc->ext.bci.be = 0x0;

                proc->internal.cycle++;
            } break;

            case 0x1: {
                bool busack = hysignal_get(&proc->ext.bci.busack, hysignal_t::HIGH);

                // Idle until BUSACK becomes high
                if (!busack) return;

                // Copy the contents of the data bus to
                // the instruction latch for decoding
                proc->internal.instruction = proc->ext.bci.d;

                proc->internal.cycle++;
            } break;

            case 0x2: {
                bool done = hyrisc_execute(proc, proc->internal.cycle - 0x2);

                if (done) {
                    proc->internal.cycle = 0;
                } else {
                    // Instruction needs to wait for I/O
                    proc->internal.cycle++;
                }
            } break;

            case 0x3: {
                // Instruction has to finish processing I/O on cycle 4
                if (!hyrisc_execute(proc, proc->internal.cycle - 0x2)) return;

                proc->internal.cycle = 0;
            } break;
        }
    }

    // If no clock input or not enough voltage then idle
    return;
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

    enum opcodes {
        HY_LOAD  = 0xff,
        HY_STORE = 0xfe,
        HY_ADD   = 0xef,
        HY_SUB   = 0xee,
        HY_MUL   = 0xed,
        HY_DIV   = 0xec,
        HY_AND   = 0xdf,
        HY_OR    = 0xde,
        HY_XOR   = 0xdd,
        HY_NOT   = 0xdc,
        HY_NEG   = 0xdb,
        HY_INC   = 0xcf,
        HY_DEC   = 0xce,
        HY_RST   = 0xcd,
        HY_TST   = 0xcc,
        HY_CMP   = 0xcb,
        HY_SLL   = 0xbf,
        HY_SLR   = 0xbe,
        HY_SAL   = 0xbd,
        HY_SAR   = 0xbc,
        HY_RL    = 0xbb,
        HY_RR    = 0xba,
        HY_JC    = 0xaf,
        HY_JP    = 0xae,
        HY_BC    = 0xad,
        HY_JR    = 0xac,
        HY_JALC  = 0xab,
        HY_JPL   = 0xaa,
        HY_CALLC = 0x9f,
        HY_CALL  = 0x9e,
        HY_RETC  = 0x9d,
        HY_RET   = 0x9c,
        HY_RTL   = 0x9b,
        HY_PUSH  = 0x8f,
        HY_POP   = 0x8e,
        HY_NOP   = 0x7f
    };

    /*
Load, Store
l    : ff
s    : fe

Add, Subtract, Multiply
add(u|s): ef
sub(u|s): ee
mul(u|s): ed
div(u|s): ec

AND, OR, XOR, NOT, NEG
and  : df
or   : de
xor  : dd
not  : dc
neg  : db

Increment, Decrement, Reset, Test, Compare
i    : cf
d    : ce
rst  : cd
tst  : cc
cmp  : cb

Shifts, Rotates
sll  : bf
slr  : be
sal  : bd
sar  : bc
rl   : bb
rr   : ba

Jumps, Branches (Basic Flow Control)
jc   : af
jp   : ae
bc   : ad
br   : ac
jalc : ab
jpl  : aa

Calls, Returns (Advanced Flow Control)
callc: 9f
call : 9e
retc : 9d
ret  : 9c

Push, Pop (Stack Manipulation)
push : 8f
pop  : 8e

Misc:
nop  : 7f
    */

#define Z 0b00000001
#define N 0b00000010
#define V 0b00000100
#define C 0b00001000

#define BITS(b, l) ((((hyu32_t)proc->internal.instruction >> b) & ((1 << l) - 1)))

inline static void hyrisc_set_flags(hyrisc_t* proc, hyu8_t mask, bool cond, bool reset = true) {
    if (cond) {
        proc->internal.st |= mask;
        return;
    }

    if (reset) proc->internal.st &= ~mask;
}

namespace alu {
    typedef hyu64_t (*operation_t)(hyu32_t&, hyu32_t, hyu32_t);

#define OPERATION(name, code) \
operation_t HY_##name = [](hyu32_t& dst, hyu32_t src1, hyu32_t src2) -> hyu64_t { hyu64_t temp = code; return temp; }

    OPERATION(addu, src1 + src2     ; dst = temp & 0xffffffff);
    OPERATION(subu, src1 - src2     ; dst = temp & 0xffffffff);
    OPERATION(mulu, src1 * src2     ; dst = temp & 0xffffffff);
    OPERATION(divu, src1 / src2     ; dst = temp & 0xffffffff);
    OPERATION(adds, src1 + (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(subs, src1 - (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(muls, src1 * (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(divs, src1 / (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(and, src1 & src2      ; dst = temp & 0xffffffff);
    OPERATION(or , src1 | src2      ; dst = temp & 0xffffffff);
    OPERATION(xor, src1 ^ src2      ; dst = temp & 0xffffffff);
    OPERATION(not, ~dst             ; dst = temp & 0xffffffff);
    OPERATION(neg, ~dst             ; dst = temp & 0xffffffff);
    OPERATION(i  , dst + 1          ; dst = temp & 0xffffffff);
    OPERATION(d  , dst - 1          ; dst = temp & 0xffffffff);
    OPERATION(rst, 0                ; dst = temp & 0xffffffff);
    OPERATION(tst, dst & (1 << src1);                        );
    OPERATION(cmp, dst - src1       ;                        );
    OPERATION(sll, src1 << src2     ; dst = temp & 0xffffffff);
    OPERATION(slr, src1 >> src2     ; dst = temp & 0xffffffff);
    OPERATION(sal, src1 << src2     ; dst = temp & 0xffffffff);
    OPERATION(sar, src1 >> src2     ; dst = temp & 0xffffffff);
    OPERATION(rl , src1 << src2     ; dst = temp & 0xffffffff);
    OPERATION(rr , src1 >> src2     ; dst = temp & 0xffffffff);

    void perform_operation(hyrisc_t* proc, hyu32_t& dst, hyu32_t src1, hyu32_t src2, operation_t op) {
        hyu32_t old = dst;

        hyu64_t temp = op(dst, src1, src2);

        hyrisc_set_flags(proc, Z, !(temp & 0xffffffff));
        hyrisc_set_flags(proc, N, (temp & 0xffffffff) & 0x80000000);
        hyrisc_set_flags(proc, C, temp > 0xffffffff);

        // jne -> Z clear
        // jeq -> Z set
        // jgt -> N clear
        // jlt -> N set
        // jzs -> Z set 
        // jzc -> Z clear
    }
}

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

#undef Z
#undef N
#undef V
#undef C
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

// lui   %r0,   #f;          ; load upper word with 000f (opcode 000f05ff)
// or    %r0,   #ffff;       ; OR in the lower component (opcode ffff05de)
// addu  %r0,   %r0,    #1;  ; Add 1                     (opcode 002006ef)

#define REGX (proc->internal.r[BITS(11, 5)])
#define REGY (proc->internal.r[BITS(16, 5)])
#define REGZ (proc->internal.r[BITS(21, 5)])
#define REGW (proc->internal.r[BITS(26, 5)])
#define IMM51  (BITS(16, 5))
#define IMM81  (BITS(16, 8))
#define IMM82  (BITS(21, 8))
#define IMM16  (BITS(16, 16))
#define COND   (BITS(11, 4))

#define hyrisc_bus_wait \
bool busack = hysignal_get(&proc->ext.bci.busack, hysignal_t::HIGH); \
\
if (!busack) return false; \
\
hysignal_set(&proc->ext.bci.busack, hysignal_t::LOW);

#define hyrisc_do_read(dest) \
hyrisc_bus_wait \
\
dest = proc->ext.bci.d;

// Binary:
// ff 05 0f 00 de 05 ff ff ef 06 20 00
bool hyrisc_execute(hyrisc_t* proc, hyint_t cycle) {
    switch (proc->internal.instruction & 0xff) {
        case HY_LOAD: {
            /*  register to register:
                iiiiiiii 111xxxxx yyyyy000 00000000
                00000000 000yyyyy xxxxxmmm iiiiiiii

                register to immediate (shift 16):
                iiiiiiii 110xxxxx iiiiiiii iiiiiiii
                00000000 000yyyyy xxxxxmmm iiiiiiii
            */

            int size = BITS(21, 2);

            switch (BITS(8, 3)) {
                case 7: { REGX = REGY; return true; } break; // l %r0, %r1;
                case 6: { REGX = IMM16; return true; } break; // li %r0, #ffff;
                case 5: { REGX = IMM16 << 16; return true; } break; // lui %r0, #ffff;
                case 4: { // l %r0, %r1+(%r2:2)
                    switch (cycle) {
                        case 0: hyrisc_init_read(proc, REGY + (REGZ << BITS(26, 4)), size); return false;
                        case 1: hyrisc_do_read(REGX); return true;
                    }
                } break;
                case 3: { // l %r0, %r1+(%r2*2)
                    switch (cycle) {
                        case 0: hyrisc_init_read(proc, REGY + (REGZ * BITS(26, 4)), size); return false;
                        case 1: hyrisc_do_read(REGX); return true;
                    }
                } break;
            }
        } break;

        case HY_STORE: {
            /*  sized register to *register: sw %r0, %r1;
                iiiiiiii 111xxxxx yyyyySS0 00000000
                00000000 00000000 11111111 11111111
                01234567 89abcdef 01234567 89abcdef

                sized register to *register with shifted index register: sw %r0, %r1(%r2:2);
                iiiiiiii 110xxxxx yyyyyzzz zzssssSS
                01234567 89abcdef 01234567 89abcdef
                
                sized register to *register with scaled index register: sw %r0, %r1(%r2*4); sw %r0, %r1(%r2);
                iiiiiiii 101xxxxx yyyyyzzz zzssssSS

                16-bit immediate to *register: stw #abcd, %r0;
                iiiiiiii 100xxxxx iiiiiiii iiiiiiii
                01234567 89abcdef 01234567 89abcdef

                8-bit immediate to *register: sh #ab, %r0;
                iiiiiiii 011xxxxx iiiiiiii 00000000
            */
            hyint_t size = BITS(21, 2);
            hyu32_t mask = (1ull << ((size + 1) << 3)) - 1;

            switch (BITS(8, 3)) {
                case 7: {
                    switch (cycle) {
                        case 0: hyrisc_init_write(proc, REGX, REGY & mask, size); return false;
                        case 1: hyrisc_bus_wait; return true;
                    }
                } break;
                case 6: { // l %r0, %r1+(%r2:2)
                    switch (cycle) {
                        case 0: hyrisc_init_write(proc, REGY + (REGZ << BITS(26, 4)), REGX, size); return false;
                        case 1: hyrisc_bus_wait; return true;
                    }
                } break;
                case 5: { // l %r0, %r1+(%r2*2)
                    switch (cycle) {
                        case 0: hyrisc_init_write(proc, REGY + (REGZ * BITS(26, 4)), REGX, size); return false;
                        case 1: hyrisc_bus_wait; return true;
                    }
                } break;
                case 4: {
                    switch (cycle) {
                        case 0: hyrisc_init_write(proc, REGX, IMM16 & mask, 1); return false;
                        case 1: hyrisc_bus_wait; return true;
                    }
                } break;
                case 3: {
                    switch (cycle) {
                        case 0: hyrisc_init_write(proc, REGX, IMM81 & mask, 0); return false;
                        case 1: hyrisc_bus_wait; return true;
                    }
                } break;
            }
        } break;

        case HY_ADD: { 
            /*  Add register y with register z, store in register x: add(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz------
                01234567 89abcdef 01234567 89abcdef

                Add register y with unsigned 8-bit immediate, store in register x: add(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii111
                01234567 89abcdef 01234567 89abcdef

                Add register x with unsigned 16-bit immediate, store in register x; add(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
                
                Add register y with signed 8-bit immediate, store in register x: adds %r0, %r0, #-2;
                iiiiiiii 100xxxxx yyyyyiii iiiii---
                01234567 89abcdef 01234567 89abcdef

                Add register x with signed 16-bit immediate, store in register x; adds %r0, #-1000;
                iiiiiiii 011xxxxx iiiiiiii iiiiiiii
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ, alu::HY_addu); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_addu); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_addu); } break;
                case 4: { alu::perform_operation(proc, REGX, REGY, (hyi32_t)IMM82, alu::HY_adds); } break;
                case 3: { alu::perform_operation(proc, REGX, REGX, (hyi32_t)IMM16, alu::HY_adds); } break;
            }
        } break;
        
        case HY_SUB: { 
            /*  Subtract register y with register z, store in register x: sub(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef

                Subtract register y with unsigned 8-bit immediate, store in register x: sub(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Subtract register x with unsigned 16-bit immediate, store in register x; sub(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
                
                Subtract register y with signed 8-bit immediate, store in register x: subs %r0, %r0, #-2;
                iiiiiiii 100xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Subtract register x with signed 16-bit immediate, store in register x; subs %r0, #-1000;
                iiiiiiii 011xxxxx iiiiiiii iiiiiiii
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ , alu::HY_subu); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_subu); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_subu); } break;
                case 4: { alu::perform_operation(proc, REGX, REGY, (hyi32_t)IMM82, alu::HY_subs); } break;
                case 3: { alu::perform_operation(proc, REGX, REGX, (hyi32_t)IMM16, alu::HY_subs); } break;
            }
        } break;

        case HY_MUL: { 
            /*  Add register y with register z, store in register x: add(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef

                Add register y with unsigned 8-bit immediate, store in register x: add(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with unsigned 16-bit immediate, store in register x; add(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
                01234567 89abcdef 01234567 89abcdef

                Add register y with signed 8-bit immediate, store in register x: adds %r0, %r0, #-2;
                iiiiiiii 100xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with signed 16-bit immediate, store in register x; adds %r0, #-1000;
                iiiiiiii 011xxxxx iiiiiiii iiiiiiii
                01234567 89abcdef 01234567 89abcdef
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ , alu::HY_mulu); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_mulu); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_mulu); } break;
                case 4: { alu::perform_operation(proc, REGX, REGY, (hyi32_t)IMM82, alu::HY_muls); } break;
                case 3: { alu::perform_operation(proc, REGX, REGX, (hyi32_t)IMM16, alu::HY_muls); } break;
            }
        } break;

        case HY_DIV: { 
            /*  Add register y with register z, store in register x: add(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef

                Add register y with unsigned 8-bit immediate, store in register x: add(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with unsigned 16-bit immediate, store in register x; add(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
                
                Add register y with signed 8-bit immediate, store in register x: adds %r0, %r0, #-2;
                iiiiiiii 100xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with signed 16-bit immediate, store in register x; adds %r0, #-1000;
                iiiiiiii 011xxxxx iiiiiiii iiiiiiii
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ , alu::HY_divu); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_divu); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_divu); } break;
                case 4: { alu::perform_operation(proc, REGX, REGY, (hyi32_t)IMM82, alu::HY_divs); } break;
                case 3: { alu::perform_operation(proc, REGX, REGX, (hyi32_t)IMM16, alu::HY_divs); } break;
            }
        } break;

        case HY_AND: { 
            /*  Add register y with register z, store in register x: add(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef

                Add register y with unsigned 8-bit immediate, store in register x: add(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with unsigned 16-bit immediate, store in register x; add(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ , alu::HY_and); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_and); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_and); } break;
            }
        } break;

        case HY_OR: { 
            /*  Add register y with register z, store in register x: add(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef

                Add register y with unsigned 8-bit immediate, store in register x: add(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with unsigned 16-bit immediate, store in register x; add(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ , alu::HY_or); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_or); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_or); } break;
            }
        } break;

        case HY_XOR: { 
            /*  Add register y with register z, store in register x: add(u) %r0, %r0, %r1
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef

                Add register y with unsigned 8-bit immediate, store in register x: add(u) %r0, %r0, #ff;
                iiiiiiii 110xxxxx yyyyyiii iiiii000
                01234567 89abcdef 01234567 89abcdef

                Add register x with unsigned 16-bit immediate, store in register x; add(u) %r0, #ffff;
                iiiiiiii 101xxxxx iiiiiiii iiiiiiii
            */

            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, REGY, REGZ , alu::HY_xor); } break;
                case 6: { alu::perform_operation(proc, REGX, REGY, IMM82, alu::HY_xor); } break;
                case 5: { alu::perform_operation(proc, REGX, REGX, IMM16, alu::HY_xor); } break;
            }
        } break;

        case HY_NOT: {
            /*  Invert register y's bits, store in register x:
                iiiiiiii 111xxxxx yyyyy000 00000000
                01234567 89abcdef 01234567 89abcdef     
            */
            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, 0, 0, alu::HY_not); } break;
            }
        } break;

        case HY_NEG: {
            /*  Invert register y's bits, store in register x:
                iiiiiiii 111xxxxx yyyyy000 00000000
                01234567 89abcdef 01234567 89abcdef     
            */
            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, 0, 0, alu::HY_neg); } break;
            }
        } break;

        case HY_INC: {
            /*  Increment register x
                iiiiiiii 111xxxxx 00000000 00000000
                01234567 89abcdef 01234567 89abcdef     
            */
            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, 0, 0, alu::HY_i); } break;
            }
        } break;

        case HY_DEC: {
            /*  Decrement register x
                iiiiiiii 111xxxxx 00000000 00000000
                01234567 89abcdef 01234567 89abcdef     
            */
            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, 0, 0, alu::HY_d); } break;
            }
        } break;

        case HY_RST: {
            /*  Reset register x
                iiiiiiii 111xxxxx 00000000 00000000
                01234567 89abcdef 01234567 89abcdef
            
                Reset register x and register y
                iiiiiiii 111xxxxx yyyyy000 00000000
                01234567 89abcdef 01234567 89abcdef
                
                Reset register x and register y and register z
                iiiiiiii 111xxxxx yyyyyzzz zz000000
                01234567 89abcdef 01234567 89abcdef
                
                Reset register x and register y and register z and register w
                iiiiiiii 111xxxxx yyyyyzzz zzwwwww0
                01234567 89abcdef 01234567 89abcdef

                rst %r0, %r1, %r2, %r3
            */
            switch (BITS(8, 3)) {
                case 7: {
                    alu::perform_operation(proc, REGX, 0, 0, alu::HY_rst);
                } break;
                case 6: {
                    alu::perform_operation(proc, REGX, 0, 0, alu::HY_rst);
                    alu::perform_operation(proc, REGY, 0, 0, alu::HY_rst);
                } break;
                case 5: {
                    alu::perform_operation(proc, REGX, 0, 0, alu::HY_rst);
                    alu::perform_operation(proc, REGY, 0, 0, alu::HY_rst);
                    alu::perform_operation(proc, REGZ, 0, 0, alu::HY_rst);
                } break;
                case 4: {
                    alu::perform_operation(proc, REGX, 0, 0, alu::HY_rst);
                    alu::perform_operation(proc, REGY, 0, 0, alu::HY_rst);
                    alu::perform_operation(proc, REGZ, 0, 0, alu::HY_rst);
                    alu::perform_operation(proc, REGW, 0, 0, alu::HY_rst);
                } break;
            }
        } break;

        case HY_TST: {
            /*  Test register x bit
                iiiiiiii 111xxxxx bbbbb000 00000000
                01234567 89abcdef 01234567 89abcdef
            */
            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, IMM51, 0, alu::HY_tst); return true; } break;
            }
        } break;

        case HY_CMP: {
            /*  Compare register x with 0           cmpz %r0;
                iiiiiiii 111xxxxx 00000000 00000000
                01234567 89abcdef 01234567 89abcdef
            
                Compare register x with register y
                iiiiiiii 110xxxxx yyyyy000 00000000
                01234567 89abcdef 01234567 89abcdef
                
                Compare register x with 8-bit immediate
                iiiiiiii 101xxxxx iiiiiiii 00000000
                01234567 89abcdef 01234567 89abcdef
                
                Compare register x with 16-bit immediate
                iiiiiiii 100xxxxx iiiiiiii iiiiiiii
                01234567 89abcdef 01234567 89abcdef

                cmp %r5, 0xdead
            */
            switch (BITS(8, 3)) {
                case 7: { alu::perform_operation(proc, REGX, 0    , 0, alu::HY_cmp); return true; } break;
                case 6: { alu::perform_operation(proc, REGX, REGY , 0, alu::HY_cmp); return true; } break;
                case 5: { alu::perform_operation(proc, REGX, IMM81, 0, alu::HY_cmp); return true; } break;
                case 4: { alu::perform_operation(proc, REGX, IMM16, 0, alu::HY_cmp); return true; } break;
            }
        } break;

        case HY_JC: {
            /*  
                iiiiiiii 111cccc0 iiiiiiii iiiiiiii jne 0xabcd
                iiiiiiii 110cccc0 xxxxxsss ssoooooo jne (%r0<<4)+1
                iiiiiiii 101cccc0 xxxxxyyy yysssss0 jne %r0, %r1<<2
            */
            switch (BITS(8, 3)) {
                case 7: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] &= 0xffff0000; proc->internal.r[pc] |= IMM16; } } break;
                case 6: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] = (REGY << BITS(21, 5)) + BITS(26, 6); } } break;
                case 5: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] = (REGY + (REGZ << BITS(26, 5))); } } break;
            }
        } break;

        case HY_BC: {
            /*  
                iiiiiiii 111cccc0 iiiiiiii 00000000
                iiiiiiii 110cccc0 iiiiiiii iiiiiiii
                iiiiiiii 101cccc0 xxxxxsss ss000000
            */
            switch (BITS(8, 3)) {
                case 7: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] += (int8_t)IMM81; } } break;
                case 6: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] += (int16_t)IMM16; } } break;
                case 5: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] += REGY << BITS(21, 5); } } break;
            }
        } break;

        case HY_JALC: {
            /*  
                iiiiiiii 111cccc0 iiiiiiii iiiiiiii jalne 0xabcd
                iiiiiiii 110cccc0 xxxxxsss ssoooooo jalne (%r0<<4)+1
                iiiiiiii 101cccc0 xxxxxyyy yysssss0 jalne %r0, %r1<<2
            */
            switch (BITS(8, 3)) {
                case 7: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[lr0] = proc->internal.r[pc] + 4; proc->internal.r[pc] &= 0xffff0000; proc->internal.r[pc] |= IMM16; } } break;
                case 6: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[lr0] = proc->internal.r[pc] + 4; proc->internal.r[pc] = (REGY << BITS(21, 5)) + BITS(26, 6); } } break;
                case 5: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[lr0] = proc->internal.r[pc] + 4; proc->internal.r[pc] = (REGY + (REGZ << BITS(26, 5))); } } break;
            }
        } break;

        case HY_RTL: {
            /*  
                iiiiiiii 111cccc0 00000000 00000000 rtlne
            */
            switch (BITS(8, 3)) {
                case 7: { if (hyrisc_test_condition(proc, COND)) { proc->internal.r[pc] = proc->internal.r[lr0]; } } break;
            }
        } break;

    // To-do
    // tst  : cc
    // cmp  : cb

    // Shifts, Rotates
    // sll  : bf
    // slr  : be
    // sal  : bd
    // sar  : bc
    // rl   : bb
    // rr   : ba

    // Jumps, Branches (Basic Flow Control)
    // jc   : af
    // jp   : ae
    // bc   : ad
    // br   : ac
    // jalc : ab
    // jpl  : aa

    // Calls, Returns (Advanced Flow Control)
    // callc: 9f
    // call : 9e
    // retc : 9d
    // ret  : 9c

    // Push, Pop (Stack Manipulation)
    // push : 8f
    // pop  : 8e

    // Misc:
    // nop  : 7f
    }

    proc->internal.r[r0] = 0;
    proc->internal.r[pc] += 4;
}

#undef BITS