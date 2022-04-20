#include "common.h"
#include "debug.h"
#include "local-include/reg.h"

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <stdint.h>
#include <stdio.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

///
///@brief Types of instruction
///
enum {
    TYPE_R,
    TYPE_I,
    TYPE_S,
    TYPE_B,
    TYPE_U,
    TYPE_J
};

///
///@brief Extract operands rd, rs1, rs2 from instruction s->isa.inst.val
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@param rd_pptr Pointer to pointer to rd register \p rd
///@param rs1_pptr Pointer to pointer to rs1 register \p rs1
///@param rs2_pptr Pointer to pointer to rs2 register \p rs2
///@param imm_ptr Pointer to immediate value \p imm
///@param type Type of the instruction
///
static void decode_operand(Decode *s, word_t **rd_pptr, word_t **rs1_pptr, word_t **rs2_pptr, word_t *imm_ptr, int type) {
    uint32_t i = s->isa.inst.val;
    int rd = BITS(i, 11, 7);
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    switch (type) {
        case TYPE_R:
            *rd_pptr = &R(rd);
            *rs1_pptr = &R(rs1);
            *rs2_pptr = &R(rs2);
            break;
        case TYPE_I:
            *rd_pptr = &R(rd);
            *rs1_pptr = &R(rs1);
            *imm_ptr = SEXT(BITS(i, 31, 20), 12);
            break;
        case TYPE_S:
            *rs1_pptr = &R(rs1);
            *rs2_pptr = &R(rs2);
            *imm_ptr = SEXT(BITS(i, 31, 25), 7) << 5 | BITS(i, 11, 7);
            break;
        case TYPE_B:
            *rs1_pptr = &R(rs1);
            *rs2_pptr = &R(rs2);
            *imm_ptr = SEXT(BITS(i, 31, 31), 1) << 12 | BITS(i, 30, 25) << 5 | BITS(i, 11, 8) << 1 | BITS(i, 7, 7) << 11;
            break;
        case TYPE_U:
            *rd_pptr = &R(rd);
            *imm_ptr = SEXT(BITS(i, 31, 12), 20) << 12;
            break;
        case TYPE_J:
            *rd_pptr = &R(rd);
            *imm_ptr = (SEXT(BITS(i, 31, 31), 1) << 20 | BITS(i, 30, 21) << 1 | BITS(i, 20, 20) << 11 | BITS(i, 19, 12) << 12);
            break;
        default:
            Assert(0, "Error instruction type\n");
    }
}

///
///@brief instruction of \p s (uint32_t)
///
///@param s Pointer to instance of \p Decode struct of current instruction
///
#define INSTPAT_INST(s) ((s)->isa.inst.val)

#define rd *rd_ptr
#define rs1 *rs1_ptr
#define rs2 *rs2_ptr

///
///@brief Execute operation for current instruction
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@param type Type of the instruction
///@param body Operations of the instruction
///
#define INSTPAT_MATCH(s, type, ... /* body */)                                     \
    {                                                                              \
        decode_operand(s, &rd_ptr, &rs1_ptr, &rs2_ptr, &imm, concat(TYPE_, type)); \
        __VA_ARGS__;                                                               \
    }

///
///@brief Actually decode \b and execute the instruction
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@return Always return 0, useless for now
///
static int decode_exec(Decode *s) {
    word_t rd, rs1, rs2, imm;

    s->dnpc = s->snpc; // For normal instructions, dynamic next pc should be same with staticc next pc

    INSTPAT_START();

    // Note: only 0/1/? are allowed in pattern string, 0/1 only matches 0/1, ? matches 0 or 1
    // spaces, │ in pattern string are ignored

    // RV32I Base Integer Instructions
    // R-type │funct7      │rs2  │rs1  │funct3│rd         │opcode │
    INSTPAT(L"│0000000     │?????│?????│000   │?????      │0110011│", R, rd = rs1 + rs2);           // add
    INSTPAT(L"│0100000     │?????│?????│000   │?????      │0110011│", R, rd = rs1 - rs2);           // sub
    INSTPAT(L"│0000000     │?????│?????│001   │?????      │0110011│", R, rd = rs1 << rs2);          // sll
    INSTPAT(L"│0000000     │?????│?????│011   │?????      │0110011│", R, rd = (rs1 < rs2) ? 1 : 0); // sltu
    INSTPAT(L"│0000000     │?????│?????│100   │?????      │0110011│", R, rd = rs1 ^ rs2);           // xor
    INSTPAT(L"│0000000     │?????│?????│101   │?????      │0110011│", R, rd = rs1 >> rs2);          // srl
    INSTPAT(L"│0100000     │?????│?????│101   │?????      │0110011│", R, TODO());                   // sra
    INSTPAT(L"│0000000     │?????│?????│110   │?????      │0110011│", R, rd = rs1 | rs2);           // or
    INSTPAT(L"│0000000     │?????│?????│111   │?????      │0110011│", R, rd = rs1 & rs2);           // and
    // I-type │imm[11:0]         │rs1  │funct3│rd         │opcode │
    INSTPAT(L"│????????????      │?????│000   │?????      │1100111│", I, rd = s->pc + 4; s->dnpc = rs1 + imm;); // jalr
    INSTPAT(L"│????????????      │?????│000   │?????      │0000011│", I, rd = SEXT(Mr(rs1 + imm, 1), 64));      // lb
    INSTPAT(L"│????????????      │?????│001   │?????      │0000011│", I, rd = SEXT(Mr(rs1 + imm, 2), 64));      // lh
    INSTPAT(L"│????????????      │?????│010   │?????      │0000011│", I, rd = SEXT(Mr(rs1 + imm, 4), 64));      // lw
    INSTPAT(L"│????????????      │?????│100   │?????      │0000011│", I, rd = Mr(rs1 + imm, 1));                // lbu
    INSTPAT(L"│????????????      │?????│101   │?????      │0000011│", I, rd = Mr(rs1 + imm, 2));                // lhu
    INSTPAT(L"│????????????      │?????│000   │?????      │0010011│", I, rd = rs1 + imm);                       // addi
    INSTPAT(L"│????????????      │?????│011   │?????      │0010011│", I, rd = (rs1 < imm) ? 1 : 0);             // sltiu
    INSTPAT(L"│????????????      │?????│100   │?????      │0010011│", I, rd = rs1 ^ imm);                       // xori
    INSTPAT(L"│????????????      │?????│110   │?????      │0010011│", I, rd = rs1 | imm);                       // ori
    INSTPAT(L"│????????????      │?????│111   │?????      │0010011│", I, rd = rs1 & imm);                       // andi
    INSTPAT(L"│0000000?????      │?????│001   │?????      │0010011│", I, rd = rs1 << BITS(imm, 4, 0));          // slli
    INSTPAT(L"│0100000?????      │?????│101   │?????      │0010011│", I, TODO());                               // srai
    INSTPAT(L"│000000000001      │00000│000   │00000      │1110011│", I, NEMUTRAP(s->pc, R(10)));               // ebreak, Note: R(10) is $a0
    // S-type │imm[11:5]   │rs2  │rs1  │funct3│imm[4:0]   │opcode │
    INSTPAT(L"│???????     │?????│?????│000   │?????      │0100011│", S, Mw(rs1 + imm, 1, BITS(rs2, 7, 0)));  // sb
    INSTPAT(L"│???????     │?????│?????│001   │?????      │0100011│", S, Mw(rs1 + imm, 2, BITS(rs2, 15, 0))); // sh
    INSTPAT(L"│???????     │?????│?????│010   │?????      │0100011│", S, Mw(rs1 + imm, 4, BITS(rs2, 31, 0))); // sw
    // B-type │imm[12|10:5]│rs2  │rs1  │funct3│imm[4:1|11]│opcode │
    INSTPAT(L"│???????     │?????│?????│000   │?????      │1100011│", B, if (rs1 == rs2) { s->dnpc = s->pc + imm; }); // beq
    INSTPAT(L"│???????     │?????│?????│001   │?????      │1100011│", B, if (rs1 != rs2) { s->dnpc = s->pc + imm; }); // bne
    // U-type │imm[31:12]                     │rd         │opcode │
    INSTPAT(L"│????????????????????           │?????      │0010111│", U, rd = s->pc + imm;);                      // auipc
    // J-type │imm[20|10:1|11|19:12]          │rd         │opcode │
    INSTPAT(L"│????????????????????           │?????      │1101111│", J, rd = s->pc + 4; s->dnpc = s->pc + imm;); // jal

    // RV64I Base Integer Instructions
    // R-type │funct7      │rs2  │rs1  │funct3│rd         │opcode │
    INSTPAT(L"│0000000     │?????│?????│000   │?????      │0111011│", R, rd = SEXT(BITS(rs1 + rs2, 31, 0), 64));              // addw
    INSTPAT(L"│0100000     │?????│?????│000   │?????      │0111011│", R, rd = SEXT(BITS(rs1 - rs2, 31, 0), 64));              // subw
    INSTPAT(L"│0000000     │?????│?????│001   │?????      │0111011│", R, rd = SEXT(BITS(rs1 << BITS(rs2, 4, 0), 31, 0), 64)); // sllw
    INSTPAT(L"│0000000     │?????│?????│001   │?????      │0111011│", R, rd = SEXT(BITS(rs1, 31, 0) >> BITS(rs2, 4, 0), 64)); // srlw
    // I-type │imm[11:0]         │rs1  │funct3│rd         │opcode │
    INSTPAT(L"│????????????      │?????│011   │?????      │0000011│", I, rd = Mr(rs1 + imm, 8));                 // ld
    INSTPAT(L"│????????????      │?????│000   │?????      │0011011│", I, rd = SEXT(BITS(rs1 + imm, 31, 0), 64)); // addiw
    // S-type │imm[11:5]   │rs2  │rs1  │funct3│imm[4:0]   │opcode │
    INSTPAT(L"│???????     │?????│?????│011   │?????      │0100011│", S, Mw(rs1 + imm, 8, rs2)); // sd
    // B-type │imm[12|10:5]│rs2  │rs1  │funct3│imm[4:1|11]│opcode │
    // U-type │imm[31:12]                     │rd         │opcode │
    // J-type │imm[20|10:1|11|19:12]          │rd         │opcode │

    // If an instruction does not match any pattern of above, it is invalid
    INSTPAT(L"│????????????      │?????│???   │?????      │???????│", I, INV(s->pc));

    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    return 0;
}
#undef rd
#undef rs1
#undef rs2

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
