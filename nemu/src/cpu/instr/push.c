#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op() 
{
	operand_read(&opr_src);
	if(opr_src.data_size == 8)
	    opr_src.val = sign_ext(opr_src.val & (0xffffffff >> (32 - data_size)), data_size);
    cpu.esp -= data_size/8;
	vaddr_write(cpu.esp, SREG_DS, data_size/8, opr_src.val);
}

make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, r, v)

make_instr_func(pusha){
    uint32_t temp = cpu.esp;
    for(int i = 0; i < 8; ++i){
        cpu.esp -= 4;
	    if(i == 4) vaddr_write(cpu.esp, SREG_DS, 4, temp);
	    else vaddr_write(cpu.esp, SREG_DS, 4, cpu.gpr[i].val);
    }
    
    return 1;
}
