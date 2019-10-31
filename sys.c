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

extern int MAX_PID;

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
  // creates the child process

	int PID=-1;

	if (list_empty(&free_queue)) return -1;
	struct list_head *child_lh = list_first(&free_queue);
	struct task_struct *child_ts = list_head_to_task_struct(child_lh);
	union task_union *child_tu = (union task_union *) child_ts;
	list_del(child_lh);
	copy_data(current(), child_lh, (int) sizeof(union task_union));

	allocate_DIR(child_ts);

	page_table_entry * child_pt = get_PT(child_ts);
	page_table_entry * curr_pt = get_PT(current());

	for (int page = 0; page < NUM_PAG_KERNEL; page++){
		child_pt[1+page].entry = curr_pt[1+page].entry;
	}

	for (int page = 0; page < NUM_PAG_CODE; page++){
		child_pt[PAG_LOG_INIT_CODE+page].entry = curr_pt[PAG_LOG_INIT_CODE+page].entry;
	}

	int new_frames[NUM_PAG_DATA];
	for(int page = 0; page < NUM_PAG_DATA; page++){
		if((new_frames[page] = alloc_frame()) < 0){
			for(page=page-1;page >=0;page--) 
				free_frame(new_frames[page]);
			return -ENOMEM; // no free space left	
		}
		set_ss_pag(child_pt, PAG_LOG_INIT_DATA+page, new_frames[page]);
		set_ss_pag(curr_pt, PAG_LOG_INIT_DATA+NUM_PAG_DATA+page, new_frames[page]);
		copy_data( (int*)((PAG_LOG_INIT_DATA+page)<<12),
			   (int*)((PAG_LOG_INIT_DATA+NUM_PAG_DATA+page)<<12),
				PAGE_SIZE
			 );
		del_ss_pag(child_pt, PAG_LOG_INIT_DATA+NUM_PAG_DATA+page);
	}

	set_cr3(get_DIR(current()));
	PID = MAX_PID++;
	child_ts->PID = PID;
	int index  = (getEbp() - (int) current())/sizeof(int);
	child_tu->stack[index] = (int) ret_from_fork;
	child_tu->stack[index-1] = 0;
	child_ts->esp= &child_tu->stack[index-1];

	list_add_tail(child_lh, &ready_queue); 	

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

