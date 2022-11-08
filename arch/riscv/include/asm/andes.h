#ifndef ANDES_H
#define ANDES_H

extern bool andestar45;
extern long sbi_get_marchid(void);
static inline void andes_local_flush_tlb_all(void)
{
	if (!andestar45)
		__asm__ __volatile__ ("sfence.vma" : : : "memory");
}

#endif
