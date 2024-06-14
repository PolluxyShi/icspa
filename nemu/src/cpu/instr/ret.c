#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near)
{
    cpu.eip = vaddr_read(cpu.esp, SREG_DS, 4);
    cpu.esp += 4;
        
    return 0;
}

make_instr_func(ret_near_i_w)
{
    int len = 1;
    decode_data_size_w
    decode_operand_i
    
    cpu.eip = vaddr_read(cpu.esp, SREG_DS, 4);
    cpu.esp += 4 ;
    
    operand_read(&opr_src);
    cpu.esp += opr_src.val;
        
    return 0;
}
