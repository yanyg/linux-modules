#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/types.h>

#define NUM_CMOS_BANKS		2
#define CMOS_BANK_SIZE		(0xff*8)
#define DEVICE_NAME			"cmos"

/* The original port range is 0x70~0x73, but 0x70~0x71 are used by rtc0 */
#define CMOS_BANK0_INDEX_PORT	0x72
#define CMOS_BANK0_DATA_PORT	0x73
#define CMOS_BANK1_INDEX_PORT	0x74
#define CMOS_BANK1_DATA_PORT	0x75

unsigned char addrports[NUM_CMOS_BANKS] = { CMOS_BANK0_INDEX_PORT, CMOS_BANK1_INDEX_PORT, };
unsigned char dataports[NUM_CMOS_BANKS] = { CMOS_BANK0_DATA_PORT, CMOS_BANK1_DATA_PORT, };

struct cmos_dev {
	unsigned short current_pointer; /* current pointer within the bank */

	unsigned int size;
	int bank_number;

	struct cdev cdev;

	char name[10];
}*cmos_devp[NUM_CMOS_BANKS];

static int cmos_open(struct inode *inode, struct file *file)
{
	struct cmos_dev *p = container_of(inode->i_cdev, struct cmos_dev, cdev);

	file->private_data = p;
	p->size = CMOS_BANK_SIZE;
	p->current_pointer = 0;

	return 0;
}

static int cmos_release(struct inode *inode, struct file *file)
{
	struct cmos_dev *p = file->private_data;

	p->current_pointer = 0;

	return 0;
}

static ssize_t cmos_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	return -1;
}

static ssize_t cmos_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	return -1;
}

static loff_t cmos_llseek(struct file *file, loff_t offset, int orig)
{
	return 0;
}

static long cmos_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct file_operations cmos_fops = {
	.owner = THIS_MODULE,
	.open = cmos_open,
	.release = cmos_release,
	.read = cmos_read,
	.write = cmos_write,
	.llseek = cmos_llseek,
	.compat_ioctl = cmos_ioctl,
};

static dev_t cmos_dev_number;
struct class *cmos_class;

int __init cmos_init(void)
{
	int i, j, r = 0;

	if ( alloc_chrdev_region(&cmos_dev_number, 0, NUM_CMOS_BANKS, DEVICE_NAME) < 0 )
	{
		printk(KERN_WARNING "cmos alloc_chrdev_region failed\n");
		r = -EIO;
		goto ERR_alloc_chrdev_region;
	}

	cmos_class = class_create(THIS_MODULE, DEVICE_NAME);
	if ( IS_ERR(cmos_class) )
	{
		printk(KERN_WARNING "cmos: class_create failed.\n");
		r = PTR_ERR(cmos_class);
		goto ERR_class_create;
	}

	for ( i = 0; i < NUM_CMOS_BANKS; ++i )
	{
		cmos_devp[i] = kcalloc(1, sizeof(struct cmos_dev), GFP_KERNEL);
		if ( !cmos_devp[i] )
		{
			printk(KERN_WARNING "cmos: kmalloc cmode_devp failed.\n");
			r = -ENOMEM;
			goto ERR_kmalloc;
		}

		sprintf(cmos_devp[i]->name, "cmos%d", i);
		if ( !request_region(addrports[i], 2, cmos_devp[i]->name) )
		{
			printk(KERN_WARNING "cmos: I/O port 0x%x is not free.\n", addrports[i]);
			r = -EBUSY;
			goto ERR_request_region;
		}

		cmos_devp[i]->bank_number = i;

		cdev_init(&cmos_devp[i]->cdev, &cmos_fops);
		cmos_devp[i]->cdev.owner = THIS_MODULE;

		if ( cdev_add(&cmos_devp[i]->cdev, MKDEV(MAJOR(cmos_dev_number),i), 1) )
		{
			printk(KERN_WARNING "cmos: cdev_add failed.\n");
			r = -EIO;
			goto ERR_cdev_add;
		}

		device_create(cmos_class, NULL, MKDEV(MAJOR(cmos_dev_number),i), NULL, "cmos%d", i);
	}

	printk(KERN_INFO "cmos: initialized.\n");
	return 0;

ERR_cdev_add:
	release_region(addrports[i], 2);
ERR_request_region:
	kfree(cmos_devp[i]);
ERR_kmalloc:
	for ( j = 0; j < i; ++j )
	{
		device_destroy( cmos_class, MKDEV(MAJOR(cmos_dev_number), j) );
		release_region( addrports[j], 2);
		cdev_del(&cmos_devp[j]->cdev);
		kfree(cmos_devp[j]);
	}
	class_destroy(cmos_class);
ERR_class_create:
	unregister_chrdev_region(cmos_dev_number, NUM_CMOS_BANKS);
ERR_alloc_chrdev_region:
	return r;
}

void __exit cmos_exit(void)
{
	int i;

	for ( i = 0; i < NUM_CMOS_BANKS; ++i )
	{
		device_destroy( cmos_class, MKDEV( MAJOR(cmos_dev_number), i) );
		release_region(addrports[i], 2);
		cdev_del(&cmos_devp[i]->cdev);
		kfree(cmos_devp[i]);
	}
	class_destroy(cmos_class);
	unregister_chrdev_region(cmos_dev_number, NUM_CMOS_BANKS);
}

module_init(cmos_init);
module_exit(cmos_exit);

MODULE_LICENSE("GPL");

/*
 * eof
 */

