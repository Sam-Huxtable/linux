/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2005-2020 Andes Technology Corporation
 */

#include <linux/export.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <asm/fixmap.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>

void *kmap(struct page *page)
{
	unsigned long vaddr;
	might_sleep();
	if (!PageHighMem(page))
		return page_address(page);
	vaddr = (unsigned long)kmap_high(page);
	flush_cache_kmaps();
	return (void *)vaddr;
}

EXPORT_SYMBOL(kmap);

void kunmap(struct page *page)
{
	BUG_ON(in_interrupt());
	if (!PageHighMem(page))
		return;
	kunmap_high(page);
}

EXPORT_SYMBOL(kunmap);

void *kmap_atomic(struct page *page)
{
	unsigned int idx;
	unsigned long vaddr;
	int type;
	pte_t *ptep;

	preempt_disable();
	pagefault_disable();

	if (!PageHighMem(page))
		return page_address(page);

	type = kmap_atomic_idx_push();

	idx = type + KM_TYPE_NR * smp_processor_id();
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);

	ptep = pte_offset_kernel(pmd_off_k(vaddr), vaddr);
	set_pte(ptep, mk_pte(page, kmap_prot));
	flush_cache_kmaps();

	return (void *)vaddr;
}

EXPORT_SYMBOL(kmap_atomic);

void __kunmap_atomic(void *kvaddr)
{
	if (kvaddr >= (void *)FIXADDR_START && kvaddr < (void *)FIXADDR_TOP) {
		unsigned long vaddr = (unsigned long)kvaddr;
		pte_t *ptep;
		kmap_atomic_idx_pop();
		ptep = pte_offset_kernel(pmd_off_k(vaddr), vaddr);
		set_pte(ptep, __pte(0));
		flush_cache_kmaps();
	}
	pagefault_enable();
	preempt_enable();
}

EXPORT_SYMBOL(__kunmap_atomic);
