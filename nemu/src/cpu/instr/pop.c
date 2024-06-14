#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op() 
{
    opr_src.val = vaddr_read(cpu.esp, SREG_SS, 4);
    cpu.esp += opr_src.data_size/8;
    operand_write(&opr_src);
}

make_instr_impl_1op(pop, r, v)

make_instr_func(popa){
    for(int i = 7; i >= 0; --i){
	    if(i != 4) cpu.gpr[i].val = vaddr_read(cpu.esp, SREG_SS, 4);
        cpu.esp += 4;
    }
    
    return 1;
}
