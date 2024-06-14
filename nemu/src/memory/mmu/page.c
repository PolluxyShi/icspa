#include "cpu/cpu.h"
#include "memory/memory.h"

// translate from linear address to physical address
#ifdef IA32_PAGE
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	uint32_t offset = laddr & 0xfff;    // 12bit
	uint32_t page = (laddr>>12)&0x3ff;  // 10bit
	uint32_t dir = (laddr>>22)&0x3ff;   // 10bit
	
	PDE pde;
	pde.val = paddr_read((cpu.cr3.pdbr<<12)+(dir<<2),4);
 	assert(pde.present);
	
	PTE pte;
	pte.val = paddr_read((pde.page_frame<<12)+(page<<2),4);
 	assert(pte.present);

	return (pte.page_frame<<12)+offset;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
#endif
