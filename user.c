#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  int a = write(1, "HOLAAAA\n" , sizeof("HOLAAAA\n"));
  //runjp_rank(3, 10);
  //runjp();


  char b[80];

  while(1) {
  //	itoa(gettime(), b);
  //	int e = write(1, b, sizeof(b));
  }
}
