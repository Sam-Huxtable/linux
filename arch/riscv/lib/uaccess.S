#include <linux/linkage.h>
#include <asm/asm.h>
#include <asm/csr.h>

	.altmacro
	.macro fixup op reg addr lbl
	LOCAL _epc
_epc:
	\op \reg, \addr
	.section __ex_table,"a"
	.balign RISCV_SZPTR
	RISCV_PTR _epc, \lbl
	.previous
	.endm

ENTRY(__asm_copy_to_user)
ENTRY(__asm_copy_from_user)

	/* Enable access to user memory */
	li t6, SR_SUM
	csrs CSR_SSTATUS, t6

	beqz a2, exit			/* if n is zero just exit */
	srli t0, a2, LGREG   	/* t0: number of word to copy */
	andi t1, a2, SZREG -1	/* t1: bytes less than a word to copy */
	beqz t0, byte_copy		/* Only less than a word to byte copy */

word_copy:
	fixup REG_L, t2, (a1), 10f
	fixup REG_S, t2, (a0), 10f
	addi a1, a1, SZREG
	addi a0, a0, SZREG
	addi t0, t0, -1			/* Decrease word count */
	bnez t0, word_copy		/* Continue looping to copy all words */
	beqz t1, exit			/* Done word copy and no left bytes to copy */

byte_copy:
	fixup lbu, t2, (a1), 10f
	fixup  sb, t2, (a0), 10f
	addi a1, a1, 1
	addi a0, a0, 1
	addi t1, t1, -1			/* Decrease byte count */
	bnez t1, byte_copy		/* Contine looping to clear all left bytes */

exit:
	/* Disable access to user memory */
	csrc CSR_SSTATUS, t6
	li a0, 0
	ret
ENDPROC(__asm_copy_to_user)
ENDPROC(__asm_copy_from_user)

ENTRY(__clear_user)

	/* Enable access to user memory */
	li t6, SR_SUM
	csrs CSR_SSTATUS, t6

	add a3, a0, a1
	addi t0, a0, SZREG-1
	andi t1, a3, ~(SZREG-1)
	andi t0, t0, ~(SZREG-1)
	/*
	 * a3: terminal address of target region
	 * t0: lowest doubleword-aligned address in target region
	 * t1: highest doubleword-aligned address in target region
	 */
	bgeu t0, t1, 2f
	bltu a0, t0, 4f
1:
	fixup REG_S, zero, (a0), 11f
	addi a0, a0, SZREG
	bltu a0, t1, 1b
2:
	bltu a0, a3, 5f

3:
	/* Disable access to user memory */
	csrc CSR_SSTATUS, t6
	li a0, 0
	ret
4: /* Edge case: unalignment */
	fixup sb, zero, (a0), 11f
	addi a0, a0, 1
	bltu a0, t0, 4b
	j 1b
5: /* Edge case: remainder */
	fixup sb, zero, (a0), 11f
	addi a0, a0, 1
	bltu a0, a3, 5b
	j 3b
ENDPROC(__clear_user)

	.section .fixup,"ax"
	.balign 4
	/* Fixup code for __copy_user(10) and __clear_user(11) */
10:
	/* Disable access to user memory */
	csrs CSR_SSTATUS, t6
	mv a0, a2
	ret
11:
	csrs CSR_SSTATUS, t6
	mv a0, a1
	ret
	.previous
