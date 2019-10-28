/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF;
  if (permissions!=ESCRIPTURA) return -EACCES;
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS;
}


int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;
  union task_union * tu_child;
  	if !list_empty(&free_queue) return -1;
	struct list_head *it_aux = list_first(&free_queue);
	struct task_struct * it_ts = list_head_to_task_struct(it_aux);
	tu_child = (union task_union *) it_ts;
	copy_data(current(),tu_child, KERNEL_STACK_SIZE * sizeof(unsigned long));
	it_ts->dir_pages_baseAddr = allocate_DIR(it_ts);
	if(it_ts->dir_pages_baseAddr != current()->dir_pages_baseAddr) set_sr3(it_ts->dir_pages_baseAddr);
	set_user_pages(it_ts);

	 int pag; 
	 int new_ph_pag;
	 page_table_entry * child_PT, parent_PT;
	 child_PT = get_PT(it_ts);
	 parent_PT = get_PT(current());
	 child_PT[PAG_LOG_INIT_CODE+pag].bits.pbase_add = alloc_frame();

}

  // creates the child process

  return PID;
}

void sys_exit()
{
}

#define CHUNK_SIZE 64

int sys_write(int fd, char *buffer, int size)
{
	int error;
	if((error=check_fd(fd, ESCRIPTURA))) return error;
	if(buffer == NULL) return -EFAULT;
	if(size < 0) return -EINVAL;
	char sys_addr[CHUNK_SIZE];
	int written = 0;
	for(int i = 0; i < size; i += CHUNK_SIZE)
	{
		if((error = copy_from_user(buffer+i, sys_addr, min(CHUNK_SIZE, size-i)))) return error;
		written += sys_write_console(sys_addr, min(CHUNK_SIZE, size-i));
	}
	return written;
}

int sys_gettime()
{
	return zeos_ticks;
}

