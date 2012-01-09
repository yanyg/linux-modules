#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/moduleparam.h>

static int param_int = 100;
static bool param_bool = true;
static bool param_invbool = false;
static char *param_charp = "init-charp";
static char param_string_real[8] = "string";
static int param_array_int[8];
static int param_array_size;

module_param(param_int, int, S_IRUGO);
MODULE_PARM_DESC(param_int, "\n\t\t\tparam_int is a integer, you should"
			    "\n\t\t\tprovide a integer value, default is 100");
module_param(param_bool, bool, S_IRUGO);
module_param(param_invbool, invbool, S_IRUGO);
module_param(param_charp, charp, S_IRUGO);

module_param_string(param_string, param_string_real, sizeof(param_string_real), 0644);
MODULE_PARM_DESC(param_string, " param_string, a copied string");

module_param_array(param_array_int, int, &param_array_size, 0644);

static void print_params(void)
{
	int i;
	printk(KERN_INFO "param_int        : %d\n", param_int);
	printk(KERN_INFO "param_bool       : %s\n", param_bool ? "true" : "false");
	printk(KERN_INFO "param_invbool    : %s\n", param_invbool ? "true" : "false");
	printk(KERN_INFO "param_charp      : %s\n", param_charp);
	printk(KERN_INFO "param_string     : %s\n", param_string_real);
	printk(KERN_INFO "param_array_size : %d\n", param_array_size);

	printk(KERN_INFO "param_array_int  : ");
	for (i = 0; i < param_array_size; ++i) {
		printk("%d ", param_array_int[i]);
	}
	printk("\n");
}

static int __init params_init(void)
{
	print_params();

	return 0;
}

static void __exit params_exit(void)
{
}

module_init(params_init);
module_exit(params_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("illustrates linux-module running-parameters");
