#pragma once

#include "types.hpp"
#include "state.hpp"
#include "flags.hpp"

namespace alu {
    typedef hyu64_t (*operation_t)(hyu32_t&, hyu32_t, hyu32_t);

#define OPERATION(name, code) \
operation_t HY_##name = [](hyu32_t& dst, hyu32_t src1, hyu32_t src2) -> hyu64_t { hyu64_t temp = code; return temp; }

    OPERATION(addu, src1 + src2         ; dst = temp & 0xffffffff);
    OPERATION(subu, src1 - src2         ; dst = temp & 0xffffffff);
    OPERATION(mulu, src1 * src2         ; dst = temp & 0xffffffff);
    OPERATION(divu, src1 / src2         ; dst = temp & 0xffffffff);
    OPERATION(adds, src1 + (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(subs, src1 - (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(muls, src1 * (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(divs, src1 / (hyi32_t)src2; dst = temp & 0xffffffff);
    OPERATION(and , src1 & src2         ; dst = temp & 0xffffffff);
    OPERATION(or  , src1 | src2         ; dst = temp & 0xffffffff);
    OPERATION(xor , src1 ^ src2         ; dst = temp & 0xffffffff);
    OPERATION(not , ~src1               ; dst = temp & 0xffffffff);
    OPERATION(neg , ~src1               ; dst = temp & 0xffffffff);
    OPERATION(inc , dst + src1          ; dst = temp & 0xffffffff);
    OPERATION(dec , dst - src1          ; dst = temp & 0xffffffff);
    OPERATION(rst , 0                   ; dst = temp & 0xffffffff);
    OPERATION(tst , dst & (1 << src1)   ;                        );
    OPERATION(cmp , dst - src1          ;                        );
    OPERATION(cmpb, dst - (src1 & 0xff) ;                        );
    OPERATION(lsl , src1 << src2        ; dst = temp & 0xffffffff);
    OPERATION(lsr , src1 >> src2        ; dst = temp & 0xffffffff);
    OPERATION(asl , src1 << src2        ; dst = temp & 0xffffffff);
    OPERATION(asr , src1 >> src2        ; dst = temp & 0xffffffff);
    OPERATION(rl  , src1 << src2        ; dst = temp & 0xffffffff);
    OPERATION(rr  , src1 >> src2        ; dst = temp & 0xffffffff);

#undef OPERATION

    void perform_operation(hyrisc_t* proc, hyu32_t& dst, hyu32_t src1, hyu32_t src2, operation_t op) {
        hyu64_t temp = op(dst, src1, src2);

        hyrisc_set_flags(proc, Z, !(temp & 0xffffffff));
        hyrisc_set_flags(proc, N, (temp & 0xffffffff) & 0x80000000);
        hyrisc_set_flags(proc, C, temp > 0xffffffff);
    }
}