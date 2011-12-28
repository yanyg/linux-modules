#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/parport.h>
#include <asm-generic/uaccess.h>
#include <linux/pci.h>

static dev_t dev_number;
static struct class *led_class;
struct cdev led_cdev;
struct pardevice *pdev;

struct kobject kobj;

struct led_attr {
	struct attribute attr;
	ssize_t (*show)(char *);
	ssize_t (*store)(const char *, size_t count);
};
