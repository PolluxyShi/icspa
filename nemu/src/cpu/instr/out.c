#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/

make_instr_func(out_b){
    pio_write(cpu.edx,1,cpu.eax);

    return 1;
}

make_instr_func(out_v){
    pio_write(cpu.edx,data_size/8,cpu.eax);

    return 1;
}
