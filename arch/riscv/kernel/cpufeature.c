// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copied from arch/arm64/kernel/cpufeature.c
 *
 * Copyright (C) 2015 ARM Ltd.
 * Copyright (C) 2017 SiFive
 */

#include <linux/of.h>
#include <asm/processor.h>
#include <asm/hwcap.h>
#include <asm/smp.h>
#include <asm/switch_to.h>

unsigned long elf_hwcap __read_mostly;
#ifdef CONFIG_FPU
bool has_fpu __read_mostly;
#endif
#ifdef CONFIG_DSP
bool has_dsp __read_mostly;
#endif


void riscv_fill_hwcap(void)
{
	struct device_node *node;
	const char *isa;

	for_each_of_cpu_node(node) {
		if (riscv_of_processor_hartid(node) < 0)
			continue;

		if (of_property_read_string(node, "riscv,isa", &isa)) {
			pr_warn("Unable to find \"riscv,isa\" devicetree entry\n");
			continue;
		}
	}

#ifdef CONFIG_FPU
	if (strstr(isa, "d") && strstr(isa, "f"))
		has_fpu = true;
#endif
#ifdef CONFIG_DSP
	if (strstr(isa, "xdsp"))
               has_dsp = true;
#endif
}
