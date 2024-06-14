#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/
make_instr_func(call_near)
{
        
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.sreg = SREG_CS;
        rel.data_size = data_size;
        rel.addr = eip + 1;
        
        operand_read(&rel);

        cpu.eip += (1 + data_size / 8);
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_DS, 4, cpu.eip);

        int offset = sign_ext(rel.val, data_size);
        print_asm_1("call", "", 1 + data_size / 8, &rel);
        
        cpu.eip += offset;
        
        return 0;
}


make_instr_func(call_near_indirect)
{
        int len = 1;
        opr_src.data_size = data_size;
        len += modrm_rm(eip + 1, &opr_src);
        operand_read(&opr_src);
        
        cpu.esp -= 4;
        vaddr_write(cpu.esp, SREG_DS, 4, cpu.eip+len);
        
        int offset = sign_ext(opr_src.val, opr_src.data_size);
        cpu.eip = offset;
        
        return 0;
}

