#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/reboot.h>
#include <linux/suspend.h>

#include <asm/tlbflush.h>
#include <asm/sbi.h>
#include <asm/csr.h>
#include <asm/andesv5/smu.h>
#include <asm/andesv5/proc.h>

// define in riscv/kernel/pm.c
extern int suspend_begin;

struct atc_smu atcsmu;
EXPORT_SYMBOL(atcsmu);

#ifdef CONFIG_ATCSMU
// main hart
extern int num_cpus;
void andes_suspend2ram(void)
{
	sbi_enter_suspend_mode(DeepSleepMode, true, *wake_mask, num_cpus);
}


// main hart
void andes_suspend2standby(void)
{
	sbi_enter_suspend_mode(LightSleepMode, true, *wake_mask, num_cpus);
}

// other harts
void atcsmu100_set_suspend_mode(void)
{
	if (suspend_begin == PM_SUSPEND_MEM) {
		sbi_set_suspend_mode(DeepSleepMode);
	} else if (suspend_begin == PM_SUSPEND_STANDBY) {
		sbi_set_suspend_mode(LightSleepMode);
	} else {
		sbi_set_suspend_mode(CpuHotplugDeepSleepMode);
	}
}
#endif

static int atcsmu100_restart_call(struct notifier_block *nb,
                                 unsigned long action, void *data)
{
	unsigned int cpu_num = num_possible_cpus();

	sbi_restart(cpu_num);
	return 0;
}

static struct notifier_block atcsmu100_restart = {
       .notifier_call = atcsmu100_restart_call,
       .priority = 128,
};

static int atcsmu_probe(struct platform_device *pdev)
{
	struct atc_smu *smu = &atcsmu;
	int ret = -ENOENT;
	int pcs = 0;

	spin_lock_init(&smu->lock);

	smu->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!smu->res)
		goto err_exit;

	ret = -EBUSY;

	smu->res = request_mem_region(smu->res->start,
					smu->res->end - smu->res->start+  1,
					pdev->name);
	if (!smu->res)
		goto err_exit;

	ret = -EINVAL;

	smu->base = ioremap(smu->res->start,
					smu->res->end - smu->res->start + 1);
	if (!smu->base)
		goto err_ioremap;

	for (pcs = 0; pcs < MAX_PCS_SLOT; pcs++)
		writel(0xffdfffff, (void *)(smu->base + PCSN_WE_OFF(pcs)));

	register_restart_handler(&atcsmu100_restart);

	return 0;
err_ioremap:
	release_resource(smu->res);
err_exit:
	return ret;
}

static int __exit atcsmu_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id atcsmu_of_id_table[] = {
	{ .compatible = "andestech,atcsmu" },
	{}
};
MODULE_DEVICE_TABLE(of, atcsmu_of_id_table);
#endif

static struct platform_driver atcsmu_driver = {
	.probe = atcsmu_probe,
	.remove = __exit_p(atcsmu_remove),
	.driver = {
		.name = "atcsmu",
		.of_match_table = of_match_ptr(atcsmu_of_id_table),
	},
};

static int __init atcsmu_init(void)
{
	int ret = platform_driver_register(&atcsmu_driver);

	return ret;
}
subsys_initcall(atcsmu_init);
