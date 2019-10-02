/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process

  return PID;
}

void sys_exit()
{
}

#define CHUNK_SIZE 64

int sys_write(int fd, char *buffer, int size){
  int error;
  if ((error = check_fd(fd, ESCRIPTURA)) < 0)  return error;
  if ((buffer) == NULL)     	                 return -1;
  if (size < 0)                     	         return -1;
  char sys_addr[CHUNK_SIZE];
  int written = 0;
  for (int i = 0; i < size; i += CHUNK_SIZE){
      if((error = copy_from_user(buffer+i,sys_addr, min(CHUNK_SIZE, size-i))) < 0) return error;
      written += sys_write_console(sys_addr, min(CHUNK_SIZE, size-i));
  }
  return written;
}

int sys_gettime(){
  return zeos_ticks;
}
