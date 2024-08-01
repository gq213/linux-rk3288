// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/pinctrl/consumer.h>

enum {
	BT_WAKE_OUT = 0,
	BT_HOST_WAKE_IN,
	BT_POWERON_OUT,
	BT_RTS_OUT,
	BT_GPIO_MAX,
};

const char *key_desc[BT_GPIO_MAX] = {
	"bt-wake", "bt-host-wake", "bt-poweron", "uart-rts", 
};

struct mydrv_bt_data {
	struct gpio_desc *gpiod[BT_GPIO_MAX];
	struct clk *lpo_clk;
	struct pinctrl *pinctrl;
	struct pinctrl_state *default_state;
	struct pinctrl_state *gpio_state;
	int power_state;
};

static struct mydrv_bt_data *p_data = NULL;

static ssize_t power_enable_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", p_data->power_state);
}

static ssize_t power_enable_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	if (count >= 1 && buf[0] == '0') {
		if (p_data->power_state) {
			p_data->power_state = 0;
			
			gpiod_set_value_cansleep(p_data->gpiod[BT_POWERON_OUT], 0);
			msleep(20);
		}
	} else if (count >= 1 && buf[0] == '1') {
		if (!p_data->power_state) {
			p_data->power_state = 1;
			
			gpiod_direction_output(p_data->gpiod[BT_HOST_WAKE_IN], 1);
			msleep(20);
			
			gpiod_set_value_cansleep(p_data->gpiod[BT_POWERON_OUT], 0);
			msleep(20);
			gpiod_set_value_cansleep(p_data->gpiod[BT_POWERON_OUT], 1);
			msleep(20);
			
			gpiod_direction_input(p_data->gpiod[BT_HOST_WAKE_IN]);
			
			pinctrl_select_state(p_data->pinctrl, p_data->gpio_state);
			gpiod_set_value_cansleep(p_data->gpiod[BT_RTS_OUT], 1);
			msleep(100);
			gpiod_set_value_cansleep(p_data->gpiod[BT_RTS_OUT], 0);
			pinctrl_select_state(p_data->pinctrl, p_data->default_state);
		}
	}
	
	return strlen(buf);
}

static DEVICE_ATTR(power_enable, 0644, power_enable_show, power_enable_store);

static int _probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int i;
	struct gpio_desc *gpiod;
	int ret;
	
	p_data = devm_kzalloc(dev, sizeof(*p_data), GFP_KERNEL);
	if (!p_data) {
		dev_err(dev, "failed to allocate mydrv_bt_data\n");
		return -ENOMEM;
	}
	
	for (i=0; i<BT_GPIO_MAX; i++) {
		if (i == BT_HOST_WAKE_IN)
			gpiod = devm_gpiod_get(dev, key_desc[i], GPIOD_IN);
		else
			gpiod = devm_gpiod_get(dev, key_desc[i], GPIOD_OUT_LOW);
		if (IS_ERR(gpiod)) {
			dev_err(dev, "Failed to get %s-gpios\n", key_desc[i]);
			return PTR_ERR(gpiod);
		}
		
		p_data->gpiod[i] = gpiod;
	}
	
	p_data->lpo_clk = devm_clk_get(dev, "lpo_clock");
	if (IS_ERR(p_data->lpo_clk)) {
		dev_err(dev, "Failed to get lpo_clk\n");
		return -1;
	}
	
	ret = clk_prepare_enable(p_data->lpo_clk);
	if (ret) {
		dev_err(dev, "Failed to enable lpo_clk\n");
		return -1;
	}
	
	gpiod_set_value_cansleep(p_data->gpiod[BT_WAKE_OUT], 1);
	
	p_data->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(p_data->pinctrl)) {
		dev_err(dev, "Failed to get pinctrl\n");
		return -1;
	}
	
	p_data->default_state = pinctrl_lookup_state(p_data->pinctrl, "default");
	if (IS_ERR(p_data->default_state)) {
		dev_err(dev, "Cannot find pinctrl default\n");
		return -1;
	}
	p_data->gpio_state = pinctrl_lookup_state(p_data->pinctrl, "rts_gpio");
	if (IS_ERR(p_data->gpio_state)) {
		dev_err(dev, "Cannot find pinctrl rts_gpio\n");
		return -1;
	}

	ret = device_create_file(dev, &dev_attr_power_enable);
	if (ret) {
		dev_err(dev, "Error creating power_enable\n");
		return -1;
	}
	
	dev_info(dev, "%s: done\n", __func__);

	return 0;
}

static int _remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	
	device_remove_file(dev, &dev_attr_power_enable);
	
	dev_info(dev, "%s: done\n", __func__);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id match_[] = {
	{ .compatible = "mydrv,bt", },
	{ }
};
MODULE_DEVICE_TABLE(of, match_);
#endif

static struct platform_driver driver_ = {
	.driver = {
		.name = "mydrv_bt",
		.of_match_table = of_match_ptr(match_),
	},
	.probe = _probe,
	.remove = _remove,
};

static int __init _driver_init(void)
{
	return platform_driver_register(&driver_);
}
module_init(_driver_init);

static void __exit _driver_exit(void)
{
	platform_driver_unregister(&driver_);
}
module_exit(_driver_exit);

MODULE_AUTHOR("gq213 <gaoqiang1211@gmail.com>");
MODULE_LICENSE("GPL v2");
