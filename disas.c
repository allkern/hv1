/* Disassemble hyrisc instructions.
   Copyright (C) 2009-2022 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Definitions for decoding the hyrisc opcode table.
   Copyright (C) 2009-2022 Free Software Foundation, Inc.
   Contributed by Lisandro Alarcon (lisandroaalarcon@gmail.com).

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define HYRISC_ENC_4    0x103
#define HYRISC_ENC_3    0x102
#define HYRISC_ENC_2    0x101
#define HYRISC_ENC_1    0x100
#define HYRISC_ENC_0    HYRISC_ENC_1

// Instruction Operand Mode
#define HYRISC_OPT_NONE  0x0 // No operands
#define HYRISC_OPT_1R    0x1 // 1 Register
#define HYRISC_OPT_2R    0x2 // 2 Registers
#define HYRISC_OPT_3R    0x3 // 3 Registers
#define HYRISC_OPT_RIXM  0x4 // Register, Indexed Multiply
#define HYRISC_OPT_RIXS  0x5 // Register, Indexed Shift
#define HYRISC_OPT_IXMR  0x6 // Indexed Multiply, Register
#define HYRISC_OPT_IXSR  0x7 // Indexed Shift, Register
#define HYRISC_OPT_IXML  0x8 // Indexed Multiply
#define HYRISC_OPT_IXSH  0x9 // Indexed Shift
#define HYRISC_OPT_2RI8  0xa // 2 Registers, 8-bit Immediate
#define HYRISC_OPT_RI16  0xb // Register, 16-bit Immediate
#define HYRISC_OPT_RRNG  0xc // Register Range
#define HYRISC_OPT_I16   0xd // 16-bit Immediate
#define HYRISC_OPT_RI8   0xe // Register, 8-bit Immediate
#define HYRISC_OPT_RI5   0xf // Register, 5-bit Immediate

// Size operand usage
#define HYRISC_SOP_NONE       0x0
#define HYRISC_SOP_LOAD_STORE 0x1
#define HYRISC_SOP_INC_DEC    0x2
#define HYRISC_SOP_BRANCH     0x3

// Conditional
#define HYRISC_NO_COND        0x0
#define HYRISC_COND           0x1

#include <stdint.h>

enum hyrisc_opcodes_t {
    HY_MOV       = 0xff,
    HY_LI        = 0xfe,
    HY_LUI       = 0xfd,
    HY_LOADM     = 0xfc, // LOAD Multiply
    HY_LOADS     = 0xfb, // LOAD Shift
    HY_STOREM    = 0xfa, // STORE Multiply
    HY_STORES    = 0xf9, // STORE Shift
    HY_LEAM      = 0xf8, // LEA Multiply
    HY_LEAS      = 0xf7, // LEA Shift
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
    HY_NOP       = 0x8f,
    HY_BAD       = 0x00
};

typedef struct hyrisc_opc_info_t
{
  short         opcode;
  unsigned      encoding;
  int           operand_mode;
  int           size_operand;
  int           conditional;
  const char *  name;
} hyrisc_opc_info_t;

const hyrisc_opc_info_t hyrisc_opc_info[256] = {
    [0xff] = { HY_MOV      , HYRISC_ENC_4, HYRISC_OPT_2R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "mov"   },
    [0xfe] = { HY_LI       , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "li"    },
    [0xfd] = { HY_LUI      , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "lui"   },
    [0xfc] = { HY_LOADM    , HYRISC_ENC_4, HYRISC_OPT_RIXM, HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "load"  },
    [0xfb] = { HY_LOADS    , HYRISC_ENC_4, HYRISC_OPT_RIXS, HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "load"  },
    [0xfa] = { HY_STOREM   , HYRISC_ENC_4, HYRISC_OPT_IXMR, HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "store" },
    [0xf9] = { HY_STORES   , HYRISC_ENC_4, HYRISC_OPT_IXSR, HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "store" },
    [0xf8] = { HY_LEAM     , HYRISC_ENC_4, HYRISC_OPT_RIXM, HYRISC_SOP_NONE      , HYRISC_NO_COND, "lea"   },
    [0xf7] = { HY_LEAS     , HYRISC_ENC_4, HYRISC_OPT_RIXS, HYRISC_SOP_NONE      , HYRISC_NO_COND, "lea"   },
    [0xf6] = { 0xf6        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xf5] = { 0xf5        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xf4] = { 0xf4        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xf3] = { 0xf3        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xf2] = { 0xf2        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xf1] = { 0xf1        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xf0] = { 0xf0        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xef] = { HY_ADDR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "addu"  },
    [0xee] = { HY_ADDUI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "addu"  },
    [0xed] = { HY_ADDUI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "addu"  },
    [0xec] = { HY_ADDSI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "adds"  },
    [0xeb] = { HY_ADDSI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "adds"  },
    [0xea] = { HY_SUBR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "subu"  },
    [0xe9] = { HY_SUBUI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "subu"  },
    [0xe8] = { HY_SUBUI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "subu"  },
    [0xe7] = { HY_SUBSI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "subs"  },
    [0xe6] = { HY_SUBSI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "subs"  },
    [0xe5] = { HY_MULR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "mulu"  },
    [0xe4] = { HY_MULUI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "mulu"  },
    [0xe3] = { HY_MULUI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "mulu"  },
    [0xe2] = { HY_MULSI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "muls"  },
    [0xe1] = { HY_MULSI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "muls"  },
    [0xe0] = { HY_DIVR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "divu"  },
    [0xdf] = { HY_DIVUI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "divu"  },
    [0xde] = { HY_DIVUI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "divu"  },
    [0xdd] = { HY_DIVSI8   , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "divs"  },
    [0xdc] = { HY_DIVSI16  , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "divs"  },
    [0xdb] = { HY_CMPZ     , HYRISC_ENC_4, HYRISC_OPT_1R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "cmpz"  },
    [0xda] = { HY_CMPR     , HYRISC_ENC_4, HYRISC_OPT_2R  , HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "cmp"   },
    [0xd9] = { HY_CMPI8    , HYRISC_ENC_3, HYRISC_OPT_RI8 , HYRISC_SOP_NONE      , HYRISC_NO_COND, "cmp.b" },
    [0xd8] = { HY_CMPI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "cmp.s" },
    [0xd7] = { 0xd7        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd6] = { 0xd6        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd5] = { 0xd5        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd4] = { 0xd4        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd3] = { 0xd3        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd2] = { 0xd2        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd1] = { 0xd1        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xd0] = { 0xd0        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xcf] = { HY_ANDR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "and"   },
    [0xce] = { HY_ANDI8    , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "and"   },
    [0xcd] = { HY_ANDI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "and"   },
    [0xcc] = { HY_ORR      , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "or"    },
    [0xcb] = { HY_ORI8     , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "or"    },
    [0xca] = { HY_ORI16    , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "or"    },
    [0xc9] = { HY_XORR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "xor"   },
    [0xc8] = { HY_XORI8    , HYRISC_ENC_3, HYRISC_OPT_2RI8, HYRISC_SOP_NONE      , HYRISC_NO_COND, "xor"   },
    [0xc7] = { HY_XORI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "xor"   },
    [0xc6] = { HY_NOT      , HYRISC_ENC_4, HYRISC_OPT_2R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "not"   },
    [0xc5] = { HY_NEG      , HYRISC_ENC_4, HYRISC_OPT_2R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "neg"   },
    [0xc4] = { HY_SEXT     , HYRISC_ENC_4, HYRISC_OPT_2R  , HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "sext"  },
    [0xc3] = { HY_ZEXT     , HYRISC_ENC_4, HYRISC_OPT_2R  , HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "zext"  },
    [0xc2] = { HY_RSTS     , HYRISC_ENC_4, HYRISC_OPT_1R  , HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "rst"   },
    [0xc1] = { HY_RSTM     , HYRISC_ENC_4, HYRISC_OPT_RRNG, HYRISC_SOP_LOAD_STORE, HYRISC_NO_COND, "rst"   },
    [0xc0] = { HY_INC      , HYRISC_ENC_4, HYRISC_OPT_1R  , HYRISC_SOP_INC_DEC   , HYRISC_NO_COND, "inc"   },
    [0xbf] = { HY_DEC      , HYRISC_ENC_4, HYRISC_OPT_1R  , HYRISC_SOP_INC_DEC   , HYRISC_NO_COND, "dec"   },
    [0xbe] = { HY_TST      , HYRISC_ENC_4, HYRISC_OPT_RI5 , HYRISC_SOP_NONE      , HYRISC_NO_COND, "tst"   },
    [0xbd] = { HY_LSLR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "lsl"   },
    [0xbc] = { HY_LSLI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "lsl"   },
    [0xbb] = { HY_LSRR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "lsr"   },
    [0xba] = { HY_LSRI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "lsr"   },
    [0xb9] = { HY_ASLR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "asl"   },
    [0xb8] = { HY_ASLI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "asl"   },
    [0xb7] = { HY_ASRR     , HYRISC_ENC_4, HYRISC_OPT_3R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "asr"   },
    [0xb6] = { HY_ASRI16   , HYRISC_ENC_2, HYRISC_OPT_RI16, HYRISC_SOP_NONE      , HYRISC_NO_COND, "asr"   },
    [0xb5] = { 0xb5        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xb4] = { 0xb4        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xb3] = { 0xb3        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xb2] = { 0xb2        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xb1] = { 0xb1        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xb0] = { 0xb0        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xaf] = { HY_BCCS     , HYRISC_ENC_2, HYRISC_OPT_I16 , HYRISC_SOP_BRANCH    , HYRISC_COND   , "b"     },
    [0xae] = { HY_BCCU     , HYRISC_ENC_2, HYRISC_OPT_I16 , HYRISC_SOP_BRANCH    , HYRISC_COND   , "b"     },
    [0xad] = { HY_JALCCI16 , HYRISC_ENC_2, HYRISC_OPT_I16 , HYRISC_SOP_NONE      , HYRISC_COND   , "jal"   },
    [0xac] = { HY_JALCCM   , HYRISC_ENC_4, HYRISC_OPT_IXML, HYRISC_SOP_NONE      , HYRISC_COND   , "jal"   },
    [0xab] = { HY_JALCCS   , HYRISC_ENC_4, HYRISC_OPT_IXSH, HYRISC_SOP_NONE      , HYRISC_COND   , "jal"   },
    [0xaa] = { HY_CALLCCI16, HYRISC_ENC_2, HYRISC_OPT_I16 , HYRISC_SOP_NONE      , HYRISC_COND   , "call"  },
    [0xa9] = { HY_CALLCCM  , HYRISC_ENC_4, HYRISC_OPT_IXML, HYRISC_SOP_NONE      , HYRISC_COND   , "call"  },
    [0xa8] = { HY_CALLCCS  , HYRISC_ENC_4, HYRISC_OPT_IXSH, HYRISC_SOP_NONE      , HYRISC_COND   , "call"  },
    [0xa7] = { HY_RTLCC    , HYRISC_ENC_4, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_COND   , "rtl"   },
    [0xa6] = { HY_RETCC    , HYRISC_ENC_4, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_COND   , "ret"   },
    [0xa5] = { 0xa5        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xa4] = { 0xa4        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xa3] = { 0xa3        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xa2] = { 0xa2        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xa1] = { 0xa1        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0xa0] = { 0xa0        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x9f] = { HY_PUSHM    , HYRISC_ENC_4, HYRISC_OPT_RRNG, HYRISC_SOP_NONE      , HYRISC_NO_COND, "push"  },
    [0x9e] = { HY_POPM     , HYRISC_ENC_4, HYRISC_OPT_RRNG, HYRISC_SOP_NONE      , HYRISC_NO_COND, "pop"   },
    [0x9d] = { HY_PUSHS    , HYRISC_ENC_4, HYRISC_OPT_1R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "push"  },
    [0x9c] = { HY_POPS     , HYRISC_ENC_4, HYRISC_OPT_1R  , HYRISC_SOP_NONE      , HYRISC_NO_COND, "pop"   },
    [0x9b] = { 0x9b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x9a] = { 0x9a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x99] = { 0x99        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x98] = { 0x98        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x97] = { 0x97        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x96] = { 0x96        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x95] = { 0x95        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x94] = { 0x94        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x93] = { 0x93        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x92] = { 0x92        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x91] = { 0x91        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x90] = { 0x90        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x8f] = { HY_NOP      , HYRISC_ENC_4, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "nop"   },
    [0x8e] = { 0x8e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x8d] = { 0x8d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x8c] = { 0x8c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x8b] = { 0x8b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x8a] = { 0x8a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x89] = { 0x89        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x88] = { 0x88        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x87] = { 0x87        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x86] = { 0x86        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x85] = { 0x85        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x84] = { 0x84        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x83] = { 0x83        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x82] = { 0x82        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x81] = { 0x81        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x80] = { 0x80        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x7f] = { 0x7f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x7e] = { 0x7e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x7d] = { 0x7d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x7c] = { 0x7c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x7b] = { 0x7b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x7a] = { 0x7a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x79] = { 0x79        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x78] = { 0x78        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x77] = { 0x77        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x76] = { 0x76        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x75] = { 0x75        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x74] = { 0x74        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x73] = { 0x73        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x72] = { 0x72        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x71] = { 0x71        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x70] = { 0x70        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x6f] = { 0x6f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x6e] = { 0x6e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x6d] = { 0x6d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x6c] = { 0x6c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x6b] = { 0x6b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x6a] = { 0x6a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x69] = { 0x69        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x68] = { 0x68        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x67] = { 0x67        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x66] = { 0x66        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x65] = { 0x65        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x64] = { 0x64        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x63] = { 0x63        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x62] = { 0x62        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x61] = { 0x61        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x60] = { 0x60        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x5f] = { 0x5f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x5e] = { 0x5e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x5d] = { 0x5d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x5c] = { 0x5c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x5b] = { 0x5b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x5a] = { 0x5a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x59] = { 0x59        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x58] = { 0x58        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x57] = { 0x57        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x56] = { 0x56        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x55] = { 0x55        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x54] = { 0x54        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x53] = { 0x53        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x52] = { 0x52        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x51] = { 0x51        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x50] = { 0x50        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x4f] = { 0x4f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x4e] = { 0x4e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x4d] = { 0x4d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x4c] = { 0x4c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x4b] = { 0x4b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x4a] = { 0x4a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x49] = { 0x49        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x48] = { 0x48        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x47] = { 0x47        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x46] = { 0x46        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x45] = { 0x45        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x44] = { 0x44        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x43] = { 0x43        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x42] = { 0x42        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x41] = { 0x41        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x40] = { 0x40        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x3f] = { 0x3f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x3e] = { 0x3e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x3d] = { 0x3d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x3c] = { 0x3c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x3b] = { 0x3b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x3a] = { 0x3a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x39] = { 0x39        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x38] = { 0x38        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x37] = { 0x37        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x36] = { 0x36        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x35] = { 0x35        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x34] = { 0x34        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x33] = { 0x33        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x32] = { 0x32        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x31] = { 0x31        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x30] = { 0x30        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x2f] = { 0x2f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x2e] = { 0x2e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x2d] = { 0x2d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x2c] = { 0x2c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x2b] = { 0x2b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x2a] = { 0x2a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x29] = { 0x29        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x28] = { 0x28        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x27] = { 0x27        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x26] = { 0x26        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x25] = { 0x25        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x24] = { 0x24        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x23] = { 0x23        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x22] = { 0x22        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x21] = { 0x21        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x20] = { 0x20        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x1f] = { 0x1f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x1e] = { 0x1e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x1d] = { 0x1d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x1c] = { 0x1c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x1b] = { 0x1b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x1a] = { 0x1a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x19] = { 0x19        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x18] = { 0x18        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x17] = { 0x17        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x16] = { 0x16        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x15] = { 0x15        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x14] = { 0x14        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x13] = { 0x13        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x12] = { 0x12        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x11] = { 0x11        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x10] = { 0x10        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x0f] = { 0x0f        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x0e] = { 0x0e        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x0d] = { 0x0d        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x0c] = { 0x0c        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x0b] = { 0x0b        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x0a] = { 0x0a        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x09] = { 0x09        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x08] = { 0x08        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x07] = { 0x07        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x06] = { 0x06        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x05] = { 0x05        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x04] = { 0x04        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x03] = { 0x03        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x02] = { 0x02        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x01] = { 0x01        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" },
    [0x00] = { 0x00        , HYRISC_ENC_0, HYRISC_OPT_NONE, HYRISC_SOP_NONE      , HYRISC_NO_COND, "<bad>" }
};

#include <stdio.h>
#include <stdlib.h>

#define HYRISC_IW_BITS(b, l) ((iword >> b) & ((1 << l) - 1))

#define HYRISC_DIS_OPCODE   HYRISC_IW_BITS(0, 8)
#define HYRISC_DIS_ENCODING HYRISC_IW_BITS(8, 2)
#define HYRISC_DIS_FIELDX   HYRISC_IW_BITS(10, 5)
#define HYRISC_DIS_FIELDY   HYRISC_IW_BITS(15, 5)
#define HYRISC_DIS_FIELDZ   HYRISC_IW_BITS(20, 5)
#define HYRISC_DIS_FIELDW   HYRISC_IW_BITS(25, 5)
#define HYRISC_DIS_SIZE     HYRISC_IW_BITS(30, 2)
#define HYRISC_DIS_IMM8     HYRISC_IW_BITS(20, 8)
#define HYRISC_DIS_IMM16    HYRISC_IW_BITS(15, 16)

const char* hyrisc_dis_cc[] = {
    "eq", "ne", "cs", "cc",
    "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt",
    "gt", "le", "ra", ""  ,
    "cc", "cc", "cc", "cc",
    "cc", "cc", "cc", "cc",
    "cc", "cc", "cc", "cc",
    "cc", "cc", "cc", "cc"
};

const char* hyrisc_register_names_abi[] = {
    "r0" , "r1" , "r2" , "r3" ,
    "r4" , "r5" , "r6" , "r7" ,
    "r8" , "r9" , "r10", "r11",
    "r12", "r13", "r14", "r15",
    "a0" , "a1" , "a2" , "rr0",
    "a3" , "a4" , "a5" , "rr1",
    "lr0", "lr1", "lr2", "lr3",
    "ir" , "br" , "sp" , "pc"
};

const char* hyrisc_dis_sop_load_store = "bslx";
const char* hyrisc_dis_sop_inc_dec = "bsld";
const char* hyrisc_dis_sop_branch = "us";

int print_insn_hyrisc(unsigned long iword) {
    unsigned opcode = HYRISC_DIS_OPCODE;

    hyrisc_opc_info_t insn = hyrisc_opc_info[opcode];

    switch (insn.operand_mode) {
        case HYRISC_OPT_NONE: {
            if (insn.conditional && insn.size_operand) { 
                printf("%s%s.%c",
                    insn.name,
                    hyrisc_dis_cc[HYRISC_DIS_FIELDX],
                    hyrisc_dis_sop_branch[opcode & 0x1]
                );
            } else if (insn.conditional && !insn.size_operand) {
                printf("%s%s",
                    insn.name,
                    hyrisc_dis_cc[HYRISC_DIS_FIELDX]
                );
            } else if (insn.size_operand && !insn.conditional) {
                switch (insn.size_operand) {
                    case HYRISC_SOP_LOAD_STORE: {
                        printf("%s.%c",
                            insn.name,
                            hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE]
                        );
                    } break;
                    case HYRISC_SOP_INC_DEC: {
                        printf("%s.%c",
                            insn.name,
                            hyrisc_dis_sop_inc_dec[HYRISC_DIS_SIZE]
                        );
                    } break;
                }
            } else {
                printf("%s", insn.name);
            }
        } break;

        case HYRISC_OPT_1R: {
            if (insn.size_operand == HYRISC_SOP_INC_DEC) {
                printf("%s.%c %s",
                    insn.name,
                    hyrisc_dis_sop_inc_dec[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX]
                );
            } else {
                printf("%s %s",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX]
                );
            }
        } break;

        case HYRISC_OPT_2R: {
            if (insn.size_operand == HYRISC_SOP_LOAD_STORE) {
                printf("%s.%c %s, %s",
                    insn.name,
                    hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY]
                );
            } else {
                printf("%s %s, %s",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY]
                );
            }
        } break;

        case HYRISC_OPT_3R: {
            printf("%s %s, %s, %s",
                insn.name,
                hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                hyrisc_register_names_abi[HYRISC_DIS_FIELDZ]
            );
        } break;

        case HYRISC_OPT_RIXM: {
            if (insn.size_operand == HYRISC_SOP_LOAD_STORE) {
                printf("%s.%c %s, [%s+%s*%i]",
                    insn.name,
                    hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            } else {
                // Bad
                printf("%s %s, [%s+%s*%i] <bad>",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            }
        } break;

        case HYRISC_OPT_RIXS: {
            if (insn.size_operand == HYRISC_SOP_LOAD_STORE) {
                printf("%s.%c %s, [%s+%s:%i]",
                    insn.name,
                    hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            } else {
                // Bad
                printf("%s %s, [%s+%s:%i] <bad>",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            }
        } break;

        case HYRISC_OPT_IXMR: {
            if (insn.size_operand == HYRISC_SOP_LOAD_STORE) {
                printf("%s.%c [%s+%s*%i], %s",
                    insn.name,
                    hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX]
                );
            } else {
                // Bad
                printf("%s [%s+%s*%i], %s <bad>",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX]
                );
            }
        } break;
        
        case HYRISC_OPT_IXSR: {
            if (insn.size_operand == HYRISC_SOP_LOAD_STORE) {
                printf("%s.%c [%s+%s:%i], %s",
                    insn.name,
                    hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX]
                );
            } else {
                // Bad
                printf("%s [%s+%s:%i], %s <bad>",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX]
                );
            }
        } break;

        case HYRISC_OPT_IXML: {
            if (insn.conditional) {
                printf("%s%s [%s+%s*%i]",
                    insn.name,
                    hyrisc_dis_cc[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            } else {
                printf("%s [%s+%s*%i] <bad>",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            }
        } break;

        case HYRISC_OPT_IXSH: {
            if (insn.conditional) {
                printf("%s%s [%s+%s:%i]",
                    insn.name,
                    hyrisc_dis_cc[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            } else {
                printf("%s [%s+%s:%i] <bad>",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDZ],
                    HYRISC_DIS_FIELDW
                );
            }
        } break;

        case HYRISC_OPT_2RI8: {
            printf("%s %s, %s, 0x%02x",
                insn.name,
                hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                hyrisc_register_names_abi[HYRISC_DIS_FIELDY],
                HYRISC_DIS_IMM8
            );
        } break;

        case HYRISC_OPT_RI16: {
            printf("%s %s, 0x%04x",
                insn.name,
                hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                HYRISC_DIS_IMM16
            );
        } break;

        case HYRISC_OPT_RRNG: {
            if (insn.size_operand == HYRISC_SOP_LOAD_STORE) {
                printf("%s.%c {%s-%s}",
                    insn.name,
                    hyrisc_dis_sop_load_store[HYRISC_DIS_SIZE],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY]
                );
            } else {
                printf("%s {%s-%s}",
                    insn.name,
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                    hyrisc_register_names_abi[HYRISC_DIS_FIELDY]
                );
            }
        } break;

        case HYRISC_OPT_I16: {
            if (insn.size_operand == HYRISC_SOP_BRANCH) {
                if (opcode & 0x1) {
                    printf("%s%s.%c %+i",
                        insn.name,
                        hyrisc_dis_cc[HYRISC_DIS_FIELDX],
                        hyrisc_dis_sop_branch[opcode & 0x1],
                        (int16_t)HYRISC_DIS_IMM16
                    );
                } else {
                    printf("%s%s.%c 0x%04x",
                        insn.name,
                        hyrisc_dis_cc[HYRISC_DIS_FIELDX],
                        hyrisc_dis_sop_branch[opcode & 0x1],
                        HYRISC_DIS_IMM16
                    );
                }
            } else {
                printf("%s%s 0x%04x",
                    insn.name,
                    hyrisc_dis_cc[HYRISC_DIS_FIELDX],
                    HYRISC_DIS_IMM16
                );
            }
        } break;

        case HYRISC_OPT_RI8: {
            printf("%s %s, %02x",
                insn.name,
                hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                HYRISC_DIS_IMM8 & 0xff
            );
        } break;
        
        case HYRISC_OPT_RI5: {
            printf("%s %s, %i",
                insn.name,
                hyrisc_register_names_abi[HYRISC_DIS_FIELDX],
                HYRISC_DIS_IMM16 & 0x1f
            );
        } break;

        default: {
            printf("<bad>");
        }
    }

    return 0;
}

int main(int argc, const char* argv[]) {
    print_insn_hyrisc(strtoul(argv[1], NULL, 0));
}