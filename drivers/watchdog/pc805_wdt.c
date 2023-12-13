// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog driver for the PC805 EVB CPU Boards
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/watchdog.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/of.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/gpio.h>

struct pc805_wdt_drv {
	struct watchdog_device wdt;
	struct gpio_desc *dw_en;
	struct gpio_desc *dw_wdi;
};

struct pc805_wdt_drv *wdt;

static int wdt_restart_handle(struct notifier_block *this, unsigned long mode,
			      void *cmd)
{
	/*
         * Cobalt devices have no way of rebooting themselves other
         * than getting the watchdog to pull reset, so we restart the
         * watchdog on reboot with no heartbeat.
         */
	printk("wdt restart\n");
	gpiod_set_value(wdt->dw_en, 1);
	/* loop until the watchdog fires */
	while (true);

	return NOTIFY_DONE;
}

static struct notifier_block wdt_restart_handler = {
	.notifier_call = wdt_restart_handle,
	.priority = 220,
};

static const struct watchdog_info pc805_wdt_info = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
	.identity = "pc805 Watchdog",
};

static int pc805_wdt_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	u32 property;
	int ret;

	wdt = devm_kzalloc(dev, sizeof(struct pc805_wdt_drv), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	ret = of_property_read_u32(pdev->dev.of_node, "priority", &property);
	if (!ret) {
		if (property > 255)
			dev_err(&pdev->dev, "Invalid priority property: %u\n",
				property);
		else
			wdt_restart_handler.priority = property;
	}

	/* Try requesting the GPIOs */
	wdt->dw_en = devm_gpiod_get(&pdev->dev, "en", GPIOD_OUT_LOW);
	if (IS_ERR(wdt->dw_en)) {
		ret = PTR_ERR(wdt->dw_en);
		dev_err(&pdev->dev, "clock line GPIO request failed\n");
		return -1;
	}

	wdt->dw_wdi = devm_gpiod_get(&pdev->dev, "wdi", GPIOD_OUT_LOW);
	if (IS_ERR(wdt->dw_wdi)) {
		ret = PTR_ERR(wdt->dw_wdi);
		dev_err(&pdev->dev, "data line GPIO request failed\n");
		return -1;
	}

	ret = register_restart_handler(&wdt_restart_handler);
	if (ret) {
		dev_err(&pdev->dev, "%s: cannot register restart handler, %d\n",
			__func__, ret);
		return -ENODEV;
	}

	dev_info(dev, "pc805 watchdog  driver enabled\n");

	return 0;
}

static const struct of_device_id pc805_wdt_ids[] = {
	{ .compatible = "picocom,pc805-wdt" },
	{},
};
MODULE_DEVICE_TABLE(of, pc805_wdt_ids);

static struct platform_driver pc805_wdt_driver = {
	.probe = pc805_wdt_probe,
	.driver = {
		.name = "pc805-watchdog",
		.of_match_table = pc805_wdt_ids,
	},
};

module_platform_driver(pc805_wdt_driver);

MODULE_DESCRIPTION("pc805 wdt driver");
MODULE_ALIAS("platform:pc805-wdt");
MODULE_AUTHOR("picocom");
MODULE_LICENSE("GPL");
