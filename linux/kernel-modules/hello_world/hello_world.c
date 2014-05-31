#include <linux/module.h>	// Included for all kernel modules
#include <linux/kernel.h>	// Included for KERN_INFO
#include <linux/init.h>		// Included for __init and __exit macros

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michal Malik");
MODULE_DESCRIPTION("A simple Hello World kernel module");

static int __init hello_init(void)
{
	printk(KERN_INFO "Hello World!\n");
	return 0;
}

static void __exit hello_cleanup(void)
{
	printk(KERN_INFO "Cleaning up Hello World module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);