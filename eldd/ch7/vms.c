#include <linux/fs.h>
#include <asm-generic/uaccess.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>

struct input_dev *vms_input_dev;
static struct platform_device *vms_dev;

static ssize_t vms_write(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int x, y;

	sscanf(buf, "%d %d", &x, &y);

	printk(KERN_INFO "x = %d, y = %d", x, y);

	input_report_rel(vms_input_dev, REL_X, x);
	input_report_rel(vms_input_dev, REL_Y, y);
	input_sync(vms_input_dev);

	return count;
}

DEVICE_ATTR(coordinates, 0666, NULL, vms_write);

static struct attribute *vms_attrs[] = {
	&dev_attr_coordinates.attr,
	NULL,
};

static struct attribute_group vms_attr_group = {
	.attrs = vms_attrs
};

int __init vms_init(void)
{
	int r = 0;

	vms_dev = platform_device_register_simple("vms", -1, NULL, 0);

	if (IS_ERR(vms_dev)) {
		printk(KERN_WARNING "platform_device_register_simple failed.\n");
		r = PTR_ERR(vms_dev);
		goto ERR_platform_device_register_simple;
	}

	r = sysfs_create_group(&vms_dev->dev.kobj, &vms_attr_group);
	if (r) {
		printk("sysfs_create_group failed.\n");
		goto ERR_sysfs_create_group;
	}

	vms_input_dev = input_allocate_device();
	if (!vms_input_dev) {
		printk("input_allocate_device failed.\n");
		r = -EIO;
		goto ERR_input_allocate_device;
	}

	set_bit(EV_REL, vms_input_dev->evbit);
	set_bit(REL_X, vms_input_dev->relbit);
	set_bit(REL_Y, vms_input_dev->relbit);

	r = input_register_device(vms_input_dev);
	if (r) {
		printk("input_register_device failed.\n");
		goto ERR_input_register_device;
	}
	printk(KERN_INFO "vms initialized success.\n");

	return 0;

ERR_input_register_device:
	input_free_device(vms_input_dev);
ERR_input_allocate_device:
	sysfs_remove_group(&vms_dev->dev.kobj, &vms_attr_group);
ERR_sysfs_create_group:
	platform_device_unregister(vms_dev);
ERR_platform_device_register_simple:
	return r;
}

void __exit vms_exit(void)
{
	input_unregister_device(vms_input_dev);

	sysfs_remove_group(&vms_dev->dev.kobj, &vms_attr_group);

	platform_device_unregister(vms_dev);
}

module_init(vms_init);
module_exit(vms_exit);

MODULE_LICENSE("GPL");
