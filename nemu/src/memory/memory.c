#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>
#include "memory/mmu/cache.h"

uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
    uint32_t map_NO = is_mmio(paddr);
    if(map_NO == -1){
#ifdef CACHE_ENABLED
		ret = cache_read(paddr, len);     // 通过cache进行读
#else
		ret = hw_mem_read(paddr, len);
#endif
    }else{
        ret = mmio_read(paddr,len,map_NO);
    }
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data)
{
    uint32_t map_NO = is_mmio(paddr);
    if(map_NO == -1){
#ifdef CACHE_ENABLED
		cache_write(paddr, len, data);    // 通过cache进行写
#else
		hw_mem_write(paddr, len, data);
#endif
    }else{
        mmio_write(paddr,len,data,map_NO);
    }
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
    assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_PAGE
    if(cpu.cr0.pg && cpu.cr0.pe){
        if((laddr&0x3ff) > (0x400-len)){
            paddr_t paddr = page_translate(laddr);
            // 低位处理
            uint32_t low = paddr_read(paddr,0x400 - (laddr&0x3ff));
            // 高位处理
            paddr = page_translate(laddr+(0x400 - (laddr&0x3ff)));
            uint32_t high = paddr_read(paddr,len-(0x400 - (laddr&0x3ff)));
            // 拼接
            return (high<<((0x400-(laddr&0x3ff))*8))+low;
        }else{
            paddr_t paddr = page_translate(laddr);
            return paddr_read(paddr,len);
        }
    }else{
        return paddr_read(laddr, len);
    }
#else
    return paddr_read(laddr, len);
#endif
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
    assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_PAGE
    if(cpu.cr0.pg && cpu.cr0.pe){
        if((laddr&0x3ff) > (0x400-len)){
            paddr_t paddr = page_translate(laddr);
            // 低位处理
            paddr_write(paddr,0x400 - (laddr&0x3ff),data);
            // 高位处理
            paddr = page_translate(laddr+(0x400-(laddr&0x3ff)));
            paddr_write(paddr,len-(0x400-(laddr&0x3ff)),data>>(8*(0x400-(laddr&0x3ff))));
        }else{
            paddr_t paddr = page_translate(laddr);
            return paddr_write(paddr,len,data);
        }
    }else{
        return paddr_write(laddr, len, data);
    }
#else
    return paddr_write(laddr, len, data);
#endif
}

uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
	return laddr_read(vaddr, len);
#else
    uint32_t laddr=vaddr;
    if(cpu.cr0.pe)
    {
        laddr=segment_translate(vaddr,sreg);
    }
    return laddr_read(laddr,len);
#endif
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifndef IA32_SEG
	laddr_write(vaddr, len, data);
#else
    uint32_t laddr=vaddr;
    if(cpu.cr0.pe)
    {
        laddr=segment_translate(vaddr,sreg);
    }
    laddr_write(laddr,len,data);
#endif
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);
#ifdef CACHE_ENABLED
	init_cache();                             // 初始化cache
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
