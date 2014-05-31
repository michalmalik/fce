#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#define KERN_START	0xc0000000

static int fd;
static unsigned long syscall_handler;

struct idtr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct idt_entry {
	uint16_t lo;
	uint16_t css;
	uint16_t flags;
	uint16_t hi;
} __attribute__((packed));

int read_virt(unsigned long addr, void *buf, unsigned int len)
{
	addr -= KERN_START;
	assert(lseek(fd, addr, SEEK_SET) == addr);
	return read(fd, buf, len);
}

int main(int argc, char **argv)
{
	fd = open("/dev/mem", O_RDWR);
	struct idtr idtr;
	struct idt_entry idt;

	assert(fd >= 0);

	__asm__ volatile("sidt %0" : "=m"(idtr));
	idtr.base = 0xc060d000;

	printf("idtr.base 0x%x(phys)\nidtr.limit 0x%x\n", idtr.base-KERN_START, idtr.limit);

	assert(read_virt(idtr.base+sizeof(struct idt_entry)*0x80, &idt, sizeof(struct idt_entry)) == sizeof(struct idt_entry));
	syscall_handler = (idt.hi<<16)|idt.lo;

	printf("idt.lo 0x%x\n", idt.lo);
	printf("idt.css 0x%x\n", idt.css);
	printf("idt.flags 0x%x\n", idt.flags);
	printf("idt.hi 0x%x\n", idt.hi);
	printf("syscall_handler 0x%x(+ KERN_START 0x%x)\n", syscall_handler, KERN_START);

	close(fd);
	return 0;
}