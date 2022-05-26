/*
    addu %pc, 0xffff
*/

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

#include "../log.hpp"
 
static const std::string whitespace = " \n\r\t\f\v";
 
std::string ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(whitespace);

    return (start == std::string::npos) ? "" : s.substr(start);
}
 
std::string rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(whitespace);

    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
std::string trim(const std::string& s) {
    return rtrim(ltrim(s));
}

typedef uint8_t  hyu8_t;
typedef uint16_t hyu16_t;
typedef uint32_t hyu32_t;
typedef uint64_t hyu64_t;
typedef  int8_t  hyi8_t;
typedef  int16_t hyi16_t;
typedef  int32_t hyi32_t;
typedef  int64_t hyi64_t;
typedef float    hyfloat_t;
typedef bool     hybool_t;
typedef int      hyint_t;

enum hyrisc_register_names_t {
    r0  , gp0 , gp1 , gp2 ,
    gp3 , gp4 , gp5 , gp6 ,
    gp7 , gp8 , gp9 , gp10,
    gp11, gp12, gp13, gp14,
    a0  , a1  , a2  , rr0 ,
    a3  , a4  , a5  , rr1 ,
    lr0 , lr1 , lr2 , lr3 ,
    ir  , br  , sp  , pc
};

std::string register_names[] = {
    "r0"  , "gp0" , "gp1" , "gp2" ,
    "gp3" , "gp4" , "gp5" , "gp6" ,
    "gp7" , "gp8" , "gp9" , "gp10",
    "gp11", "gp12", "gp13", "gp14",
    "a0"  , "a1"  , "a2"  , "fr0" ,
    "a3"  , "a4"  , "a5"  , "fr1" ,
    "lr0" , "lr1" , "lr2" , "lr3" ,
    "ir"  , "br"  , "sp"  , "pc"
};

std::unordered_map <std::string, int> hyrisc_register_map = {
    { "r0"  , r0   }, { "gp0" , gp0  }, { "gp1" , gp1  }, { "gp2" , gp2  },
    { "gp3" , gp3  }, { "gp4" , gp4  }, { "gp5" , gp5  }, { "gp6" , gp6  },
    { "gp7" , gp7  }, { "gp8" , gp8  }, { "gp9" , gp9  }, { "gp10", gp10 },
    { "gp11", gp11 }, { "gp12", gp12 }, { "gp13", gp13 }, { "gp14", gp14 },
    { "a0"  , a0   }, { "a1"  , a1   }, { "a2"  , a2   }, { "rr0" , rr0  },
    { "a3"  , a3   }, { "a4"  , a4   }, { "a5"  , a5   }, { "rr1" , rr1  },
    { "lr0" , lr0  }, { "lr1" , lr1  }, { "lr2" , lr2  }, { "lr3" , lr3  },
    { "ir"  , ir   }, { "br"  , br   }, { "sp"  , sp   }, { "pc"  , pc   }
};

enum hyrisc_opcodes_t : hyu8_t {
    HY_LOAD  = 0xff, HY_STORE = 0xfe, HY_ADD   = 0xef, HY_SUB   = 0xee,
    HY_MUL   = 0xed, HY_DIV   = 0xec, HY_AND   = 0xdf, HY_OR    = 0xde,
    HY_XOR   = 0xdd, HY_NOT   = 0xdc, HY_NEG   = 0xdb, HY_INC   = 0xcf,
    HY_DEC   = 0xce, HY_RST   = 0xcd, HY_TST   = 0xcc, HY_CMP   = 0xcb,
    HY_SLL   = 0xbf, HY_SLR   = 0xbe, HY_SAL   = 0xbd, HY_SAR   = 0xbc,
    HY_RL    = 0xbb, HY_RR    = 0xba, HY_JC    = 0xaf, HY_JP    = 0xae,
    HY_BC    = 0xad, HY_JR    = 0xac, HY_JALC  = 0xab, HY_JPL   = 0xaa,
    HY_CALLC = 0x9f, HY_CALL  = 0x9e, HY_RETC  = 0x9d, HY_RET   = 0x9c,
    HY_RTLC  = 0x9b, HY_PUSH  = 0x8f, HY_POP   = 0x8e, HY_NOP   = 0x7f,
    HY_LEA   = 0xfd
};

enum instruction_id_t : hyu8_t {
    ID_LOAD  = 0xff, ID_STORE = 0xfe, ID_ADD   = 0xef, ID_SUB   = 0xee,
    ID_MUL   = 0xed, ID_DIV   = 0xec, ID_AND   = 0xdf, ID_OR    = 0xde,
    ID_XOR   = 0xdd, ID_NOT   = 0xdc, ID_NEG   = 0xdb, ID_INC   = 0xcf,
    ID_DEC   = 0xce, ID_RST   = 0xcd, ID_TST   = 0xcc, ID_CMP   = 0xcb,
    ID_SLL   = 0xbf, ID_SLR   = 0xbe, ID_SAL   = 0xbd, ID_SAR   = 0xbc,
    ID_RL    = 0xbb, ID_RR    = 0xba, ID_JC    = 0xaf, ID_JP    = 0xae,
    ID_BC    = 0xad, ID_JR    = 0xac, ID_JALC  = 0xab, ID_JPL   = 0xaa,
    ID_CALLC = 0x9f, ID_CALL  = 0x9e, ID_RETC  = 0x9d, ID_RET   = 0x9c,
    ID_RTL   = 0x9b, ID_PUSH  = 0x8f, ID_POP   = 0x8e, ID_NOP   = 0x7f,
    ID_LEA   = 0xfd, ID_RTLC  = 0x9a, ID_LI    = 0xfc, ID_LUI   = 0xfb,
    ID_CMPZ  = 0xca
};

std::string hyrisc_mnemonics[] = {
    "l", "li", "lui",      // LOAD
    "lea",                 // LEA
    "s", "st",             // STORE
    "add", "addu", "adds", // ADD
    "sub",                 // SUB
    "mul",                 // MUL
    "div",                 // DIV
    "and",                 // AND
    "or",                  // OR
    "xor",                 // XOR
    "not",                 // NOT
    "neg",                 // NEG
    "inc",                 // INC
    "dec",                 // DEC
    "rst",                 // RST
    "tst",                 // TST
    "cmp",                 // CMP
    "sll",                 // SLL
    "slr",                 // SLR
    "sal",                 // SAL
    "sar",                 // SAR
    "rl",                  // RL
    "rr",                  // RR
    "jc",                  // JC
    "jp",                  // JP
    "bc",                  // BC
    "jr",                  // JR
    "jalc",                // JALC
    "jpl",                 // JPL
    "call",                // CALL
    "ret",                 // RET
    "rtl",                 // RTL
    "push",                // PUSH
    "pop",                 // POP
    "nop"                  // NOP
};

bool match(std::string src, std::string dst) {
    if (src.size() != dst.size()) return false;

    return src == dst;
}

enum encoding_t {
    REGX,                   // cmpz %r0
    REGX_REGY,              // l %r0, %r1
    REGX_REGY_REGZ,         // add %r0, %r1, %r2
    REGX_REGY_REGZ_REGW,    // rst %r0, %r1, %r2, %r3
    REGX_IMM8,              // s %r0, 0xab
    REGX_REGY_IMM8,         // add %r0, %r1, 0xab
    REGX_IMM16,             // add %r0, 0xdead
    MEMORY_INDEXED,         // sw %r0+(%r1*2), %r1
    FLOW_COND_IMM8,         // bne -4
    FLOW_COND_IMM16,        // bne 0xabcd
    FLOW_COND_REGISTER,     // jalne (%r1:4)+1
    FLOW_COND_INDEXED,      // jalne %r0+%r1:2
    NO_OPERAND_COND,        // rtleq
    NO_OPERAND
};

struct instruction_t {
    encoding_t encoding;
    hyu8_t opcode;
    int sub_opcode;
    hyu16_t imm16;
    int imm8_offset;
    hyu8_t imm8;
    int regx, regy, regz, regw;
    int shift_mul;
    int access_size;
    bool sign;
    int condition;
};

enum operand_type_t {
    OPERAND_INTEGER,
    OPERAND_REGISTER,
    OPERAND_SCALED_INDEX,
    OPERAND_SHIFTED_INDEX,
    OPERAND_MACRO
};

struct operand_t {
    std::string text;
    operand_type_t type;
    hyu32_t integer;
    int integer_size;
    int reg;
    int base_register;
    int index_register;
    int shift_mul;
};

encoding_t decode_encoding(std::vector <operand_t>& operands) {
    switch (operands.size()) {
        case 1: {
            switch (operands[0].type) {
                case OPERAND_INTEGER: { return FLOW_COND_IMM16; } break;
                case OPERAND_REGISTER: { return REGX; } break;
            }
        } break;

        case 2: {
            switch (operands[0].type) {
                case OPERAND_SCALED_INDEX: case OPERAND_SHIFTED_INDEX: {
                    return MEMORY_INDEXED;
                }
            }

            switch (operands[1].type) {
                case OPERAND_INTEGER: return REGX_IMM16;
                case OPERAND_REGISTER: return REGX_REGY;
                case OPERAND_SCALED_INDEX: case OPERAND_SHIFTED_INDEX: {
                    return MEMORY_INDEXED;
                }
            }
        } break;

        case 3: {
            switch (operands[2].type) {
                case OPERAND_REGISTER: return REGX_REGY_REGZ;
                case OPERAND_INTEGER: return REGX_REGY_IMM8;
            }
        }

        case 4: {
            return REGX_REGY_REGZ_REGW;
        }
    }
}

// CPU:
// Hyrisc™ HY-2 Primary CPU @ 24 MHz
// Hyrisc™ HY-1 Auxiliary CPU @ 12 MHz
// Video:
// Hyrisc™ DVSP-1 @ 60 Hz (Hyrisc Digital Video Signal Processor)
// Sound:
// Yamaha YMF292 + QSound Coprocessor or Sony SPU + Custom 65C816 Core

int decode_mnemonic(std::string mnemonic, instruction_t* instruction) {
    if (mnemonic == "l")    {
        instruction->access_size = 2;
        instruction->opcode - HY_LOAD;
        
        return ID_LOAD;
    }
    if (mnemonic == "li")   {
        instruction->encoding = REGX_IMM16;
        instruction->access_size = 2;
        instruction->opcode = HY_LOAD;
        instruction->sub_opcode = 0x6;

        return ID_LI;
    }
    if (mnemonic == "lui")   {
        instruction->encoding = REGX_IMM16;
        instruction->access_size = 2;
        instruction->opcode = HY_LOAD;
        instruction->sub_opcode = 0x5;

        return ID_LUI;
    }
    if (mnemonic == "lea")  { instruction->opcode = HY_LEA; return ID_LEA; }
    if (mnemonic == "s")    { instruction->opcode = HY_STORE; instruction->access_size = 2; return ID_STORE; }
    if (mnemonic == "sb")   { instruction->opcode = HY_STORE; instruction->access_size = 0; return ID_STORE; }
    if (mnemonic == "sh")   { instruction->opcode = HY_STORE; instruction->access_size = 1; return ID_STORE; }
    if (mnemonic == "sw")   { instruction->opcode = HY_STORE; instruction->access_size = 2; return ID_STORE; }
    if (mnemonic == "st")   { instruction->opcode = HY_STORE; instruction->access_size = 2; return ID_STORE; }
    if (mnemonic == "stb")  { instruction->opcode = HY_STORE; instruction->access_size = 0; return ID_STORE; }
    if (mnemonic == "sth")  { instruction->opcode = HY_STORE; instruction->access_size = 1; return ID_STORE; }
    if (mnemonic == "stw")  { instruction->opcode = HY_STORE; instruction->access_size = 2; return ID_STORE; }
    if (mnemonic == "add")  { instruction->opcode = HY_ADD; instruction->sign = false; return ID_ADD; }
    if (mnemonic == "addu") { instruction->opcode = HY_ADD; instruction->sign = false; return ID_ADD; }
    if (mnemonic == "adds") { instruction->opcode = HY_ADD; instruction->sign = true; return ID_ADD; }
    if (mnemonic == "sub")  { instruction->opcode = HY_SUB; instruction->sign = false; return ID_SUB; }
    if (mnemonic == "subu") { instruction->opcode = HY_SUB; instruction->sign = false; return ID_SUB; }
    if (mnemonic == "subs") { instruction->opcode = HY_SUB; instruction->sign = true; return ID_SUB; }
    if (mnemonic == "mul")  { instruction->opcode = HY_MUL; instruction->sign = false; return ID_MUL; }
    if (mnemonic == "mulu") { instruction->opcode = HY_MUL; instruction->sign = false; return ID_MUL; }
    if (mnemonic == "muls") { instruction->opcode = HY_MUL; instruction->sign = true; return ID_MUL; }
    if (mnemonic == "div")  { instruction->opcode = HY_DIV; instruction->sign = false; return ID_DIV; }
    if (mnemonic == "divu") { instruction->opcode = HY_DIV; instruction->sign = false; return ID_DIV; }
    if (mnemonic == "divs") { instruction->opcode = HY_DIV; instruction->sign = true; return ID_DIV; }
    if (mnemonic == "and")  { instruction->opcode = HY_AND; return ID_AND; }
    if (mnemonic == "or")   { instruction->opcode = HY_OR; return ID_OR; }
    if (mnemonic == "xor")  { instruction->opcode = HY_XOR; return ID_XOR; }
    if (mnemonic == "not")  { instruction->opcode = HY_NOT; return ID_NOT; }
    if (mnemonic == "neg")  { instruction->opcode = HY_NEG; return ID_NEG; }
    if (mnemonic == "inc")  { instruction->opcode = HY_INC; return ID_INC; }
    if (mnemonic == "dec")  { instruction->opcode = HY_DEC; return ID_DEC; }
    if (mnemonic == "rst")  { instruction->opcode = HY_RST; return ID_RST; }
    if (mnemonic == "tst")  { instruction->opcode = HY_TST; return ID_TST; }
    if (mnemonic == "cmp")  { instruction->opcode = HY_CMP; return ID_CMP; }
    if (mnemonic == "cmpz") { instruction->opcode = HY_CMP; return ID_CMPZ; }
    if (mnemonic == "sll")  { instruction->opcode = HY_SLL; return ID_SLL; }
    if (mnemonic == "slr")  { instruction->opcode = HY_SLR; return ID_SLR; }
    if (mnemonic == "sal")  { instruction->opcode = HY_SAL; return ID_SAL; }
    if (mnemonic == "sar")  { instruction->opcode = HY_SAR; return ID_SAR; }
    if (mnemonic == "rl")   { instruction->opcode = HY_RL; return ID_RL; }
    if (mnemonic == "rr")   { instruction->opcode = HY_RR; return ID_RR; }

    if (mnemonic == "jeq")  { instruction->opcode = HY_JC; instruction->condition = 0; return ID_JC; }
    if (mnemonic == "jne")  { instruction->opcode = HY_JC; instruction->condition = 1; return ID_JC; }
    if (mnemonic == "jcs")  { instruction->opcode = HY_JC; instruction->condition = 2; return ID_JC; }
    if (mnemonic == "jcc")  { instruction->opcode = HY_JC; instruction->condition = 3; return ID_JC; }
    if (mnemonic == "jmi")  { instruction->opcode = HY_JC; instruction->condition = 4; return ID_JC; }
    if (mnemonic == "jpl")  { instruction->opcode = HY_JC; instruction->condition = 5; return ID_JC; }
    if (mnemonic == "jvs")  { instruction->opcode = HY_JC; instruction->condition = 6; return ID_JC; }
    if (mnemonic == "jvc")  { instruction->opcode = HY_JC; instruction->condition = 7; return ID_JC; }
    if (mnemonic == "jhi")  { instruction->opcode = HY_JC; instruction->condition = 8; return ID_JC; }
    if (mnemonic == "jls")  { instruction->opcode = HY_JC; instruction->condition = 9; return ID_JC; }
    if (mnemonic == "jge")  { instruction->opcode = HY_JC; instruction->condition = 10; return ID_JC; }
    if (mnemonic == "jlt")  { instruction->opcode = HY_JC; instruction->condition = 11; return ID_JC; }
    if (mnemonic == "jgt")  { instruction->opcode = HY_JC; instruction->condition = 12; return ID_JC; }
    if (mnemonic == "jle")  { instruction->opcode = HY_JC; instruction->condition = 13; return ID_JC; }
    if (mnemonic == "j")    { instruction->opcode = HY_JC; instruction->condition = 14; return ID_JC; }
    if (mnemonic == "jp")   { instruction->opcode = HY_JC; instruction->condition = 14; return ID_JC; }
    if (mnemonic == "jmp")  { instruction->opcode = HY_JC; instruction->condition = 14; return ID_JC; }
    
    if (mnemonic == "beq")  { instruction->opcode = HY_BC; instruction->condition = 0; return ID_BC; }
    if (mnemonic == "bne")  { instruction->opcode = HY_BC; instruction->condition = 1; return ID_BC; }
    if (mnemonic == "bcs")  { instruction->opcode = HY_BC; instruction->condition = 2; return ID_BC; }
    if (mnemonic == "bcc")  { instruction->opcode = HY_BC; instruction->condition = 3; return ID_BC; }
    if (mnemonic == "bmi")  { instruction->opcode = HY_BC; instruction->condition = 4; return ID_BC; }
    if (mnemonic == "bpl")  { instruction->opcode = HY_BC; instruction->condition = 5; return ID_BC; }
    if (mnemonic == "bvs")  { instruction->opcode = HY_BC; instruction->condition = 6; return ID_BC; }
    if (mnemonic == "bvc")  { instruction->opcode = HY_BC; instruction->condition = 7; return ID_BC; }
    if (mnemonic == "bhi")  { instruction->opcode = HY_BC; instruction->condition = 8; return ID_BC; }
    if (mnemonic == "bls")  { instruction->opcode = HY_BC; instruction->condition = 9; return ID_BC; }
    if (mnemonic == "bge")  { instruction->opcode = HY_BC; instruction->condition = 10; return ID_BC; }
    if (mnemonic == "blt")  { instruction->opcode = HY_BC; instruction->condition = 11; return ID_BC; }
    if (mnemonic == "bgt")  { instruction->opcode = HY_BC; instruction->condition = 12; return ID_BC; }
    if (mnemonic == "ble")  { instruction->opcode = HY_BC; instruction->condition = 13; return ID_BC; }
    if (mnemonic == "b")    { instruction->opcode = HY_BC; instruction->condition = 14; return ID_BC; }
    if (mnemonic == "br")   { instruction->opcode = HY_BC; instruction->condition = 14; return ID_BC; }

    if (mnemonic == "jaleq")  { instruction->opcode = HY_JALC; instruction->condition = 0; return ID_JALC; }
    if (mnemonic == "jalne")  { instruction->opcode = HY_JALC; instruction->condition = 1; return ID_JALC; }
    if (mnemonic == "jalcs")  { instruction->opcode = HY_JALC; instruction->condition = 2; return ID_JALC; }
    if (mnemonic == "jalcc")  { instruction->opcode = HY_JALC; instruction->condition = 3; return ID_JALC; }
    if (mnemonic == "jalmi")  { instruction->opcode = HY_JALC; instruction->condition = 4; return ID_JALC; }
    if (mnemonic == "jalpl")  { instruction->opcode = HY_JALC; instruction->condition = 5; return ID_JALC; }
    if (mnemonic == "jalvs")  { instruction->opcode = HY_JALC; instruction->condition = 6; return ID_JALC; }
    if (mnemonic == "jalvc")  { instruction->opcode = HY_JALC; instruction->condition = 7; return ID_JALC; }
    if (mnemonic == "jalhi")  { instruction->opcode = HY_JALC; instruction->condition = 8; return ID_JALC; }
    if (mnemonic == "jalls")  { instruction->opcode = HY_JALC; instruction->condition = 9; return ID_JALC; }
    if (mnemonic == "jalge")  { instruction->opcode = HY_JALC; instruction->condition = 10; return ID_JALC; }
    if (mnemonic == "jallt")  { instruction->opcode = HY_JALC; instruction->condition = 11; return ID_JALC; }
    if (mnemonic == "jalgt")  { instruction->opcode = HY_JALC; instruction->condition = 12; return ID_JALC; }
    if (mnemonic == "jalle")  { instruction->opcode = HY_JALC; instruction->condition = 13; return ID_JALC; }
    if (mnemonic == "jal")    { instruction->opcode = HY_JALC; instruction->condition = 14; return ID_JALC; }
    if (mnemonic == "jalr")   { instruction->opcode = HY_JALC; instruction->condition = 14; return ID_JALC; }

    if (mnemonic == "calleq")  { instruction->opcode = HY_CALLC; instruction->condition = 0; return ID_CALLC; }
    if (mnemonic == "callne")  { instruction->opcode = HY_CALLC; instruction->condition = 1; return ID_CALLC; }
    if (mnemonic == "callcs")  { instruction->opcode = HY_CALLC; instruction->condition = 2; return ID_CALLC; }
    if (mnemonic == "callcc")  { instruction->opcode = HY_CALLC; instruction->condition = 3; return ID_CALLC; }
    if (mnemonic == "callmi")  { instruction->opcode = HY_CALLC; instruction->condition = 4; return ID_CALLC; }
    if (mnemonic == "callpl")  { instruction->opcode = HY_CALLC; instruction->condition = 5; return ID_CALLC; }
    if (mnemonic == "callvs")  { instruction->opcode = HY_CALLC; instruction->condition = 6; return ID_CALLC; }
    if (mnemonic == "callvc")  { instruction->opcode = HY_CALLC; instruction->condition = 7; return ID_CALLC; }
    if (mnemonic == "callhi")  { instruction->opcode = HY_CALLC; instruction->condition = 8; return ID_CALLC; }
    if (mnemonic == "callls")  { instruction->opcode = HY_CALLC; instruction->condition = 9; return ID_CALLC; }
    if (mnemonic == "callge")  { instruction->opcode = HY_CALLC; instruction->condition = 10; return ID_CALLC; }
    if (mnemonic == "calllt")  { instruction->opcode = HY_CALLC; instruction->condition = 11; return ID_CALLC; }
    if (mnemonic == "callgt")  { instruction->opcode = HY_CALLC; instruction->condition = 12; return ID_CALLC; }
    if (mnemonic == "callle")  { instruction->opcode = HY_CALLC; instruction->condition = 13; return ID_CALLC; }
    if (mnemonic == "call")    { instruction->opcode = HY_CALLC; instruction->condition = 14; return ID_CALLC; }
    
    if (mnemonic == "reteq")  { instruction->opcode = HY_RETC; instruction->condition = 0; return ID_RETC; }
    if (mnemonic == "retne")  { instruction->opcode = HY_RETC; instruction->condition = 1; return ID_RETC; }
    if (mnemonic == "retcs")  { instruction->opcode = HY_RETC; instruction->condition = 2; return ID_RETC; }
    if (mnemonic == "retcc")  { instruction->opcode = HY_RETC; instruction->condition = 3; return ID_RETC; }
    if (mnemonic == "retmi")  { instruction->opcode = HY_RETC; instruction->condition = 4; return ID_RETC; }
    if (mnemonic == "retpl")  { instruction->opcode = HY_RETC; instruction->condition = 5; return ID_RETC; }
    if (mnemonic == "retvs")  { instruction->opcode = HY_RETC; instruction->condition = 6; return ID_RETC; }
    if (mnemonic == "retvc")  { instruction->opcode = HY_RETC; instruction->condition = 7; return ID_RETC; }
    if (mnemonic == "rethi")  { instruction->opcode = HY_RETC; instruction->condition = 8; return ID_RETC; }
    if (mnemonic == "retls")  { instruction->opcode = HY_RETC; instruction->condition = 9; return ID_RETC; }
    if (mnemonic == "retge")  { instruction->opcode = HY_RETC; instruction->condition = 10; return ID_RETC; }
    if (mnemonic == "retlt")  { instruction->opcode = HY_RETC; instruction->condition = 11; return ID_RETC; }
    if (mnemonic == "retgt")  { instruction->opcode = HY_RETC; instruction->condition = 12; return ID_RETC; }
    if (mnemonic == "retle")  { instruction->opcode = HY_RETC; instruction->condition = 13; return ID_RETC; }
    if (mnemonic == "ret")    { instruction->opcode = HY_RETC; instruction->condition = 14; return ID_RETC; }

    if (mnemonic == "rtleq")  { instruction->opcode = HY_RTLC; instruction->condition = 0; return ID_RTLC; }
    if (mnemonic == "rtlne")  { instruction->opcode = HY_RTLC; instruction->condition = 1; return ID_RTLC; }
    if (mnemonic == "rtlcs")  { instruction->opcode = HY_RTLC; instruction->condition = 2; return ID_RTLC; }
    if (mnemonic == "rtlcc")  { instruction->opcode = HY_RTLC; instruction->condition = 3; return ID_RTLC; }
    if (mnemonic == "rtlmi")  { instruction->opcode = HY_RTLC; instruction->condition = 4; return ID_RTLC; }
    if (mnemonic == "rtlpl")  { instruction->opcode = HY_RTLC; instruction->condition = 5; return ID_RTLC; }
    if (mnemonic == "rtlvs")  { instruction->opcode = HY_RTLC; instruction->condition = 6; return ID_RTLC; }
    if (mnemonic == "rtlvc")  { instruction->opcode = HY_RTLC; instruction->condition = 7; return ID_RTLC; }
    if (mnemonic == "rtlhi")  { instruction->opcode = HY_RTLC; instruction->condition = 8; return ID_RTLC; }
    if (mnemonic == "rtlls")  { instruction->opcode = HY_RTLC; instruction->condition = 9; return ID_RTLC; }
    if (mnemonic == "rtlge")  { instruction->opcode = HY_RTLC; instruction->condition = 10; return ID_RTLC; }
    if (mnemonic == "rtllt")  { instruction->opcode = HY_RTLC; instruction->condition = 11; return ID_RTLC; }
    if (mnemonic == "rtlgt")  { instruction->opcode = HY_RTLC; instruction->condition = 12; return ID_RTLC; }
    if (mnemonic == "rtlle")  { instruction->opcode = HY_RTLC; instruction->condition = 13; return ID_RTLC; }
    if (mnemonic == "rtl")    { instruction->opcode = HY_RTLC; instruction->condition = 14; return ID_RTLC; }
    
    if (mnemonic == "push") { instruction->opcode = HY_PUSH; return ID_PUSH; }
    if (mnemonic == "pop")  { instruction->opcode = HY_POP; return ID_POP; }
    if (mnemonic == "nop")  { instruction->opcode = HY_NOP; return ID_NOP; }
}

instruction_t decode_instruction(std::string mnemonic, std::vector <operand_t>& operands) {
    instruction_t instruction;

    std::memset(&instruction, 0, sizeof(instruction_t));

    int id = decode_mnemonic(mnemonic, &instruction);

    switch (id) {
        case ID_LOAD: { // l
            instruction.opcode = HY_LOAD;

            switch (decode_encoding(operands)) {
                case REGX_REGY: { // l r0, r1
                    instruction.encoding = REGX_REGY;
                    instruction.sub_opcode = 0x7;
                    instruction.regx = operands[0].reg;
                    instruction.regy = operands[1].reg;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = 0x6;
                    instruction.regx  = operands[0].reg;
                    instruction.imm16 = operands[1].integer;
                } break;

                case MEMORY_INDEXED: {
                    instruction.access_size = 2;
                    instruction.encoding = MEMORY_INDEXED;
                    instruction.sub_opcode = (operands[1].type == OPERAND_SHIFTED_INDEX) ? 0x4 : 0x3;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].base_register;
                    instruction.regz      = operands[1].index_register;
                    instruction.shift_mul = operands[1].shift_mul;
                } break;

                default: {
                    _log(error, "Unrecognized mode for LOAD");
                } break;
            }
        } break;

        case ID_LI: { // li
            instruction.regx  = operands[0].reg;
            instruction.imm16 = operands[1].integer;
        } break;

        case ID_LUI: { // lui
            instruction.regx  = operands[0].reg;
            instruction.imm16 = operands[1].integer;
        } break;

        case 3: { // lea
            instruction.opcode = HY_LEA;

            switch (operands[1].type) {
                case OPERAND_SHIFTED_INDEX: instruction.sub_opcode = 7; break;
                case OPERAND_SCALED_INDEX : instruction.sub_opcode = 6; break;
            }

            instruction.encoding  = MEMORY_INDEXED;
            instruction.regx      = operands[0].reg;
            instruction.regy      = operands[1].base_register;
            instruction.regz      = operands[1].index_register;
            instruction.shift_mul = operands[1].shift_mul;
        } break;

        case ID_STORE: { // s
            instruction.opcode = HY_STORE;

            switch (decode_encoding(operands)) {
                case REGX_REGY: { // s r0, r1
                    instruction.encoding = MEMORY_INDEXED;
                    instruction.sub_opcode = 0x7;
                    instruction.regx = operands[0].reg;
                    instruction.regy = operands[1].reg;
                } break;

                case REGX_IMM16: {
                    instruction.access_size = 1;
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = 0x6;
                    instruction.regx  = operands[0].reg;
                    instruction.imm16 = operands[1].integer;
                } break;

                case MEMORY_INDEXED: {
                    instruction.encoding = MEMORY_INDEXED;
                    instruction.sub_opcode = (operands[1].type == OPERAND_SHIFTED_INDEX) ? 0x4 : 0x3;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].base_register;
                    instruction.regz      = operands[1].index_register;
                    instruction.shift_mul = operands[1].shift_mul;
                } break;

                default: {
                    _log(error, "Unrecognized mode for STORE");
                } break;
            }
        } break;

        case ID_ADD: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = instruction.sign ? 4 : 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = instruction.sign ? 3 : 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;
    
        case ID_SUB: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = instruction.sign ? 4 : 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = instruction.sign ? 3 : 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;

        case ID_MUL: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = instruction.sign ? 4 : 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = instruction.sign ? 3 : 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;

        case ID_DIV: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = instruction.sign ? 4 : 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = instruction.sign ? 3 : 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;

        case ID_AND: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;

        case ID_OR: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;

        case ID_XOR: {
            switch (decode_encoding(operands)) {
                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 7;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.regz      = operands[2].reg;
                } break;

                case REGX_REGY_IMM8: {
                    instruction.encoding = REGX_REGY_IMM8;
                    instruction.sub_opcode = 6;
                    instruction.regx      = operands[0].reg;
                    instruction.regy      = operands[1].reg;
                    instruction.imm8      = operands[2].integer;
                    instruction.imm8_offset = 21;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = 5;
                    instruction.regx      = operands[0].reg;
                    instruction.imm16     = operands[1].integer;
                } break;
            }
        } break;

        case ID_NOT: {
            instruction.encoding = REGX_REGY;
            instruction.sub_opcode = 7;
            instruction.regx     = operands[0].reg;
            instruction.regy     = operands[1].reg;
        } break;
        
        case ID_NEG: {
            instruction.encoding = REGX_REGY;
            instruction.sub_opcode = 7;
            instruction.regx     = operands[0].reg;
            instruction.regy     = operands[1].reg;
        } break;

        case ID_INC: {
            instruction.encoding = REGX;
            instruction.sub_opcode = 7;
            instruction.regx     = operands[0].reg;
        } break;

        case ID_DEC: {
            instruction.encoding = REGX;
            instruction.sub_opcode = 7;
            instruction.regx     = operands[0].reg;
        } break;

        case ID_RST: {
            switch (decode_encoding(operands)) {
                case REGX: {
                    instruction.encoding = REGX;
                    instruction.sub_opcode = 7;
                    instruction.regx = operands[0].reg;
                } break;

                case REGX_REGY: {
                    instruction.encoding = REGX_REGY;
                    instruction.sub_opcode = 6;
                    instruction.regx = operands[0].reg;
                    instruction.regy = operands[1].reg;
                } break;

                case REGX_REGY_REGZ: {
                    instruction.encoding = REGX_REGY_REGZ;
                    instruction.sub_opcode = 5;
                    instruction.regx = operands[0].reg;
                    instruction.regy = operands[1].reg;
                    instruction.regz = operands[2].reg;
                } break;

                case REGX_REGY_REGZ_REGW: {
                    instruction.encoding = REGX_REGY_REGZ_REGW;
                    instruction.sub_opcode = 4;
                    instruction.regx = operands[0].reg;
                    instruction.regy = operands[1].reg;
                    instruction.regz = operands[2].reg;
                    instruction.regw = operands[3].reg;
                } break;
            }
        } break;

        case ID_TST: {
            instruction.encoding = REGX_REGY;
            instruction.sub_opcode = 7;
            instruction.regx = operands[0].reg;
            instruction.regy = operands[1].integer;
        } break;

        case ID_CMPZ: {
            instruction.encoding = REGX;
            instruction.sub_opcode = 7;
            instruction.regx = operands[0].reg;
        } break;

        case ID_CMP: {
            switch (decode_encoding(operands)) {
                case REGX_REGY: {
                    instruction.encoding = REGX_REGY;
                    instruction.sub_opcode = 6;
                    instruction.regx = operands[0].reg;
                    instruction.regy = operands[1].reg;
                } break;

                case REGX_IMM8: {
                    instruction.encoding = REGX_IMM8;
                    instruction.sub_opcode = 5;
                    instruction.regx = operands[0].reg;
                    instruction.imm8 = operands[1].integer;
                    instruction.imm8_offset = 16;
                } break;

                case REGX_IMM16: {
                    instruction.encoding = REGX_IMM16;
                    instruction.sub_opcode = 4;
                    instruction.regx = operands[0].reg;
                    instruction.imm16 = operands[1].integer;
                } break;
            }
        } break;

        case ID_JC: {
            switch (decode_encoding(operands)) {
                case FLOW_COND_IMM16: {
                    instruction.encoding = FLOW_COND_IMM16;
                    instruction.imm16 = operands[0].integer;
                } break;

                // To-do
            }
        } break;

        case ID_BC: {
            switch (decode_encoding(operands)) {
                case FLOW_COND_IMM16: {
                    instruction.encoding = FLOW_COND_IMM16;
                    instruction.imm16 = operands[0].integer;
                } break;

                // To-do
            }
        } break;

        case ID_JALC: {
            switch (decode_encoding(operands)) {
                case FLOW_COND_IMM16: {
                    instruction.encoding = FLOW_COND_IMM16;
                    instruction.imm16 = operands[0].integer;
                } break;

                // To-do
            }
        } break;

        case ID_RTLC: {
            switch (decode_encoding(operands)) {
                case FLOW_COND_IMM16: {
                    instruction.encoding = FLOW_COND_IMM16;
                    instruction.imm16 = operands[0].integer;
                } break;

                // To-do
            }
        } break;
    }

    return instruction;
}

hyu32_t assemble_instruction(instruction_t instruction) {
    hyu32_t opcode = (hyu32_t)0x0 | instruction.opcode | (instruction.sub_opcode << 8),
            regx  = (instruction.regx & 0x1f) << 11,
            regy  = (instruction.regy & 0x1f) << 16,
            regz  = (instruction.regz & 0x1f) << 21,
            regw  = (instruction.regw & 0x1f) << 26,
            imm8  = (instruction.imm8 & 0xff) << instruction.imm8_offset,
            imm16 = (instruction.imm16 & 0xffff) << 16,
            scale = (instruction.shift_mul & 0xf) << 26,
            size  = (instruction.access_size & 0x3) << 30,
            cond  = (instruction.condition & 0xf) << 11;
    
    // _log(debug, "\nopcode=%08x\nregx=%08x\nregy=%08x\nregz=%08x\nscale=%08x\nsize=%08x\n",
    //     opcode,
    //     regx,
    //     regy,
    //     regz,
    //     scale,
    //     size
    // );

    switch (instruction.encoding) {
        case REGX               : return opcode | regx;
        case REGX_IMM16         : return opcode | regx | imm16;
        case REGX_IMM8          : return opcode | regx | imm8;
        case REGX_REGY          : return opcode | regx | regy;
        case REGX_REGY_IMM8     : return opcode | regx | regy | imm8;
        case REGX_REGY_REGZ     : return opcode | regx | regy | regz;
        case REGX_REGY_REGZ_REGW: return opcode | regx | regy | regz | regw;
        case MEMORY_INDEXED     : return opcode | regx | regy | regz | scale | size;
        case FLOW_COND_IMM16    : return opcode | cond | imm16;
        default: _log(error, "Unsupported encoding %u", instruction.encoding);
    }
}

#define ERROR_DEFAULT 0xffffffff;

enum assembly_error_t {
    E_OK,
    E_STREAM_EMPTY,
    E_EXPECTED_MNEMONIC,
    E_EXPECTED_OPERANDS
};

struct value_t {
    std::string name;
    hyu32_t value;
};

struct hyrisc_assembler_t {
    assembly_error_t error;
    hyu32_t pos;

    std::vector <value_t> labels;
    std::vector <value_t> values;
};

void skip_whitespace(char* c, std::istream* input) {
    while (std::isblank(*c))
        *c = input->get();
}

std::string parse_mnemonic(char* c, std::istream* input) {
    std::string mnemonic;

    while (std::isalpha(*c)) {
        mnemonic.append(1, *c);

        *c = input->get();
    }

    skip_whitespace(c, input);

    return mnemonic;
}

hyu32_t assemble_no_operand(std::string mnemonic, hyrisc_assembler_t* state) {
    if (mnemonic == "nop") return 0xffffff00 | HY_NOP;

    if (state) state->error = E_EXPECTED_OPERANDS;

    return ERROR_DEFAULT;
}

std::string parse_operand(char* c, std::istream* input) {
    std::string operand;

    while ((*c != ',') && (*c != '\n') && (*c != ';') && (!input->eof())) {
        operand.append(1, *c);

        *c = input->get();
    }

    return trim(operand);
}

inline bool is_terminator(char c) {
    return (c == '\n') || (c == ';') || (c == -1);
}

hyu32_t parse_integer(char* c, std::istream* input) {
    bool negative = false;

    if (*c == '-') {
        negative = true;

        *c = input->get();
    }

    std::string integer;

    switch (input->peek()) {
        // Hex
        case 'x': {
            // integer = "0x";
            input->get();
            *c = input->get();

            while (std::isxdigit(*c)) {
                integer.append(1, *c);

                *c = input->get();
            }

            hyu32_t value = std::stoul(integer, nullptr, 16);

            return negative ? -value : value;
        } break;

        // Binary
        case 'b': {
            input->get();
            *c = input->get();

            while (*c == '0' || *c == '1') {
                integer.append(1, *c);

                *c = input->get();
            }

            hyu32_t value = std::stoul(integer, nullptr, 2);

            return negative ? -value : value;
        }
        
        // Treat as decimal
        default: {
            while (std::isdigit(*c)) {
                integer.append(1, *c);

                *c = input->get();
            }

            hyu32_t value = std::stoul(integer, nullptr, 10);

            return negative ? -value : value;
        } break;
    }
}

int parse_register_number(char* c, std::istream* input) {
    if (*c == 'r' && std::isdigit(input->peek())) {
        std::string number;

        *c = input->get();

        while (std::isdigit(*c)) {
            number.append(1, *c);

            *c = input->get();
        }

        return std::stoi(number);
    } else {
        std::string name;

        while (std::isalnum(*c)) {
            name.append(1, *c);

            *c = input->get();
        }

        return hyrisc_register_map[trim(name)];
    }
}

operand_t parse_register(char* c, std::istream* input) {
    operand_t operand;

    if (*c == '%')
        *c = input->get();

    operand.reg = parse_register_number(c, input);

    if (input->eof()) {
        operand.type = OPERAND_REGISTER;

        return operand;
    }

    operand.base_register = operand.reg;

    skip_whitespace(c, input);

    if (*c != '+') {
        // Indexed mode only supports adding

        return operand;
    }

    // Consume +
    *c = input->get();

    skip_whitespace(c, input);

    bool parent = false;

    if (*c == '(') {
        parent = true;

        // Consume parent
        *c = input->get();
    }

    skip_whitespace(c, input);

    if (*c == '%')
        *c = input->get();

    operand.index_register = parse_register_number(c, input);

    skip_whitespace(c, input);

    if (*c == ':') {
        operand.type = OPERAND_SHIFTED_INDEX;
    } else if (*c == '*') {
        operand.type = OPERAND_SCALED_INDEX;
    }

    *c = input->get();

    skip_whitespace(c, input);

    operand.shift_mul = parse_integer(c, input);

    if (parent) {
        skip_whitespace(c, input);

        if (*c != ')') {
            // Expected closing parenthesis

            return operand;
        }
    }

    return operand;
}

operand_t decode_operand(std::string text, hyrisc_assembler_t* state) {
    operand_t operand;

    operand.text = text;

    std::istringstream stream(text);

    char c = stream.get();

    if (c == '.') {
        if (!state) {
            // Can't use labels or defines when there's no state
        }

        operand.type = OPERAND_INTEGER;

        c = stream.get();

        std::string name;

        while (std::isalnum(c) || (c == '_')) {
            name.append(1, c);

            c = stream.get();
        }

        auto label_it = std::find_if(
            std::begin(state->labels),
            std::end(state->labels),
            [name] (value_t value) { return value.name == name; }
        );

        if (label_it != std::end(state->labels)) {
            // Is label
            operand.integer = (*label_it).value - (state->pos + 4);

            return operand;
        }

        auto values_it = std::find_if(
            std::begin(state->values),
            std::end(state->values),
            [name] (value_t value) { return value.name == name; }
        );

        if (values_it != std::end(state->values)) {
            // Is value
            operand.integer = (*values_it).value;

            return operand;
        }

        // else couldn't find symbol
    }

    if (std::isdigit(c) || (c == '-')) {
        operand.type = OPERAND_INTEGER;
        operand.integer = parse_integer(&c, &stream);
        operand.integer_size = 8;

        if (operand.integer > 0xff) operand.integer_size = 16;

        return operand;
    }

    operand_t reg = parse_register(&c, &stream);

    // _log(debug, "type=%u, register=%u, base=%u, index=%u, shift_mul=%u", 
    //     reg.type,
    //     reg.reg,
    //     reg.base_register,
    //     reg.index_register,
    //     reg.shift_mul
    // );

    reg.text = operand.text;

    return reg;
}

hyu32_t assemble(char& c, std::istream* input, hyrisc_assembler_t* state = nullptr) {
    if (input->eof()) {
        if (state) state->error = E_STREAM_EMPTY;
    }

    // Ignore leading whitespace
    while (std::isblank(c) || std::isspace(c) || (c == '\x0d') || (c == '\x0a'))
        c = input->get();

    if (!std::isalpha(c)) {
        if (state) state->error = E_EXPECTED_MNEMONIC;

        return 0x0;
    }

    std::string mnemonic = parse_mnemonic(&c, input);

    for (char& c : mnemonic)
        c = std::tolower(c);

    if (is_terminator(c)) {
        return assemble_no_operand(mnemonic, state);
    }

    std::vector <operand_t> operands;

    while (true) {
        std::string text = parse_operand(&c, input);

        operands.push_back(decode_operand(text, state));
        
        //_log(debug, "c=%s", operands.back().text.c_str());
        //_log(debug, "c=%c (%i)", c, (int)c);

        if (is_terminator(c)) break;

        // Consume operand separator
        c = input->get();
    }

    instruction_t instruction = decode_instruction(mnemonic, operands);

    // _log(debug, "i.encoding=%u\ni.regx=%u\ni.regy=%u\ni.regz=%u\ni.regw=%u\ni.shift_mul=%u\ni.imm16=%u\ni.sub_opcode=%u",
    //     instruction.encoding,
    //     instruction.regx,
    //     instruction.regy,
    //     instruction.regz,
    //     instruction.regw,
    //     instruction.shift_mul,
    //     instruction.imm16,
    //     instruction.sub_opcode
    // );

    return assemble_instruction(instruction);
}

bool parse_preprocessor(char* c, std::istream* input, hyrisc_assembler_t* state) {
    // Ignore leading whitespace
    while (std::isblank(*c) || std::isspace(*c) || (*c == '\x0d') || (*c == '\x0a'))
        *c = input->get();

    if (*c != '.') return false;

    *c = input->get();

    std::string command;

    while (std::isalnum(*c) || (*c == '_')) {
        command.append(1, *c);

        *c = input->get();
    }

    if (std::isblank(*c)) {
        // Parse command
        if (command == "org") {
            while (std::isspace(*c))
                *c = input->get();
        
            if (!std::isdigit(*c)) {
                // Expected number after org
            }

            hyu32_t addr = parse_integer(c, input);

            _log(debug, "Setting origin to %08x", addr);

            state->pos = addr;
        }
        
        if (command == "def") {
            while (std::isspace(*c))
                *c = input->get();
        
            if (!(std::isalpha(*c) || (*c == '_'))) {
                // Expected number after org
            }

            std::string name;

            while (std::isalnum(*c) || (*c == '_')) {
                name.append(1, *c);

                *c = input->get();
            }

            while (std::isspace(*c))
                *c = input->get();
            
            if (!std::isdigit(*c)) {
                // Expected number after define name
            }

            hyu32_t value = parse_integer(c, input);

            // Ignore trailing whitespace
            while (std::isblank(*c) || std::isspace(*c) || (*c == '\x0d') || (*c == '\x0a'))
                *c = input->get();
            
            state->values.push_back(value_t{ name, value });

            _log(debug, "Creating value %s:%08x", name.c_str(), value);
        }

        if (command == "undef") {
            while (std::isspace(*c))
                *c = input->get();
        
            if (!(std::isalpha(*c) || (*c == '_'))) {
                // Expected number after org
            }

            std::string name;

            while (std::isalnum(*c) || (*c == '_')) {
                name.append(1, *c);

                *c = input->get();
            }

            auto value_it = std::find_if(
                std::begin(state->values),
                std::end(state->values),
                [name] (value_t value) { return value.name == name; }
            );

            if (value_it != std::end(state->values)) {
                _log(debug, "undeffing %s", (*value_it).name.c_str());
                state->values.erase(value_it);
            }

            auto label_it = std::find_if(
                std::begin(state->labels),
                std::end(state->labels),
                [name] (value_t value) { return value.name == name; }
            );

            if (label_it != std::end(state->labels)) {
                _log(debug, "undeffing %s", (*label_it).name.c_str());
                state->values.erase(label_it);
            }
        }

        // Ignore trailing whitespace
        while (std::isblank(*c) || std::isspace(*c) || (*c == '\x0d') || (*c == '\x0a'))
            *c = input->get();
        
        return *c == '.';

    } else if (*c == ':') {
        // Label
        state->labels.push_back(value_t{ command, state->pos });
        _log(debug, "label=%s:%08x", command.c_str(), state->pos);
        *c = input->get();
    }
}

void assemble_input(std::istream* input, std::ostream* output, hyrisc_assembler_t* state = nullptr) {
    char c = input->get();

    while (c != -1) {
        while (parse_preprocessor(&c, input, state));

        hyu32_t opcode = assemble(c, input, state);

        if (state) state->pos += 4;

        output->write((char*)&opcode, sizeof(hyu32_t));

        while (std::isspace(c))
            c = input->get();
    }
}

int main(int argc, const char* argv[]) {
    _log::init("hyrisc-a");

    std::ofstream output("test.bin", std::ios::binary);

    std::ifstream input(std::string(argv[1]), std::ios::binary);

    hyrisc_assembler_t* state = new hyrisc_assembler_t;

    std::memset(state, 0, sizeof(hyrisc_assembler_t));

    _log(debug, "Assembling %s", argv[1]);

    assemble_input(&input, &output, state);

    _log(debug, "Done");
}