#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  char b[80];
  int i = 0;
  switch (fork())
  {
  case 0:
      while(420){
        itoa(++i, b);
        write(1, "Child was called ",sizeof("Child was called "));
        int e = write(1, b, sizeof(b));
        write(1," times\n",sizeof(" times\n"));
      }
      break;
  
  default:
    break;
  }


  while(1) {
    itoa(++i, b);
    itoa(gettime(), b);
        write(1, "Parent was called ",sizeof("Parent was called "));
        int e = write(1, b, sizeof(b));
        write(1," times\n",sizeof(" times\n"));
  }
}
