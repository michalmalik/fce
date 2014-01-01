#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michal Malik");
MODULE_DESCRIPTION("Hook sys_open syscall");

static void **sys_call_table;
static const unsigned long sys_call_addr = 0xc0493148;

asmlinkage int (*original_call)(const char *, int, int);

asmlinkage int our_sys_open(const char *file, int flags, int mode)
{
	printk(KERN_INFO "[hook sys_open] \"%s\"", file);
	return original_call(file, flags, mode);
}

static int make_rw(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);

	if(pte->pte & ~_PAGE_RW)
		pte->pte |= _PAGE_RW;

	return 0;
}

static int make_ro(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte & ~_PAGE_RW;

	return 0;
}

static int __init hook_init(void)
{
	sys_call_table = (void *)sys_call_addr;
	original_call = sys_call_table[__NR_open];
	make_rw((unsigned long)sys_call_table);
	sys_call_table[__NR_open] = our_sys_open;

	return 0;
}

static void __exit hook_cleanup(void)
{
	sys_call_table[__NR_open] = original_call;
	make_ro((unsigned long)(sys_call_table));
}

module_init(hook_init);
module_exit(hook_cleanup);