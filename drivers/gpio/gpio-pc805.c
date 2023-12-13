#include <linux/err.h>
#include <linux/gpio/driver.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>

/**
 * \brief  Structure type to access the GPIO_GEN registers.
 */
typedef struct {
	uint32_t ID;
	uint32_t VERSION;
} GPIO_GEN_Type;

/**
 * \brief  Structure type to access the GPIO_CTRL registers.
 */
typedef struct {
	uint32_t TIMER_R;
	uint32_t DIR_SET_HIGH;
	uint32_t DIR_SET_LOW;
	uint32_t DIR_CLEAR_HIGH;
	uint32_t DIR_CLEAR_LOW;
	uint32_t DIR_RD_HIGH;
	uint32_t DIR_RD_LOW;
	uint32_t DATA_IN_HIGH;
	uint32_t DATA_IN_LOW;
	uint32_t CAPT_EN_SET_HIGH;
	uint32_t CAPT_EN_SET_LOW;
	uint32_t CAPT_EN_CLEAR_HIGH;
	uint32_t CAPT_EN_CLEAR_LOW;
	uint32_t CAPT_EN_RD_HIGH;
	uint32_t CAPT_EN_RD_LOW;
	uint32_t CAPT_POL_SET_HIGH;
	uint32_t CAPT_POL_SET_LOW;
	uint32_t CAPT_POL_CLEAR_HIGH;
	uint32_t CAPT_POL_CLEAR_LOW;
	uint32_t CAPT_POL_RD_HIGH;
	uint32_t CAPT_POL_RD_LOW;
	uint32_t CAPT_IRQ_MASK_SET_HIGH;
	uint32_t CAPT_IRQ_MASK_SET_LOW;
	uint32_t CAPT_IRQ_MASK_CLEAR_HIGH;
	uint32_t CAPT_IRQ_MASK_CLEAR_LOW;
	uint32_t CAPT_IRQ_MASK_RD_HIGH;
	uint32_t CAPT_IRQ_MASK_RD_LOW;
	uint32_t CAPT_IRQ_STATUS_HIGH;
	uint32_t CAPT_IRQ_STATUS_LOW;
	uint32_t CAPT_IRQ_ACK_HIGH;
	uint32_t CAPT_IRQ_ACK_LOW;
	uint32_t GPIO_CAPT_DATA[48UL];
	uint32_t GPIO_CAPT_FIFO_LEVEL[48UL];
	uint32_t GPIO_CAPT_ERR[48UL];
	uint32_t FLUSH_CAPT_FIFO_HIGH;
	uint32_t FLUSH_CAPT_FIFO_LOW;
	uint32_t OUT_IRQ_MASK_SET_HIGH;
	uint32_t OUT_IRQ_MASK_SET_LOW;
	uint32_t OUT_IRQ_MASK_CLEAR_HIGH;
	uint32_t OUT_IRQ_MASK_CLEAR_LOW;
	uint32_t OUT_IRQ_MASK_RD_HIGH;
	uint32_t OUT_IRQ_MASK_RD_LOW;
	uint32_t OUT_IRQ_STATUS_HIGH;
	uint32_t OUT_IRQ_STATUS_LOW;
	uint32_t OUT_IRQ_ACK_HIGH;
	uint32_t OUT_IRQ_ACK_LOW;
	uint32_t DATA_OUT_DEF_SET_HIGH;
	uint32_t DATA_OUT_DEF_SET_LOW;
	uint32_t DATA_OUT_DEF_CLEAR_HIGH;
	uint32_t DATA_OUT_DEF_CLEAR_LOW;
	uint32_t DATA_OUT_DEF_RD_HIGH;
	uint32_t DATA_OUT_DEF_RD_LOW;
	uint32_t DATA_OUT_SAFE_SET_HIGH;
	uint32_t DATA_OUT_SAFE_SET_LOW;
	uint32_t DATA_OUT_SAFE_CLEAR_HIGH;
	uint32_t DATA_OUT_SAFE_CLEAR_LOW;
	uint32_t DATA_OUT_SAFE_RD_HIGH;
	uint32_t DATA_OUT_SAFE_RD_LOW;
	uint32_t GPIO_OUT[48UL];
	uint32_t GPIO_OUT_FIFO_LEVEL[48UL];
} GPIO_CTRL_Type;

struct gpio_reg {
	GPIO_GEN_Type gpio_gen;
	GPIO_CTRL_Type gpio_ctrl;
};

typedef struct {
	uint32_t GPIO_0_DBG_SEL;
	uint32_t GPIO_1_DBG_SEL;
	uint32_t GPIO_2_DBG_SEL;
	uint32_t GPIO_3_DBG_SEL;
	uint32_t GPIO_4_DBG_SEL;
	uint32_t GPIO_5_DBG_SEL;
	uint32_t GPIO_6_DBG_SEL;
	uint32_t GPIO_7_DBG_SEL;
	uint32_t GPIO_8_DBG_SEL;
	uint32_t GPIO_9_DBG_SEL;
	uint32_t GPIO_10_DBG_SEL;
	uint32_t GPIO_11_DBG_SEL;
	uint32_t GPIO_12_DBG_SEL;
	uint32_t GPIO_13_DBG_SEL;
	uint32_t GPIO_14_DBG_SEL;
	uint32_t GPIO_15_DBG_SEL;
	uint32_t GPIO_16_DBG_SEL;
	uint32_t GPIO_17_DBG_SEL;
	uint32_t GPIO_18_DBG_SEL;
	uint32_t GPIO_19_DBG_SEL;
	uint32_t GPIO_20_DBG_SEL;
	uint32_t GPIO_21_DBG_SEL;
	uint32_t GPIO_22_DBG_SEL;
	uint32_t GPIO_23_DBG_SEL;
	uint32_t GPIO_24_DBG_SEL;
	uint32_t GPIO_25_DBG_SEL;
	uint32_t GPIO_26_DBG_SEL;
	uint32_t GPIO_27_DBG_SEL;
	uint32_t GPIO_28_DBG_SEL;
	uint32_t GPIO_29_DBG_SEL;
	uint32_t GPIO_30_DBG_SEL;
	uint32_t GPIO_31_DBG_SEL;
} PAD_GPIO_DBG_SEL_Type;

/**
 * \brief  Structure type to access the PAD_GPIO_DBG_MODE registers.
 */
typedef struct {
	uint32_t GPIO_0_DEBUG_MODE;
	uint32_t GPIO_1_DEBUG_MODE;
	uint32_t GPIO_2_DEBUG_MODE;
	uint32_t GPIO_3_DEBUG_MODE;
	uint32_t GPIO_4_DEBUG_MODE;
	uint32_t GPIO_5_DEBUG_MODE;
	uint32_t GPIO_6_DEBUG_MODE;
	uint32_t GPIO_7_DEBUG_MODE;
	uint32_t GPIO_8_DEBUG_MODE;
	uint32_t GPIO_9_DEBUG_MODE;
	uint32_t GPIO_10_DEBUG_MODE;
	uint32_t GPIO_11_DEBUG_MODE;
	uint32_t GPIO_12_DEBUG_MODE;
	uint32_t GPIO_13_DEBUG_MODE;
	uint32_t GPIO_14_DEBUG_MODE;
	uint32_t GPIO_15_DEBUG_MODE;
	uint32_t GPIO_16_DEBUG_MODE;
	uint32_t GPIO_17_DEBUG_MODE;
	uint32_t GPIO_18_DEBUG_MODE;
	uint32_t GPIO_19_DEBUG_MODE;
	uint32_t GPIO_20_DEBUG_MODE;
	uint32_t GPIO_21_DEBUG_MODE;
	uint32_t GPIO_22_DEBUG_MODE;
	uint32_t GPIO_23_DEBUG_MODE;
	uint32_t GPIO_24_DEBUG_MODE;
	uint32_t GPIO_25_DEBUG_MODE;
	uint32_t GPIO_26_DEBUG_MODE;
	uint32_t GPIO_27_DEBUG_MODE;
	uint32_t GPIO_28_DEBUG_MODE;
	uint32_t GPIO_29_DEBUG_MODE;
	uint32_t GPIO_30_DEBUG_MODE;
	uint32_t GPIO_31_DEBUG_MODE;
} PAD_GPIO_DBG_MODE_Type;

/**
 * \brief  Structure type to access the PAD_FUNC_SEL registers.
 */
typedef struct {
	uint32_t PAD_0_FUNC_SEL_HI;
	uint32_t PAD_0_FUNC_SEL_LO;
	uint32_t PAD_1_FUNC_SEL_HI;
	uint32_t PAD_1_FUNC_SEL_LO;
	uint32_t PAD_2_FUNC_SEL_HI;
	uint32_t PAD_2_FUNC_SEL_LO;
	uint32_t PAD_3_FUNC_SEL_HI;
	uint32_t PAD_3_FUNC_SEL_LO;
	uint32_t PAD_4_FUNC_SEL_HI;
	uint32_t PAD_4_FUNC_SEL_LO;
	uint32_t PAD_5_FUNC_SEL_HI;
	uint32_t PAD_5_FUNC_SEL_LO;
	uint32_t PAD_6_FUNC_SEL_HI;
	uint32_t PAD_6_FUNC_SEL_LO;
	uint32_t PAD_7_FUNC_SEL_HI;
	uint32_t PAD_7_FUNC_SEL_LO;
	uint32_t PAD_8_FUNC_SEL_HI;
	uint32_t PAD_8_FUNC_SEL_LO;
	uint32_t PAD_9_FUNC_SEL_HI;
	uint32_t PAD_9_FUNC_SEL_LO;
	uint32_t PAD_10_FUNC_SEL_HI;
	uint32_t PAD_10_FUNC_SEL_LO;
	uint32_t PAD_11_FUNC_SEL_HI;
	uint32_t PAD_11_FUNC_SEL_LO;
	uint32_t PAD_12_FUNC_SEL_HI;
	uint32_t PAD_12_FUNC_SEL_LO;
	uint32_t PAD_13_FUNC_SEL_HI;
	uint32_t PAD_13_FUNC_SEL_LO;
	uint32_t PAD_14_FUNC_SEL_HI;
	uint32_t PAD_14_FUNC_SEL_LO;
	uint32_t PAD_15_FUNC_SEL_HI;
	uint32_t PAD_15_FUNC_SEL_LO;
	uint32_t PAD_16_FUNC_SEL_HI;
	uint32_t PAD_16_FUNC_SEL_LO;
	uint32_t PAD_17_FUNC_SEL_HI;
	uint32_t PAD_17_FUNC_SEL_LO;
	uint32_t PAD_18_FUNC_SEL_HI;
	uint32_t PAD_18_FUNC_SEL_LO;
	uint32_t PAD_19_FUNC_SEL_HI;
	uint32_t PAD_19_FUNC_SEL_LO;
	uint32_t PAD_20_FUNC_SEL_HI;
	uint32_t PAD_20_FUNC_SEL_LO;
	uint32_t PAD_21_FUNC_SEL_HI;
	uint32_t PAD_21_FUNC_SEL_LO;
	uint32_t PAD_22_FUNC_SEL_HI;
	uint32_t PAD_22_FUNC_SEL_LO;
	uint32_t PAD_23_FUNC_SEL_HI;
	uint32_t PAD_23_FUNC_SEL_LO;
	uint32_t PAD_24_FUNC_SEL_HI;
	uint32_t PAD_24_FUNC_SEL_LO;
	uint32_t PAD_25_FUNC_SEL_HI;
	uint32_t PAD_25_FUNC_SEL_LO;
	uint32_t PAD_26_FUNC_SEL_HI;
	uint32_t PAD_26_FUNC_SEL_LO;
	uint32_t PAD_27_FUNC_SEL_HI;
	uint32_t PAD_27_FUNC_SEL_LO;
	uint32_t PAD_28_FUNC_SEL_HI;
	uint32_t PAD_28_FUNC_SEL_LO;
	uint32_t PAD_29_FUNC_SEL_HI;
	uint32_t PAD_29_FUNC_SEL_LO;
	uint32_t PAD_30_FUNC_SEL_HI;
	uint32_t PAD_30_FUNC_SEL_LO;
	uint32_t PAD_31_FUNC_SEL_HI;
	uint32_t PAD_31_FUNC_SEL_LO;
	uint32_t PAD_32_FUNC_SEL_HI;
	uint32_t PAD_32_FUNC_SEL_LO;
	uint32_t PAD_33_FUNC_SEL_HI;
	uint32_t PAD_33_FUNC_SEL_LO;
	uint32_t PAD_34_FUNC_SEL_HI;
	uint32_t PAD_34_FUNC_SEL_LO;
	uint32_t PAD_35_FUNC_SEL_HI;
	uint32_t PAD_35_FUNC_SEL_LO;
	uint32_t PAD_36_FUNC_SEL_HI;
	uint32_t PAD_36_FUNC_SEL_LO;
	uint32_t PAD_37_FUNC_SEL_HI;
	uint32_t PAD_37_FUNC_SEL_LO;
	uint32_t PAD_38_FUNC_SEL_HI;
	uint32_t PAD_38_FUNC_SEL_LO;
	uint32_t PAD_39_FUNC_SEL_HI;
	uint32_t PAD_39_FUNC_SEL_LO;
} PAD_FUNC_SEL_Type;

/**
 * \brief  Structure type to access the PAD_LVDS_MODE registers.
 */
typedef struct {
	uint32_t PAD_JESD_SYNCIN_0_MODE;
	uint32_t PAD_JESD_SYNCIN_1_MODE;
	uint32_t PAD_JESD_SYNCOUT_0_MODE;
	uint32_t PAD_JESD_SYNCOUT_1_MODE;
	uint32_t PAD_JESD_SYNCOUT_2_MODE;
	uint32_t PAD_JESD_SYNCOUT_3_MODE;
	uint32_t PAD_JESD_SYNCOUT_4_MODE;
	uint32_t PAD_JESD_SYNCOUT_5_MODE;
} PAD_LVDS_MODE_Type;

/**
 * \brief  Structure type to access the PAD_PAD_CFG_CTRL registers.
 */
typedef struct {
	uint32_t QSPI_CS_CFG;
	uint32_t QSPI_SCK_CFG;
	uint32_t QSPI_DATA0_CFG;
	uint32_t QSPI_DATA1_CFG;
	uint32_t QSPI_DATA2_CFG;
	uint32_t QSPI_DATA3_CFG;
	uint32_t FHI_CLK_IN_CFG;
	uint32_t CLK_OUT_CFG;
	uint32_t GNSS_ONE_PPS_IN_CFG;
	uint32_t GNSS_ONE_PPS_OUT_CFG;
	uint32_t JESD_SYNCIN_0_CFG;
	uint32_t JESD_SYNCIN_1_CFG;
	uint32_t JESD_SYNCOUT_0_CFG;
	uint32_t JESD_SYNCOUT_1_CFG;
	uint32_t JESD_SYNCOUT_2_CFG;
	uint32_t JESD_SYNCOUT_3_CFG;
	uint32_t JESD_SYNCOUT_4_CFG;
	uint32_t JESD_SYNCOUT_5_CFG;
	uint32_t JESD_DEVCLK_CFG;
	uint32_t JESD_SYSREF_CFG;
	uint32_t RMII_TXD0_CFG;
	uint32_t RMII_TXD1_CFG;
	uint32_t RMII_TX_EN_CFG;
	uint32_t RMII_RXD0_CFG;
	uint32_t RMII_RXD1_CFG;
	uint32_t RMII_CRS_DV_CFG;
	uint32_t RMII_REF_CLK_CFG;
	uint32_t SPI_0_CLK_CFG;
	uint32_t SPI_0_DO_CFG;
	uint32_t SPI_0_DI_CFG;
	uint32_t SPI_0_CS0_CFG;
	uint32_t SPI_0_CS1_CFG;
	uint32_t SPI_1_CLK_CFG;
	uint32_t SPI_1_DO_CFG;
	uint32_t SPI_1_DI_CFG;
	uint32_t SPI_1_CS0_CFG;
	uint32_t SPI_1_CS1_CFG;
	uint32_t SPI_1_CS2_CFG;
	uint32_t SPI_1_CS3_CFG;
	uint32_t UART_0_RX_CFG;
	uint32_t UART_0_TX_CFG;
	uint32_t UART_1_RX_CFG;
	uint32_t UART_1_TX_CFG;
	uint32_t UART_2_RX_CFG;
	uint32_t UART_2_TX_CFG;
	uint32_t UART_2_CTS_CFG;
	uint32_t UART_2_RTS_CFG;
	uint32_t I2C_SCL0_CFG;
	uint32_t I2C_SDA0_CFG;
	uint32_t I2C_SCL1_CFG;
	uint32_t I2C_SDA1_CFG;
	uint32_t MDIO_MDC_CFG;
	uint32_t MDIO_MDIO_CFG;
	uint32_t GPIO_0_CFG;
	uint32_t GPIO_1_CFG;
	uint32_t GPIO_2_CFG;
	uint32_t GPIO_3_CFG;
	uint32_t GPIO_4_CFG;
	uint32_t GPIO_5_CFG;
	uint32_t GPIO_6_CFG;
	uint32_t GPIO_7_CFG;
	uint32_t GPIO_8_CFG;
	uint32_t GPIO_9_CFG;
	uint32_t GPIO_10_CFG;
	uint32_t GPIO_11_CFG;
	uint32_t GPIO_12_CFG;
	uint32_t GPIO_13_CFG;
	uint32_t GPIO_14_CFG;
	uint32_t GPIO_15_CFG;
	uint32_t GPIO_16_CFG;
	uint32_t GPIO_17_CFG;
	uint32_t GPIO_18_CFG;
	uint32_t GPIO_19_CFG;
	uint32_t GPIO_20_CFG;
	uint32_t GPIO_21_CFG;
	uint32_t GPIO_22_CFG;
	uint32_t GPIO_23_CFG;
	uint32_t GPIO_24_CFG;
	uint32_t GPIO_25_CFG;
	uint32_t GPIO_26_CFG;
	uint32_t GPIO_27_CFG;
	uint32_t GPIO_28_CFG;
	uint32_t GPIO_29_CFG;
	uint32_t GPIO_30_CFG;
	uint32_t GPIO_31_CFG;
	uint32_t JTAG_TMS_CFG;
	uint32_t JTAG_TCK_CFG;
	uint32_t JTAG_TDI_CFG;
	uint32_t JTAG_TDO_CFG;
	uint32_t JTAG_TRST_N_CFG;
	uint32_t JTAG_BSCOMPLIANCE_CFG;
	uint32_t ULPI_CLOCK_CFG;
	uint32_t ULPI_DATA_0_CFG;
	uint32_t ULPI_DATA_1_CFG;
	uint32_t ULPI_DATA_2_CFG;
	uint32_t ULPI_DATA_3_CFG;
	uint32_t ULPI_DATA_4_CFG;
	uint32_t ULPI_DATA_5_CFG;
	uint32_t ULPI_DATA_6_CFG;
	uint32_t ULPI_DATA_7_CFG;
	uint32_t ULPI_DIR_CFG;
	uint32_t ULPI_STP_CFG;
	uint32_t ULPI_NXT_CFG;
	uint32_t SYS_RST_IN_N_CFG;
	uint32_t SYS_RST_OUT_CFG;
	uint32_t SYS_CLK_CFG;
	uint32_t REPAIR_BYPASS_CFG;
	uint32_t BOOT_CONFIG_CFG;
	uint32_t DEV_ID_0_CFG;
	uint32_t DEV_ID_1_CFG;
} PAD_PAD_CFG_CTRL_Type;

struct pad_reg {
	PAD_GPIO_DBG_SEL_Type dbg_sel;
	PAD_GPIO_DBG_MODE_Type dbg_mode;
	PAD_FUNC_SEL_Type fun_sel;
	PAD_LVDS_MODE_Type lvds_mode;
	PAD_PAD_CFG_CTRL_Type cfg_ctrl;
};

#define PC805_MAX_GPIO 48

struct pc805_gpio {
	struct gpio_chip chip;
	void __iomem *pad_base; //!< Peripheral: gpio
	void __iomem *gpio_base; //!< Peripheral: pad
	struct gpio_reg *gpio;
	struct pad_reg *pad;
	spinlock_t lock;
};

static int pc805_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct gpio_reg *gpio;
	struct pc805_gpio *pc805;
	unsigned high_offset;
	uint32_t val;

	pc805 = gpiochip_get_data(chip);
	gpio = pc805->gpio;

	if (offset <= 31) {
		val = readl(&gpio->gpio_ctrl.DATA_IN_LOW);
		return (val >> offset) & 0x1;
	} else {
		high_offset = (offset - 32);
		val = readl(&gpio->gpio_ctrl.DATA_IN_HIGH);
		return (val >> high_offset) & 0x1;
	}
}

static void pc805_gpio_set(struct gpio_chip *chip, unsigned offset, int data)
{
	struct gpio_reg *gpio;
	struct pc805_gpio *pc805;
	unsigned high_offset;

	pc805 = gpiochip_get_data(chip);
	gpio = pc805->gpio;

	if (data) {
		if (offset <= 31) {
			writel((1 << offset),
			       &gpio->gpio_ctrl.DATA_OUT_DEF_SET_LOW);
			writel((1 << offset), &gpio->gpio_ctrl.OUT_IRQ_ACK_LOW);
		} else {
			high_offset = (offset - 32);
			writel((1 << high_offset),
			       &gpio->gpio_ctrl.DATA_OUT_DEF_SET_HIGH);
			writel((1 << high_offset),
			       &gpio->gpio_ctrl.OUT_IRQ_ACK_HIGH);
		}
	} else {
		if (offset <= 31) {
			writel((1 << offset),
			       &gpio->gpio_ctrl.DATA_OUT_DEF_CLEAR_LOW);
			writel((1 << offset), &gpio->gpio_ctrl.OUT_IRQ_ACK_LOW);
		} else {
			high_offset = (offset - 32);
			writel((1 << high_offset),
			       &gpio->gpio_ctrl.DATA_OUT_DEF_CLEAR_HIGH);
			writel((1 << high_offset),
			       &gpio->gpio_ctrl.OUT_IRQ_ACK_HIGH);
		}
	}
}

static int pc805_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned long flags;
	unsigned high_offset;
	struct pc805_gpio *pc805;
	struct gpio_reg *gpio;

	pc805 = gpiochip_get_data(chip);
	gpio = pc805->gpio;
	spin_lock_irqsave(&pc805->lock, flags);
	if (offset <= 31) {
		writel((1 << offset), &gpio->gpio_ctrl.DIR_CLEAR_LOW);
	} else {
		high_offset = offset - 32;
		writel((1 << high_offset), &gpio->gpio_ctrl.DIR_CLEAR_HIGH);
	}
	spin_unlock_irqrestore(&pc805->lock, flags);

	return 0;
}

static int pc805_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
				       int value)
{
	unsigned long flags;
	unsigned high_offset;
	struct pc805_gpio *pc805;
	struct gpio_reg *gpio;

	pc805 = gpiochip_get_data(chip);
	gpio = pc805->gpio;
	spin_lock_irqsave(&pc805->lock, flags);
	if (offset <= 31) {
		writel((1 << offset), &gpio->gpio_ctrl.DIR_SET_LOW);
	} else {
		high_offset = offset - 32;
		writel((1 << high_offset), &gpio->gpio_ctrl.DIR_SET_HIGH);
	}
	chip->set(chip, offset, value);
	spin_unlock_irqrestore(&pc805->lock, flags);

	return 0;
}

static const struct gpio_chip pc805_gpio_chip = {
	.label = "pc805",
	.owner = THIS_MODULE,
	.direction_input = pc805_gpio_direction_input,
	.get = pc805_gpio_get,
	.direction_output = pc805_gpio_direction_output,
	.set = pc805_gpio_set,
	.base = -1,
	.ngpio = PC805_MAX_GPIO,
	.can_sleep = false,
};

static int pc805_gpio_probe(struct platform_device *pdev)
{
	struct gpio_reg *gpio;
	struct pad_reg *pad;
	struct pc805_gpio *pc805;
	int ret;

	pc805 = devm_kzalloc(&pdev->dev, sizeof(*pc805), GFP_KERNEL);
	if (pc805 == NULL)
		return -ENOMEM;

	pc805->gpio_base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(pc805->gpio_base))
		return PTR_ERR(pc805->gpio_base);

	pc805->pad_base = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(pc805->pad_base))
		return PTR_ERR(pc805->pad_base);

	spin_lock_init(&pc805->lock);
	pc805->gpio = (struct gpio_reg *)pc805->gpio_base;
	pc805->pad = (struct pad_reg *)pc805->pad_base;
	pad = pc805->pad;
	gpio = pc805->gpio;

	/*config Pad function selection */
	writel(0, &pad->fun_sel.PAD_26_FUNC_SEL_LO);
	writel(0, &pad->fun_sel.PAD_27_FUNC_SEL_LO);

	writel(1, &pad->fun_sel.PAD_26_FUNC_SEL_HI);
	writel(2, &pad->fun_sel.PAD_27_FUNC_SEL_HI);

	/*config gpio control */
	writel(3, &gpio->gpio_ctrl.DIR_SET_HIGH);

	writel(1, &gpio->gpio_ctrl.DATA_OUT_DEF_CLEAR_HIGH);
	writel(1, &gpio->gpio_ctrl.OUT_IRQ_ACK_HIGH);

	pc805->chip = pc805_gpio_chip;
	pc805->chip.parent = &pdev->dev;

	ret = devm_gpiochip_add_data(&pdev->dev, &pc805->chip, pc805);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register gpiochip, %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, pc805);

	dev_info(&pdev->dev, "pc805 GPIO chip registered\n");

	return ret;
}

static const struct of_device_id pc805_gpio_of_match[] = {
	{
		.compatible = "picocom,pc805-gpio",
	},
	{}
};
MODULE_DEVICE_TABLE(of, pc805_gpio_of_match);

static struct platform_driver pc805_gpio_driver = {
	.probe = pc805_gpio_probe,
	.driver = {
		.name = "pc805-gpio",
		.of_match_table = pc805_gpio_of_match,
	},

};
module_platform_driver(pc805_gpio_driver);

MODULE_DESCRIPTION("pc805 GPIO driver");
MODULE_ALIAS("platform:pc805-gpio");
MODULE_AUTHOR("picocom");
MODULE_LICENSE("GPL");
