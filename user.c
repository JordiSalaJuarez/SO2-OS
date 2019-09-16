#include <libc.h>
#include<asm.h>

ENTRY(addASM)

char buff[24];

int pid;

int add(int partial, int par2) {
	return partial + par2;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  int x = add(0x42, 0x666);  
  while(1) { }
}
