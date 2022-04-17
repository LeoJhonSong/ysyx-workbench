#include "common.h"
#include "debug.h"
#include "local-include/reg.h"

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <stdint.h>

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

// usage of do while, see: https://stackoverflow.com/a/257425/10088906
#define src1R(n) do { *src1_ptr = R(n); } while (0)
#define src2R(n) do { *src2_ptr = R(n); } while (0)
#define destR(n) do { *dest_ptr = n; } while (0)
#define src1I(i) do { *src1_ptr = i; } while (0)
#define src2I(i) do { *src2_ptr = i; } while (0)
#define destI(i) do { *dest_ptr = i; } while (0)

///
///@brief Extract operands rd, rs1, rs2 from instruction s->isa.inst.val to vars dest, src1, src2
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@param dest Stores operand \p rd
///@param src1 Stores operand \p rs1
///@param src2 Stores operand \p rs2
///@param type Type of the instruction
///
static void decode_operand(Decode *s, word_t **rd_pptr, word_t **rs1_pptr, word_t **rs2_pptr, word_t *imm_ptr, int type) {
    uint32_t i = s->isa.inst.val;
    int rd = BITS(i, 11, 7);
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    switch (type) {
        case TYPE_R:
            TODO();
            break;
        case TYPE_I:
            *rd_pptr = &R(rd);
            *rs1_pptr = &R(rs1);
            *imm_ptr = SEXT(BITS(i, 31, 20), 12);
            break;
        case TYPE_S:
            *rs1_pptr = &R(rs1);
            *rs2_pptr = &R(rs2);
            *imm_ptr = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);
            break;
        case TYPE_B:
            TODO();
            break;
        case TYPE_U:
            *rd_pptr = &R(rd);
            *imm_ptr = SEXT(BITS(i, 31, 12), 20) << 12;
            break;
        case TYPE_J:
            *rd_pptr = &R(rd);
            *imm_ptr = (SEXT(BITS(i, 31, 31), 1) << 20 | SEXT(BITS(i, 30, 21), 10) << 1 | SEXT(BITS(i, 20, 20), 1) << 11 | SEXT(BITS(i, 19, 12), 8) << 12);
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
#define PC s->pc

///
///@brief Execute operation for current instruction
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@param type Type of the instruction
///@param name Name of the instruction
///@param body Operation of the instruction
///
#define INSTPAT_MATCH(s, type, name, ... /* body */)                               \
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

    // RV32I/RV64I Base Integer Instructions
    // R-type │funct7      │rs2  │rs1  │funct3│rd         │opcode │
    // I-type │imm[11:0]         │rs1  │funct3│rd         │opcode │
    INSTPAT(L"│????????????      │?????│000   │?????      │1100111│", I, jalr, TODO()); //TODO: remove name
    INSTPAT(L"│????????????      │?????│011   │?????      │0000011│", I, ld, rd = Mr(rs1 + imm, 8));
    INSTPAT(L"│????????????      │?????│000   │?????      │0010011│", I, addi, TODO());
    INSTPAT(L"│000000000001      │00000│000   │00000      │1110011│", I, ebreak, NEMUTRAP(PC, R(10))); // R(10) is $a0
    // S-type │imm[11:5]   │rs2  │rs1  │funct3│imm[4:0]   │opcode │
    INSTPAT(L"│???????     │?????│?????│011   │?????      │0100011│", S, sd, Mw(rs1 + imm, 8, rs2));
    // B-type │imm[12|10:5]│rs2  │rs1  │funct3│imm[4:1|11]│opcode │
    // U-type │imm[31:12]                     │rd         │opcode │
    INSTPAT(L"│????????????????????           │?????      │0010111│", U, auipc, rd = PC + imm);
    // J-type │imm[20|10:1|11|19:12]          │rd         │opcode │
    INSTPAT(L"│????????????????????           │?????      │1101111│", J, jal, TODO());

    // If an instruction does not match any pattern of above, it is invalid
    INSTPAT(L"│????????????      │?????│???   │?????      │???????│", I, inv, INV(s->pc));

    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    return 0;
}
#undef rd
#undef rs1
#undef rs2
#undef PC

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
