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

#include <libc.h>

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

int ret_from_fork(){
	return 0;
}


int sys_fork()
{
  // creates the child process

	int PID=-1;

	if (list_empty(&freequeue)) return -EAGAIN;
	struct list_head *child_lh = list_first(&freequeue);
	list_del(child_lh);

	struct task_struct *child_ts = list_head_to_task_struct(child_lh);
	union task_union *child_tu = (union task_union *) child_ts;

	copy_data(current(), child_ts, sizeof(union task_union));

	child_ts->dir_pages_baseAddr = allocate_DIR(child_ts);

	page_table_entry * child_pt = get_PT(child_ts);
	page_table_entry * curr_pt = get_PT(current());

	// Copy shared pages entries of system  code and data
	for (int i = 0; i < NUM_PAG_KERNEL; ++i){
		child_pt[1 + i].entry = curr_pt[1 + i].entry;
	}

	// Copy shared pages entries of user code
	for (int i = 0; i < NUM_PAG_CODE; ++i){
		child_pt[PAG_LOG_INIT_CODE + i].entry = curr_pt[PAG_LOG_INIT_CODE + i].entry;
	}

	// frames addr are needed just in case it fails while allocating to free the previously allocated
	int new_frames[NUM_PAG_DATA];
	for(int i = 0; i < NUM_PAG_DATA; ++i){
		if((new_frames[i] = alloc_frame()) == -1){
			for(int j = 0 ; i <= i ; ++i) 
				free_frame(new_frames[j]);
			return -ENOMEM; // no free space left	
		}
		// map child physical address to a new frame 
		set_ss_pag(child_pt, PAG_LOG_INIT_DATA+i, new_frames[i]);
		// map parent physical addres to the same frame (logical addres PAG_LOG_INIT_DATA+NUM_PAG_DATA+page is empty)
		set_ss_pag(curr_pt, PAG_LOG_INIT_DATA+NUM_PAG_DATA+i, new_frames[i]);
		// copy data code data from parent to child because of same mapped physical region 
		copy_data( (int*)((PAG_LOG_INIT_DATA+i)<<12),
			   (int*)((PAG_LOG_INIT_DATA+NUM_PAG_DATA+i)<<12),
				PAGE_SIZE
			 );
		// remove parent from accessing the same physical region
		del_ss_pag(curr_pt, PAG_LOG_INIT_DATA+NUM_PAG_DATA+i);
	}
	// flush tlb
	set_cr3(get_DIR(current()));
	PID = MAX_PID++;
	child_ts->PID = PID;
	child_ts->ticks = 0;
	child_ts->state = ST_READY;
	child_ts->stats.remaining_ticks = current()->quantum;
	child_ts->quantum = current()->quantum;


	int index  = (getEBP() - (int) current())/sizeof(int);
	child_tu->stack[index] = (int) ret_from_fork;
	child_tu->stack[index-1] = 0;
	child_ts->esp= &((union task_union*)child_ts)->stack[index-1];;
	

	list_add_tail(child_lh, &readyqueue); 	

	return PID;

}


// int sys_fork()
// {
// 	int PID=-1;

// 	// creates the child process
// 	if (list_empty(&freequeue)) return -EAGAIN; // NO PROCESS LEFT
// 	struct list_head *new_task_ptr = list_first(&freequeue);
// 	list_del(new_task_ptr);

// 	struct task_struct *new_task = list_head_to_task_struct(new_task_ptr);

// 	// Copy data from parent to child
// 	copy_data(current(), new_task,(int) sizeof(union task_union));

// 	new_task->dir_pages_baseAddr = allocate_DIR(new_task);

// 	page_table_entry * n_pt = get_PT(new_task);
// 	page_table_entry * c_pt = get_PT(current());

// 	int page;
// 	for (page = 0; page < NUM_PAG_KERNEL; page++){
// 		n_pt[1+page].entry = c_pt[1+page].entry;
// 	}

// 	for (page = 0; page < NUM_PAG_CODE; page++){
// 		n_pt[PAG_LOG_INIT_CODE+page].entry = c_pt[PAG_LOG_INIT_CODE+page].entry;
// 	}

// 	int new_frames[NUM_PAG_DATA];
// 	for(page = 0; page< NUM_PAG_DATA; page++){
// 		if((new_frames[page] = alloc_frame()) < 0){
// 			for(page=page-1;page >=0;page--) 
// 				free_frame(new_frames[page]);
// 			return -ENOMEM; // no free space left	
// 		}
// 		set_ss_pag(n_pt, PAG_LOG_INIT_DATA+page, new_frames[page]);
// 		set_ss_pag(c_pt, PAG_LOG_INIT_DATA+NUM_PAG_DATA+page, new_frames[page]);
// 		copy_data( (int*)((PAG_LOG_INIT_DATA+page)<<12),
// 			   (int*)((PAG_LOG_INIT_DATA+NUM_PAG_DATA+page)<<12),
// 				PAGE_SIZE
// 			 );
// 		del_ss_pag(c_pt, PAG_LOG_INIT_DATA+NUM_PAG_DATA+page);
// 	}
// 	set_cr3(get_DIR(current()));
// 	PID = MAX_PID++;
// 	new_task->PID = PID;
// 	new_task->state = ST_READY;
// 	new_task->ticks = 0;
// 	int index  = (getEBP() - (int) current())/sizeof(int);
// 	((union task_union*)new_task)->stack[index] =(int) ret_from_fork;
// 	((union task_union*)new_task)->stack[index-1] = 0;
// 	new_task->esp= &((union task_union*)new_task)->stack[index-1];

// 	list_add_tail(new_task_ptr, &readyqueue); 	

// 	return PID;
// }



void sys_exit()
{	
	struct task_struct * a = current();
	
	page_table_entry * pt = get_PT(a);
	int page;
	for(page = 0; page< NUM_PAG_DATA; page++){
		free_frame(pt[PAG_LOG_INIT_DATA+page].bits.pbase_addr);
		del_ss_pag(pt, PAG_LOG_INIT_DATA+page);
	}
	update_process_state_rr(a,&freequeue);
	if(!list_empty(&readyqueue)){
		struct task_struct * ts = list_head_to_task_struct(list_first(&readyqueue));
		update_process_state_rr(ts, NULL);
		task_switch((union task_union*)ts);
	}else{
		task_switch((union task_union*) &idle_task);
	}	
}


int sys_get_stats(int pid, struct stats *st){
	struct task_struct *act;
	int i;
	if(!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;
	for (act = &(task[i=0].task); i < NR_TASKS; act = &(task[++i].task)){
		if (act -> PID == pid){
			copy_to_user(&(act->stats), st, sizeof(struct stats));	
			return 0;
		}
	}	
	return -EINVAL;
}


// void sys_exit()
// {	
// 	page_table_entry * pt = get_PT(current());
// 	for(int page = 0; page< NUM_PAG_DATA; page++){
// 		free_frame(pt[PAG_LOG_INIT_DATA+page].bits.pbase_addr);
// 		del_ss_pag(pt, PAG_LOG_INIT_DATA+page);
// 	}
// 	update_process_state_rr(current(), &freequeue);
// 	sched_next_rr();
// }

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

