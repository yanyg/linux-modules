#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/parport.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

#define DEVICE_NAME "led"

static dev_t dev_number;
static struct class *led_class;

struct cdev led_cdev;
struct pardevice *pdev;

int led_open(struct inode *inode, struct file *file)
{
	return 0;
}

int led_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t led_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	char kbuf;

	if (copy_from_user(&kbuf, buf, 1))
		return -EFAULT;

	parport_claim_or_block(pdev);

	parport_write_data(pdev->port, kbuf);

	parport_release(pdev);

	return count;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_release,
	.write = led_write,
};

static int led_preempt(void *handle)
{
	return 1;
}

static void led_attach(struct parport *port)
{
	pdev = parport_register_device(port, DEVICE_NAME, led_preempt, NULL, NULL, 0, NULL);

	if (!pdev)
		printk(KERN_WARNING "led: parport_register_device failed.\n");
}

static void led_detach(struct parport *port)
{
}

static struct parport_driver led_driver = {
	.name = "led",
	.attach = led_attach,
	.detach = led_detach,
};

int __init led_init(void)
{
	int r;
	if (alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "led: alloc_chrdev_region failed.\n");
		r = -EIO;
		goto ERR_alloc_chrdev_region;
	}

	led_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(led_class))
	{
		printk(KERN_WARNING "led: class_create failed.\n");
		r = PTR_ERR(led_class);
		goto ERR_class_create;
	}

	cdev_init(&led_cdev, &led_fops);
	if (cdev_add(&led_cdev, dev_number, 1)) {
		printk(KERN_WARNING "led: cdev_add failed.\n");
		r = -EIO;
		goto ERR_cdev_add;
	}

	device_create(led_class, NULL, dev_number, NULL, DEVICE_NAME);

	if (parport_register_driver(&led_driver)) {
		printk(KERN_WARNING "led: parport_register_driver failed.\n");
		r = -EIO;
		goto ERR_parport_register_driver;
	}

	printk(KERN_NOTICE "led: initialized success.\n");

	return 0;

ERR_parport_register_driver:
	device_destroy(led_class, dev_number);
	cdev_del(&led_cdev);
ERR_cdev_add:
	class_destroy(led_class);
ERR_class_create:
	unregister_chrdev_region(MAJOR(dev_number), 1);
ERR_alloc_chrdev_region:
	return r;
}

void led_exit(void)
{
	device_destroy(led_class, dev_number);
	cdev_del(&led_cdev);
	class_destroy(led_class);
	unregister_chrdev_region(MAJOR(dev_number), 1);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
