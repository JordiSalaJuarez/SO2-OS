#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  char test[64] = "Hello, world";
  itoa(69, test+4);
  write(1, test, sizeof(test));
  char buffer[64] = "hi hallo";
  itoa(12, buffer);
 	write(1, buffer, sizeof(buffer));


  perror();
  while(1) { }
}
