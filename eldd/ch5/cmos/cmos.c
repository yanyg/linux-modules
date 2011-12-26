#include <linux/fs.h>

#define NUM_CMOS_BANKS		2
#define CMOS_BANK_SIZE		(0xff*8)
#define DEVICE_NAME			"cmos"
#define CMOS_BANK0_INDEX_PORT	0x70
#define CMOS_BANK0_DATA_PORT	0x71
#define CMOS_BANK1_INDEX_PORT	0x72
#define CMOS_BANK1_DATA_PORT	0x73

unsigned char addrports[NUM_CMOS_BANKS] = { CMOS_BANK0_INDEX_PORT, CMOS_BANK1_INDEX_PORT, };
unsigned char dataports[NUM_CMOS_BANKS] = { CMOS_BANK0_DATA_PORT, CMOS_BANK1_DATA_PORT, };

struct cmos_dev {
	unsigned short current_pointer	/* current pointer within the bank */

	unsigned int size;
	int bank_number;

	struct cdev dev;

	char name[10];
}*cmos_devp[NUM_CMOS_BANKS];

static struct file_operations cmos_fops = {
	.owner = THIS_MODULE,
	.open = cmos_open,
	.release = cmos_release,
	.read = cmos_read,
	.write = cmos_write,
	.llseek = cmos_llseek,
	.ioctl = cmos_ioctl,
};

static dev_t cmos_dev_number;
struct class cmos_class;

int __init cmos_init(void)
{
	int i, j, r;

	if ( alloc_chrdev_region(&cmos_dev_number, 0, NUM_CMOS_BANKS, DEVICE_NAME) < 0 )
	{
		printk(KERN_WARN "cmos alloc_chrdev_region failed\n");
		r = -EIO;
		goto ERR_alloc_chrdev_region;
	}

	cmos_class = class_create(THIS_MODULE, DEVICE_NAME);
	if ( IS_ERR(cmos_class) )
	{
		printk(KERN_WARN "cmos: class_create failed.\n");
		r = PTR_ERR(cmos_class);
		goto ERR_class_create;
	}

	for ( i = 0; i < NUM_CMOS_BANKS; ++i )
	{
		cmos_devp[i] = kzmalloc(sizeof(struct cmos_dev), GFP_KERNEL);
		if ( !cmos_devp[i] )
		{
			printk(KERN_WARN "cmos: kmalloc cmode_devp failed.\n");
			goto ERR_kmalloc;
		}

		sprintf(cmos_devp[i]->name, "cmos%d", i);
		if ( !request_region(addrports[i], 2, cmos_devp[i]->name) )
		{
			printk(KERN_WARN "cmos: I/O port 0x%x is not free.\n", addrports[i]);
			goto ERR_request_region;
		}

		cmos_devp[i]->bank_number = i;

		cdev_init(&cmos_devp[i]->cdev, &cmos_fops);
		cmos_devp[i]->owner = THIS_MODULE;

		if ( cdev_add(&cmos_devp[i]->cdev, dev_number+i, 1) )
		{
			printk(KERN_WARN "cmos: cdev_add failed.\n");
			goto ERR_cdev_add;
		}

		device_create(cmos_class, NULL, (dev_number+i), NULL, "cmos%d", i);
	}

	printk(KERN_INFO "cmos: initialized.\n");
	return 0;

ERR_ERR_cdev_add:
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
