#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/timex.h>
#include <linux/of_platform.h>

#include <asm/sbi.h>

#define LV_MASK	0xF0
#define LV_OFF	4

#define NUM_LV	16
#define LOW_LV	15
#define HIGH_LV	0
#define PERIOD	(policy->cpuinfo.max_freq / NUM_LV)

void read_powerbrake(void *arg)
{
	int *ret = arg;

	*ret = sbi_read_powerbrake();
}

void write_powerbrake(void *arg)
{
	int *val = arg;

	sbi_write_powerbrake(*val);
}

static unsigned int riscv_cpufreq_get(unsigned int cpu)
{
	int val, max;
	struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);

	smp_call_function_single(cpu, read_powerbrake, &val, 1);
	val = (val & LV_MASK) >> LV_OFF;

	max = (LOW_LV - val + 1) * PERIOD;
	return max;
}

static int riscv_cpufreq_set_policy(struct cpufreq_policy *policy)
{
	int val, i;
	unsigned int cpu = policy->cpu;

	if (!policy)
		return -EINVAL;

	if (policy->min == 0 && policy->max == 0) {
		pr_err ("Cannot set zero freq\n");
		return -EINVAL;
	}
	val = (policy->min + policy->max) / 2;
	switch (policy->policy) {
		case CPUFREQ_POLICY_POWERSAVE:
			val = (val + policy->min) / 2;
			break;
		case CPUFREQ_POLICY_PERFORMANCE:
			val = (val + policy->max) / 2;
			break;
		default:
			pr_err ("Not Support this governor\n");
			break;
	}

	if (val < 0) {
		pr_err ("The freq is valid\n");
		return -EINVAL;
	}

	/*
	 * Powerbrake register has 16 level,
	 * so we divide the xxxkHZ into 16 parts.
	 *
	 * EX: 100MHZ->100*1000kHZ
	 *	|    |    |........|
	 * Mhz	0  6250 12500    16*6250
	 */

	// transfer MHZ to kHZ, and divided to 16 level
	for (i = 1; i <= NUM_LV; i++) {
		if (val <= i * PERIOD) {
			val = LOW_LV - (i - 1);
			break;
		}
	}
	val = val << LV_OFF;

	return smp_call_function_single(cpu, write_powerbrake, &val, 1);
}

static int riscv_cpufreq_verify_policy(struct cpufreq_policy_data *policy)
{
	if (!policy)
		return -EINVAL;

	cpufreq_verify_within_cpu_limits(policy);

	return 0;
}

static void riscv_get_policy(struct cpufreq_policy *policy)
{
	int val;

	smp_call_function_single(policy->cpu, read_powerbrake, &val, 1);
	val = (val & LV_MASK) >> LV_OFF;

	/*
	 * Powerbrake register has 16 level,
	 * so we divide the xxxkHZ into 16 parts.
	 *
	 * EX: 100MHZ->100*1000kHZ
	 *	|    |    |........|
	 * Mhz	0  6250 12500    16*6250
	 */
	policy->min = (LOW_LV - val) * PERIOD;
	policy->max = (LOW_LV - val + 1) * PERIOD;
	policy->policy = CPUFREQ_POLICY_POWERSAVE;
}

static int riscv_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	u32 max_freq;
	struct device_node *cpu;

	cpu = of_get_cpu_node(policy->cpu, NULL);

        if (!cpu)
                return -ENODEV;

	pr_debug("init cpufreq on CPU %d\n", policy->cpu);
	if (of_property_read_u32(cpu, "clock-frequency", &max_freq)) {
		pr_err("%s missing clock-frequency\n", cpu->name);
		return -EINVAL;
	}
        of_node_put(cpu);

	policy->cpuinfo.min_freq = 0;
	policy->cpuinfo.max_freq = max_freq / 1000; /* kHZ */
	riscv_get_policy(policy);

	return 0;
}

static struct cpufreq_driver riscv_cpufreq_driver = {
	.flags		= CPUFREQ_CONST_LOOPS,
	.verify		= riscv_cpufreq_verify_policy,
	.setpolicy	= riscv_cpufreq_set_policy,
	.get		= riscv_cpufreq_get,
	.init		= riscv_cpufreq_cpu_init,
	.name		= "riscv_cpufreq",
};

static int __init riscv_cpufreq_init(void)
{
	return cpufreq_register_driver(&riscv_cpufreq_driver);
}

static void __exit riscv_cpufreq_exit(void)
{
	cpufreq_unregister_driver(&riscv_cpufreq_driver);
}

MODULE_AUTHOR("Nick Hu <nickhu@andestech.com>");
MODULE_DESCRIPTION("Riscv cpufreq driver.");
MODULE_LICENSE("GPL");
module_init(riscv_cpufreq_init);
module_exit(riscv_cpufreq_exit);
