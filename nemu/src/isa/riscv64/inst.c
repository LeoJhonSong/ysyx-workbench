#include "local-include/reg.h"

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

///
///@brief Types of instruction
///
enum {
    TYPE_I,
    TYPE_U,
    TYPE_S,
    TYPE_N, // none
};

// usage of do while, see: https://stackoverflow.com/a/257425/10088906
#define src1R(n) do { *src1_ptr = R(n); } while (0)
#define src2R(n) do { *src2_ptr = R(n); } while (0)
#define destR(n) do { *dest_ptr = n; } while (0)
#define src1I(i) do { *src1_ptr = i; } while (0)
#define src2I(i) do { *src2_ptr = i; } while (0)
#define destI(i) do { *dest_ptr = i; } while (0)

// Get immediate value from instruction based on instruction type
static word_t immI(uint32_t i) { return SEXT(BITS(i, 31, 20), 12); }
static word_t immU(uint32_t i) { return SEXT(BITS(i, 31, 12), 20) << 12; }
static word_t immS(uint32_t i) { return (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); }

///
///@brief Extract operands rd, rs1, rs2 from instruction s->isa.inst.val to vars dest, src1, src2
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@param dest Stores operand \p rd
///@param src1 Stores operand \p rs1
///@param src2 Stores operand \p rs2
///@param type Type of the instruction
///
static void decode_operand(Decode *s, word_t *dest_ptr, word_t *src1_ptr, word_t *src2_ptr, int type) {
    uint32_t i = s->isa.inst.val;
    int rd = BITS(i, 11, 7);
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    destR(rd);
    switch (type) {
        case TYPE_I:
            src1R(rs1);
            src2I(immI(i));
            break;
        case TYPE_U:
            src1I(immU(i));
            break;
        case TYPE_S:
            destI(immS(i));
            src1R(rs1);
            src2R(rs2);
            break;
    }
}

///
///@brief instruction of \p s (uint32_t)
///
///@param s Pointer to instance of \p Decode struct of current instruction
///
#define INSTPAT_INST(s) ((s)->isa.inst.val)

///
///@brief Execute operation for current instruction
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@param name Name of the instruction
///@param type Type of the instruction
///@param body Operation of the instruction
///
#define INSTPAT_MATCH(s, name, type, ... /* body */)                 \
    {                                                                \
        decode_operand(s, &dest, &src1, &src2, concat(TYPE_, type)); \
        __VA_ARGS__;                                                 \
    }

///
///@brief Actually decode \b and execute the instruction
///
///@param s Pointer to instance of \p Decode struct of current instruction
///@return Always return 0, useless for now
///
static int decode_exec(Decode *s) {
    word_t dest = 0, src1 = 0, src2 = 0;
    s->dnpc = s->snpc; // For normal instructions, dynamic next pc should be same with staticc next pc

    INSTPAT_START();

    // INSTPAT(pattern string, instruction name, instruction type, instruction operation)
    // Note: only 0/1/? are allowed in pattern string, 0/1 only matches 0/1, ? matches 0 or 1
    // spaces in pattern string do not participate in matching
    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U, R(dest) = src1 + s->pc);
    INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld, I, R(dest) = Mr(src1 + src2, 8));
    INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd, S, Mw(src1 + dest, 8, src2));

    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));                // If an instruction does not match any pattern of above, it is invalid

    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
