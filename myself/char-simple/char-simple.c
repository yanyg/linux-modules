#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/device.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("yanyg <yanyg02@gmail.com>");
MODULE_VERSION("v1.0");

/* cs stands for char-simple */
static dev_t cs_devnum;
static unsigned cs_baseminor = 0;
static unsigned cs_count = 4;
static struct cdev cs_cdev;
static struct class *cs_class;

#define CS_SIZE	32
static char cs_buf[CS_SIZE+1] = "uninit char-simple\n";

static ssize_t cs_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t cs_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

static struct file_operations cs_fops = 
{
	.owner = THIS_MODULE,
	.read = cs_read,
	.write = cs_write,
};

#define CS_NAME "char-simple"

ssize_t cs_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	if (*f_pos >= CS_SIZE) return 0;
	if (*f_pos + count > CS_SIZE) count = CS_SIZE - *f_pos;

	if ( copy_to_user(buf, cs_buf + *f_pos, count) ) return -EINVAL;

	*f_pos += count;

	return count;
}

ssize_t cs_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	if (*f_pos >= CS_SIZE) return -ENOBUFS;

	if (*f_pos + count > CS_SIZE) count = CS_SIZE - *f_pos;

	if ( copy_from_user(cs_buf + *f_pos, buf, count) ) return -EFAULT;

	*f_pos += count;
	if (cs_buf[*f_pos-1] != '\n' ) cs_buf [*f_pos] = '\n';

	memset(cs_buf+*f_pos, 0, sizeof cs_buf - *f_pos);

	return count;
}

static int __init char_simple_init(void)
{
	int r;

	printk(KERN_INFO "init char_simple ...\n");

	r = alloc_chrdev_region(&cs_devnum, cs_baseminor, cs_count, CS_NAME);
	if ( r < 0 )
	{
		printk(KERN_WARNING "alloc_chrdev_region failed.\n");
		return r;
	}

	cs_class = class_create(THIS_MODULE, CS_NAME);

	cdev_init(&cs_cdev, &cs_fops);
	r = cdev_add(&cs_cdev, cs_devnum, cs_count);
	
	if ( r < 0 )
	{
		printk(KERN_WARNING "cdev_add failed.\n");
		unregister_chrdev_region(cs_devnum, cs_count);
		return r;
	}

	device_create(cs_class, NULL, cs_devnum, NULL, CS_NAME);

	return 0;
}

static void __exit char_simple_exit(void)
{
	cdev_del(&cs_cdev);
	unregister_chrdev_region(cs_devnum, cs_count);
	device_destroy(cs_class, cs_devnum);
	class_destroy(cs_class);
	printk(KERN_INFO "exit char_simple ...\n");
}

module_init(char_simple_init);
module_exit(char_simple_exit);
