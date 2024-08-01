// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define CUSTOM_KERNEL_VERSION 1

static ssize_t kernel_version_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", CUSTOM_KERNEL_VERSION);
}

static DEVICE_ATTR(kernel_version, 0444, kernel_version_show, NULL);

static int _probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	
	device_create_file(dev, &dev_attr_kernel_version);
	
	dev_info(dev, "probe done\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id match_[] = {
	{ .compatible = "mydrv,ver", },
	{ }
};
MODULE_DEVICE_TABLE(of, match_);
#endif

static struct platform_driver driver_ = {
	.driver = {
		.name = "mydrv_ver",
		.of_match_table = of_match_ptr(match_),
	},
	.probe = _probe,
};

static int __init _driver_init(void)
{
	return platform_driver_register(&driver_);
}
fs_initcall(_driver_init);

static void __exit _driver_exit(void)
{
	platform_driver_unregister(&driver_);
}
module_exit(_driver_exit);

MODULE_AUTHOR("gq213 <gaoqiang1211@gmail.com>");
MODULE_LICENSE("GPL v2");
