#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave){
    cpu.esp = cpu.ebp;
    cpu.ebp = vaddr_read(cpu.esp, SREG_DS, data_size/8);
    cpu.esp += data_size/8;
    return 1;
}