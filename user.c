#include <libc.h>

char buff[24];

int pid;


int fact(int i){
	if(i == 0) return 1;
	else return fact(i-1)*i;
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  runjp();
  while(1);
}
