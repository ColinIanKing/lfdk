/*
 * LFDD - Linux Firmware Debug Driver
 * File: libmem.c
 *
 * Copyright (C) 2006 - 2009 Merck Hung <merckhung@gmail.com>
 *										<merck_hung@asus.com.tw>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>

#include <linux/delay.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/highmem.h>

#include "lfdd.h"


static const unsigned long maxaddr = ~0UL;

static void *__do_phys_to_virt(unsigned long phys) {
	void *addr = NULL;
	unsigned long start = phys & PAGE_MASK;
	unsigned long pfn = PFN_DOWN(phys);

	if (page_is_ram(start >> PAGE_SHIFT)) {
		struct page *pg = pfn_to_page(pfn);
		if (PageHighMem(pg)) {
			return kmap(pg);
		}
		return __va(phys);
	}
	addr = (void __force *)ioremap_nocache(start, PAGE_SIZE);
	if (addr)
		return (void *)((unsigned long)addr | (phys & ~PAGE_MASK));

	return NULL;
}

static void __undo_phys_to_virt(unsigned long phys, void *addr)
{
	unsigned long start = phys & PAGE_MASK;
	unsigned long pfn = PFN_DOWN(phys);

	if (page_is_ram(start >> PAGE_SHIFT)) {
		struct page *pg = pfn_to_page(pfn);
		if (PageHighMem(pg));
			kunmap(pfn_to_page(pfn));
		return;
	}
	iounmap((void __iomem *)((unsigned long)addr & PAGE_MASK));
}


unsigned char lfdd_mem_read_byte( unsigned long addr ) {

    unsigned char *vaddr;
    unsigned char value;

    // Check the range of physical address
    if( ((maxaddr - addr) <= LFDD_MASSBUF_SIZE)
        || ((addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        return 0xff;
    }

    // Map physical memory address
    vaddr = __do_phys_to_virt(addr);
    value = (vaddr) ? *vaddr : 0xff;
    __undo_phys_to_virt(addr, vaddr);

    return value;
}


unsigned short int lfdd_mem_read_word( unsigned long addr ) {

    unsigned short int *vaddr;
    unsigned short int value;

    // Check the range of physical address
    if( ((maxaddr - addr) <= LFDD_MASSBUF_SIZE)
        || ((addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        return 0xffff;
    }

    // Map physical memory address
    vaddr = __do_phys_to_virt(addr);
    value = (vaddr) ? *vaddr : 0xffff;
    __undo_phys_to_virt(addr, vaddr);

    return value;
}


unsigned int lfdd_mem_read_dword( unsigned long addr ) {

    unsigned int *vaddr;
    unsigned int value;

    // Check the range of physical address
    if( ((maxaddr - addr) <= LFDD_MASSBUF_SIZE)
        || ((addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        return 0xffffffff;
    }

    // Map physical memory address
    vaddr = __do_phys_to_virt(addr);
    value = (vaddr) ? *vaddr : 0xffffffff;
    __undo_phys_to_virt(addr, vaddr);

    return value;
}


void lfdd_mem_write_byte( unsigned int value, unsigned long addr ) {

    unsigned char *vaddr;

    // Check the range of physical address
    if( ((maxaddr - addr) <= LFDD_MASSBUF_SIZE)
        || ((addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        return;
    }

    // Map physical memory address
    vaddr = __do_phys_to_virt(addr);
    if (vaddr) {
        *vaddr = (unsigned char)value;
        __undo_phys_to_virt(addr, vaddr);
    }
}


void lfdd_mem_write_word( unsigned int value, unsigned long addr ) {

    unsigned short int *vaddr;

    // Check the range of physical address
    if( ((maxaddr - addr) <= LFDD_MASSBUF_SIZE)
        || ((addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        return;
    }

    // Map physical memory address
    vaddr = __do_phys_to_virt(addr);
    if (vaddr) {
        *vaddr = value;
        __undo_phys_to_virt(addr, vaddr);
    }
}


void lfdd_mem_write_dword( unsigned int value, unsigned long addr ) {

    unsigned int *vaddr;

    // Check the range of physical address
    if( ((maxaddr - addr) <= LFDD_MASSBUF_SIZE)
        || ((addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        return;
    }

    // Map physical memory address
    vaddr = __do_phys_to_virt(addr);
    if (vaddr) {
        *vaddr = value;
        __undo_phys_to_virt(addr, vaddr);
    }
}


void lfdd_mem_read_256byte( struct lfdd_mem_t *pmem ) {

    unsigned char *vaddr;
    int i;

    // Check the range of physical address
    if( ((maxaddr - pmem->addr) <= LFDD_MASSBUF_SIZE)
        || ((pmem->addr + LFDD_MASSBUF_SIZE) >= virt_to_phys( high_memory )) ) {

        memset( pmem->mass_buf, 0xff, LFDD_MASSBUF_SIZE );
    } else {

        // Map physical memory address
        vaddr = __do_phys_to_virt(pmem->addr);
        if (vaddr) {
            // Read LFDD_MASSBUF_SIZE bytes
            for( i = 0 ; i < LFDD_MASSBUF_SIZE ; i++ ) {
                pmem->mass_buf[ i ] = *(vaddr + i);
            }
            __undo_phys_to_virt(pmem->addr, vaddr);
        }
    }
}
