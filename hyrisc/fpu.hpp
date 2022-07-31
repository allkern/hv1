#pragma once

#include "types.hpp"
#include "state.hpp"

#include <cfenv>
#include <cmath>

// FPU version 1 revision 0
// FPU Basic ISA
// Supported instructions:
// fadd   fd, fs0, fs1
// fsub   fd, fs0, fs1
// fmul   fd, fs0, fs1
// fdiv   fd, fs0, fs1
// ffma   fd, fs0, fs1
// fsqrt  fd, fs0
// fpow   fd, fs0, fs1
// fabs   fd, fs0
// fmod   fd, fs0, fs1
// fexp   fd, fs0
// fmin   fd, fs0, fs1
// fmax   fd, fs0, fs1
// fround fd, fs0
// fclamp fd, fs0, fs1

// Conversion functions, use when transferring data from
// main registers to floating point and back
// fcvti  fd, fs0       f[d] = hyu32_t(round(f[fs0]))
// fcvtf  fd, fs0       f[d] = float(f[fs0])

// Example:
// add   r0, 0x2
// mul   r0, 0x2
// l     f1, r0
// fcvtf f1, f1

// FPU Trig extension (supported)
// fsin   fd, fs0
// fcos   fd, fs0
// ftan   fd, fs0
// fasin  fd, fs0
// facos  fd, fs0
// fatan  fd, fs0
// fsinh  fd, fs0
// fcosh  fd, fs0
// ftanh  fd, fs0

// FPU Vector extension (not yet supported)
// iiiiiiii 111xxxxx yyyyyzz zzzzzRss
// fvadd{2, 4, 8} fdi, fsi0, fsi1  f[fdi+0] = f[fsi0+0] + f[fsi1+0]
// fvsub{2, 4, 8} fdi, fsi0, fsi1
// fvmul{2, 4, 8} fdi, fsi0, fsi1
// fvdiv{2, 4, 8} fdi, fsi0, fsi1
// fvfma{2, 4, 8} fdi, fsi0, fsi1  f[fdi+0] += f[fsi0+0] * f[fsi1+0]
// fvsum{2, 4, 8} fd , fsi0        f[fd] += f[fsi0+i], i = 0 -> S-1
// fvwsm{2, 4, 8} fd , fsi0, fsi1  f[fd] += f[fsi0+i] * f[fsi1+i]
// fvavg{2, 4, 8} fd , fsi0        f[fd] += f[fsi0+i], f[fd] = f[fd] /= S
// fvwvg{2, 4, 8} fd , fsi0, fsi1  f[fd] += f[fsi0+i] * f[fsi1+i], f[fd] /= S

namespace fpu {
    typedef hyfloat_t (*operation_t)(hyfloat_t&, hyfloat_t, hyfloat_t);

#define OPERATION(name, code) \
    operation_t HY_##name = [](hyfloat_t& dst, hyfloat_t src1, hyfloat_t src2) -> hyfloat_t { \
        hyfloat_t temp = code; \
\
        return temp; \
    }

    OPERATION(fadd  , src1 + src2           ; dst  = temp);
    OPERATION(fsub  , src1 - src2           ; dst  = temp);
    OPERATION(fmul  , src1 * src2           ; dst  = temp);
    OPERATION(fdiv  , src1 / src2           ; dst  = temp);
    OPERATION(ffma  , src1 * src2           ; dst += temp);
    OPERATION(fpow  , std::pow(src1, src2)  ; dst  = temp);
    OPERATION(fsqrt , std::sqrt(src1)       ; dst  = temp);
    OPERATION(fabs  , std::fabs(src1)       ; dst  = temp);
    OPERATION(fmod  , std::fmod(src1, src2) ; dst  = temp);
    OPERATION(fexp  , std::exp(src1)        ; dst  = temp);
    OPERATION(fmin  , std::fminf(src1, src2); dst  = temp);
    OPERATION(fmax  , std::fmaxf(src1, src2); dst  = temp);
    OPERATION(fsin  , std::sin(src1)        ; dst  = temp);
    OPERATION(fcos  , std::cos(src1)        ; dst  = temp);
    OPERATION(ftan  , std::tan(src1)        ; dst  = temp);
    OPERATION(fasin , std::asin(src1)       ; dst  = temp);
    OPERATION(facos , std::acos(src1)       ; dst  = temp);
    OPERATION(fatan , std::atan(src1)       ; dst  = temp);
    OPERATION(fsinh , std::sinh(src1)       ; dst  = temp);
    OPERATION(fcosh , std::cosh(src1)       ; dst  = temp);
    OPERATION(ftanh , std::tanh(src1)       ; dst  = temp);
    OPERATION(fround, std::rintf(src1)      ; dst  = temp);
    OPERATION(fclamp, (dst < src1) ? src1 : ((dst > src2) ? src2 : dst); dst = temp);
    OPERATION(fcvti , 0.0f; *(hyu32_t*)&dst = (hyu32_t)(hyint_t)std::rintf(src1));
    OPERATION(fcvtf , 0.0f; dst = (hyfloat_t)(hyu32_t)src1);

#undef OPERATION

    void perform_operation(hyrisc_t* proc, hyfloat_t& dst, hyfloat_t src1, hyfloat_t src2, operation_t op) {
        hyu32_t fpcsr = *(hyu32_t*)&proc->internal.f[31];

        std::fesetround((fpcsr >> 5) & 0x3);

        hyfloat_t temp = op(dst, src1, src2);

        if (std::fetestexcept(FE_DIVBYZERO)) fpcsr |= (1 << 0);
        if (std::fetestexcept(FE_INEXACT  )) fpcsr |= (1 << 1);
        if (std::fetestexcept(FE_INVALID  )) fpcsr |= (1 << 2);
        if (std::fetestexcept(FE_OVERFLOW )) fpcsr |= (1 << 3);
        if (std::fetestexcept(FE_UNDERFLOW)) fpcsr |= (1 << 4);

        *(hyu32_t*)&proc->internal.f[31] = fpcsr;
    }
}