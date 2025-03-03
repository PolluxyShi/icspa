#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
    // Trigger an exception/interrupt with 'intr_no'
    // 'intr_no' is the index to the IDT
    // Push EFLAGS, CS, and EIP
    cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_DS, 4, cpu.eflags.val);
    cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_DS, 4, cpu.cs.val);
	cpu.esp -= 4;
	vaddr_write(cpu.esp, SREG_DS, 4, cpu.eip);
	// Find the IDT entry using 'intr_no'
	GateDesc gateDesc;
	laddr_t laddr = cpu.idtr.base + 8*intr_no;
	gateDesc.val[0] = laddr_read(laddr,4);
	gateDesc.val[1] = laddr_read(laddr + 4,4);
    // Clear IF if it is an interrupt
    if(gateDesc.type == 0xE) cpu.eflags.IF = 0;
    // Set EIP to the entry of the interrupt handler
    cpu.eip = gateDesc.offset_15_0 + (gateDesc.offset_31_16 << 16);
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
