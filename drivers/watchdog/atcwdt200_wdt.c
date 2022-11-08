/*
 *  Watchdog driver for the ATCWDT200 Watch Dog Driver
 *
 *      (c) Copyright 2016 Andes Technology Corp. (www.andestech.com)
 *      Based on sa1100_wdt.c by Oleg Drokin <green@crimea.edu>
 *      Based on SoftDog driver by Alan Cox <alan@redhat.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *      11/07/2016 Initial release
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/reboot.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/reboot.h>
#include <linux/uaccess.h>
#include <linux/cpumask.h>
#include <asm/processor.h>
#include <asm/sbi.h>
#include <asm/andesv5/smu.h>

#define DRIVER_NAME	"atcwdt200"
#define DEBUG( str, ...)			\
	do{					\
		if( debug)			\
		printk( str, ##__VA_ARGS__);	\
	} while(0)
/* ID and Revision Register */
#define idrev		(*( volatile unsigned int *)( wdt_base + 0x00))
#define ID_OFF		12
#define ID_MSK		(0xfffff<<ID_OFF)
#define ID		(0x03002<<ID_OFF)

/* Control Register */
#define ctrl		(*( volatile unsigned int *)( wdt_base + 0x10))
#define RST_TIME_OFF	8
#define RST_TIME_MSK	(0x3<<RST_TIME_OFF)
#define RST_CLK_128	(0<<RST_TIME_OFF)
#define RST_CLK_256	(1<<RST_TIME_OFF)
#define RST_CLK_512	(2<<RST_TIME_OFF)
#define RST_CLK_1024	(3<<RST_TIME_OFF)
#define INT_TIME_OFF	4
#define INT_TIME_MSK	(0xf<<INT_TIME_OFF)
#define INT_CLK_64	(0<<INT_TIME_OFF)
#define INT_CLK_256	(1<<INT_TIME_OFF)
#define INT_CLK_1024	(2<<INT_TIME_OFF)
#define INT_CLK_2048	(3<<INT_TIME_OFF)
#define INT_CLK_4096	(4<<INT_TIME_OFF)
#define INT_CLK_8192	(5<<INT_TIME_OFF)
#define INT_CLK_16384	(6<<INT_TIME_OFF)
#define INT_CLK_32768	(7<<INT_TIME_OFF)
#define RST_EN		(1<<3)
#define INT_EN		(1<<2)
#define CLK_PCLK	(1<<1)
#define WDT_EN		(1<<0)

/* Restart Register */
#define restart		(*( volatile unsigned int *)( wdt_base + 0x14))
#define RESTART_MAGIC	0xcafe

/* Write Enable Register */
#define wren		(*( volatile unsigned int *)( wdt_base + 0x18))
#define WP_MAGIC	0x5aa5

/* Status Register */
#define st		(*( volatile unsigned int *)( wdt_base + 0x1c))
#define INT_EXPIRED	(1<<0)

#define TIMER_MARGIN	60	/* (secs) Default is 1 minute */

static int expect_close = 0;
static int debug = 0;
static int timeout = TIMER_MARGIN;  /* in seconds */
static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started "
	"(default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");
static int wdt_panic = 0;
module_param(wdt_panic, int, 0);
MODULE_PARM_DESC(wdt_panic,
	"Watchdog action, set to 1 to panic, 0 to reboot (default=0)");

module_param(debug, int, 0);
module_param(timeout, int, 0);
static u32 wdt_freq;
static void __iomem *wdt_base;

// enable the WDT
void wdt_start(void)
{
#ifdef CONFIG_WATCHDOG_DEBUG
	wren = WP_MAGIC;
	ctrl = (INT_CLK_32768|INT_EN|WDT_EN);
#else
	wren = WP_MAGIC;
	ctrl = (INT_CLK_32768|INT_EN|RST_CLK_128|RST_EN|WDT_EN);
#endif
}

int wdt_get_timeout(void)
{
	int time = (ctrl & INT_TIME_MSK) >> INT_TIME_OFF;

	if (time > 7)
		return (time - 7) * 2;
	else
		return 1;
}

// set reset vector and enable WDT
static int wdtdog_open(struct inode *inode, struct file *file){
	int val = num_online_cpus();

	sbi_set_reset_vec(val);
	wdt_start();
	DEBUG("Activating WDT..\n");
	return 0;
}

// disable WDT, NOWAY_OUT MEANS NEED MAGIC NUMBER
// OR just disable it if NOWAY_OUT not set
static int wdtdog_release(struct inode *inode, struct file *file){
#ifndef CONFIG_WATCHDOG_NOWAYOUT
	/*
	 *  Shut off the timer.
	 *  Lock it in if it's a module and we defined ...NOWAYOUT
	 */
	if(expect_close) {
		wren = WP_MAGIC;
		ctrl = 0;
		DEBUG( "Deactivating WDT..\n");
	}
#endif
	expect_close = 0;
	return 0;
}
// clean the WDT
static ssize_t wdtdog_write(struct file *file, const char *data, size_t len, loff_t *ppos){
	if (!len)
		return 0;

	if (!nowayout){
		size_t i;
		for (i = 0; i < len; i++){
			char c;
			if (get_user(c, data + i))
				return -EFAULT;
			if (c == 'V'){
				expect_close = 42;
				break;
			}
		}
	}
	wren = WP_MAGIC;
	restart = RESTART_MAGIC;
	st |= INT_EXPIRED;
	return len;
}

static long wdtdog_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static struct watchdog_info ident = {
		.identity = "FTWDT010 watchdog",
	};

	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_options, retval = -EINVAL;

	switch( cmd){
		case WDIOC_GETSUPPORT:
			return copy_to_user(argp, &ident, sizeof( ident));

		case WDIOC_GETSTATUS:
		case WDIOC_GETBOOTSTATUS:
			return put_user(0, p);

		case WDIOC_SETOPTIONS:
			if (get_user(new_options, p))
				return -EFAULT;

			if (new_options & WDIOS_DISABLECARD){
				wren = WP_MAGIC;
				ctrl &= ~WDT_EN;
				retval = 0;
			}

			if (new_options & WDIOS_ENABLECARD){
				wdt_start();
				retval = 0;
			}
			return retval;

		case WDIOC_GETTIMEOUT:
			timeout = wdt_get_timeout();
			return put_user(timeout, p);

		case WDIOC_KEEPALIVE:
			wren = WP_MAGIC;
			restart = RESTART_MAGIC;
			st |= INT_EXPIRED;
			return 0;

		default:
			return -ENOIOCTLCMD;
	}
}

#ifdef CONFIG_ATCSMU
extern struct atc_smu atcsmu;
static int atcwdt200_restart_call(struct notifier_block *nb,
								unsigned long action, void *data)
{
	int i;
	int cpu_num = num_online_cpus();

	if (atcsmu.base <= 0)
       pr_err("Need to add smu node to dts!!\n");

	for (i = 0; i < cpu_num; i++)
		writel(FLASH_BASE ,atcsmu.base + RESET_VEC_OFF +
               i * RESET_VEC_PER_CORE);

	wren = WP_MAGIC;
	ctrl = (INT_CLK_32768|INT_EN|RST_CLK_128|RST_EN|WDT_EN);
	return 0;
}

static struct notifier_block atcwdt200_restart = {
    .notifier_call = atcwdt200_restart_call,
    .priority = 255,
};
#endif


static struct file_operations wdtdog_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= wdtdog_write,
	.unlocked_ioctl	= wdtdog_ioctl,
	.open		= wdtdog_open,
	.release	= wdtdog_release,
};

static struct miscdevice atcwdt200_dog_miscdev = {
	WATCHDOG_MINOR,
	"ATCWDT200 watchdog",
	&wdtdog_fops
};

#ifdef CONFIG_OF
static const struct of_device_id atcwdt200_dog_match[] = {
	{ .compatible = "andestech,atcwdt200" },
	{},
};
MODULE_DEVICE_TABLE(of, atcwdt200_dog_match);
#endif

static int atcwdt200_dog_probe(struct platform_device *pdev){
	int (*read_fixup)(void __iomem *addr, unsigned int val,
		unsigned int shift_bits);
	int ret;
	struct resource *res;
	struct device_node *node;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	wdt_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(wdt_base))
		return PTR_ERR(wdt_base);

	/* check ID and Revision register */
	read_fixup = symbol_get(readl_fixup);
	ret = read_fixup(wdt_base, 0x03002, 12);
	symbol_put(readl_fixup);
	if (!ret){
		dev_err(&pdev->dev, "fail to read ID and Revision reg, bitmap not support atcwdt200.\n");
		return -ENOENT;
	}

	if((idrev & ID_MSK)!=ID)
		return -ENOENT;

	node = of_find_matching_node(NULL, atcwdt200_dog_match);
	if (of_property_read_u32(node, "clock-frequency", &wdt_freq))
		panic("Can't read clock-frequency");

	ret = misc_register(&atcwdt200_dog_miscdev);
	if( ret)
		return ret;

#ifdef CONFIG_ATCSMU 
	register_restart_handler(&atcwdt200_restart);
#endif

	dev_info(&pdev->dev, "ATCWDT200 watchdog timer driver.\n");

	DEBUG("ATCWDT200 watchdog timer: timer timeout %d sec\n", timeout);
	return 0;
}

static int atcwdt200_dog_remove(struct platform_device *pdev){
	misc_deregister(&atcwdt200_dog_miscdev);
	return 0;
}

static struct platform_driver atcwdt200_dog_driver = {
	.probe		= atcwdt200_dog_probe,
	.remove		= atcwdt200_dog_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(atcwdt200_dog_match),
	},
};

module_platform_driver(atcwdt200_dog_driver);
MODULE_AUTHOR("Andes Corp.");
MODULE_LICENSE("GPL");
