#include "cpu/instr.h"

make_instr_func(jmp_near)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = data_size;
        rel.addr = eip + 1;

        operand_read(&rel);

        int offset = sign_ext(rel.val, data_size);
        // thank Ting Xu from CS'17 for finding this bug
        print_asm_1("jmp", "", 1 + data_size / 8, &rel);

        cpu.eip += offset;

        return 1 + data_size / 8;
}

make_instr_func(jmp_short_)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = 8;
        rel.addr = eip + 1;

        operand_read(&rel);

        int offset = sign_ext(rel.val, rel.data_size);
        print_asm_1("jmp", "", 1 + data_size / 8, &rel);

        cpu.eip += offset;

        return 1 + rel.data_size / 8;
}

make_instr_func(jmp_near_rm_v)
{
        int len = 1;
        decode_data_size_v
        decode_operand_rm
        operand_read(&opr_src);

        int offset = sign_ext(opr_src.val, opr_src.data_size);
        cpu.eip = offset;

        return 0;
}

make_instr_func(jmp_far_imm)
{
        OPERAND EIP;
        EIP.type = OPR_IMM;
        EIP.data_size = 32;
        EIP.addr = eip + 1;
        operand_read(&EIP);

        OPERAND CS;
        CS.type = OPR_IMM;
        CS.data_size = 16;
        CS.addr = eip + 5;
        operand_read(&CS);
        
        cpu.cs.val = CS.val;
        cpu.eip = EIP.val;
        
        load_sreg(1);
        
        return 0;
}
