#include <libc.h>


#if 0

int pid;


int stack[4096];

char buff[sizeof("BBBBBBBBBBBB")] = "BBBBBBBBBBBB";
int xx=666;

int fib(int i){
  if (i == 0)return 0;
  else if (i == 1) return 1;
  else return fib(i-2) + fib(i-1); 
}

void my_func(void){
  int x = fib(7);
  itoa(x,buff);
  write(1, buff, sizeof(buff)); 
  exit();
}

#endif

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  #if 0
  int x = fib(7);
  itoa(x,buff);
  write(1, buff, sizeof(buff));
  clone(my_func, &stack[4096]);
  while(1);
  #endif
  
  runjp();
  while(1);
}
