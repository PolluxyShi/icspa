#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>
#include <stdio.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

#ifdef HAS_DEVICE_IDE
	uint8_t buf[4096];
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	elf = (void *)0x0;
	Log("ELF loading from ram disk.");
#endif

	/* Load each program segment */
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{
			// remove this panic!!!
// 			panic("Please implement the loader");

#ifndef IA32_PAGE
/* TODO: copy the segment from the ELF file to its proper memory area */
            #ifdef HAS_DEVICE_IDE
            ide_read((void *)ph->p_vaddr,ELF_OFFSET_IN_DISK+(uint32_t)ph->p_offset,ph->p_filesz);
            #else
            memcpy((void *)ph->p_vaddr,(void *)ph->p_offset,ph->p_filesz);
            #endif
/* TODO: zeror the memory area [vaddr + file_sz, vaddr + mem_sz) */
            if(ph->p_memsz>ph->p_filesz) memset((void *)ph->p_vaddr+ph->p_filesz,0x00,ph->p_memsz-ph->p_filesz);
#else
            uint32_t paddr = mm_malloc(ph->p_vaddr,ph->p_memsz);
            #ifdef HAS_DEVICE_IDE
            ide_read((void *)paddr,ELF_OFFSET_IN_DISK+(uint32_t)ph->p_offset,ph->p_filesz);
            #else
            memcpy((void*)paddr,(void*)ph->p_offset,ph->p_filesz);
            #endif
            if(ph->p_memsz>ph->p_filesz) memset((void*)paddr+ph->p_filesz,0x00,ph->p_memsz-ph->p_filesz);
            // Log("laddr = 0x%x,\tpaddr = 0x%x\n",ph->p_vaddr,paddr);
#endif
            // printf("p_offset = %x, p_filesz = %x, p_vaddr = %x, p_memsz = %x\n", ph->p_offset,ph->p_filesz,ph->p_vaddr,ph->p_memsz);

#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
#endif
		}
	}

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
	return entry;
}
