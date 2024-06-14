#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
	return cpu.segReg[sreg].base + offset;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */
	 SegDesc segDesc;
	 laddr_t laddr = cpu.gdtr.base + 8*cpu.segReg[sreg].index;
	 segDesc.val[0] = laddr_read(laddr,4);
	 segDesc.val[1] = laddr_read(laddr + 4,4);

	 assert(segDesc.privilege_level == 0);
	 assert(segDesc.granularity == 1);
	 assert(segDesc.present == 1);

	 
	 union{
	     uint32_t val;
	     struct{
            uint32_t base_15_0 : 16;
	        uint32_t base_23_16 : 8;
		    uint32_t base_31_24 : 8;
	     };
	 } base;
	 
	 base.base_15_0 = segDesc.base_15_0;
	 base.base_23_16 = segDesc.base_23_16;
	 base.base_31_24 = segDesc.base_31_24;
	 
	 assert(base.val == 0x00000000);
	 
	 union{
	     uint32_t val;
	     struct{
            uint32_t limit_15_0 : 16;
            uint32_t limit_19_16 : 4;
            uint32_t limit_31_20 : 12;
	     };
	 } limit;
	 
	 limit.limit_15_0 = segDesc.limit_15_0;
	 limit.limit_19_16 = segDesc.limit_19_16;
	 limit.limit_31_20 = 0xfff;
	 
	 assert(limit.val == 0xffffffff);
	 
	 union{
	     struct{
	         uint8_t val : 5;
	     };
	     struct{
	         uint8_t type_3_0 : 4;
	         uint8_t type_4 : 1;
	     };
	 } type;
	 type.type_3_0 = segDesc.type;
	 type.type_4 = segDesc.segment_type;
	 
	 cpu.segReg[sreg].base = base.val;
	 cpu.segReg[sreg].limit = limit.val;
	 cpu.segReg[sreg].type = type.val;
	 cpu.segReg[sreg].privilege_level = segDesc.privilege_level;
	 cpu.segReg[sreg].soft_use = segDesc.soft_use;
}
