#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/

make_instr_func(lea)
{
        opr_dest.data_size = opr_src.data_size = data_size;
        int len = 1;
        len += modrm_r_rm(eip+1,&opr_dest,&opr_src);
        
        opr_dest.val = opr_src.addr;
        operand_write(&opr_dest);

        return len;
}
