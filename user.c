#include <libc.h>



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
  xx = 42;
  write(1, buff, sizeof(buff));
  
  exit();
}



int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  // runjp_rank(0,50);
  // while(1);
    int x = fib(7);
  itoa(x,buff);
  write(1, buff, sizeof(buff));
  clone(my_func, &stack[4096]);
  while(xx==666){};
  write(1,"ok", 2);
  while(1);
}
