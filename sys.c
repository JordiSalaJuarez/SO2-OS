/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>
#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>
#include <dict.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int dir_pages_n_refs[NR_TASKS];
extern struct sem sems[NR_SEMS];
extern dict *dict_sems;
extern struct list_head free_sems;


void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_clone(void (* function)(void), void *stack)
{

  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  if (!access_ok(VERIFY_WRITE, stack, sizeof(void*)))   return -EFAULT;
	if (!access_ok(VERIFY_READ, function, sizeof(void*))) return -EFAULT;

  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* indcrease number of references on directory */
  dir_pages_n_refs[get_DIR_index((struct task_struct *)uchild)]++; 

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;
  uchild->task.sem_destroyed=0;
  uchild->task.len_chars_read=0;
  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

	uchild->stack[KERNEL_STACK_SIZE-2]=(int)stack;
	uchild->stack[KERNEL_STACK_SIZE-5]=(int)function;


  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}


int sys_sem_init(int n_sem, unsigned int value){
  if(contains(dict_sems, n_sem))return -EBUSY;
  struct list_head *new = list_first(&free_sems);
  list_del(new);
  INIT_LIST_HEAD(new);
  struct sem *s = (struct sem *) new;
  s->owner = current()->PID;
  s->counter = value;
  set_item(dict_sems, n_sem, s);
  return 0;

}

int sys_sem_wait(int n_sem){
  if(!contains(dict_sems, n_sem))return -EINVAL;
  struct sem *s = get_item(dict_sems, n_sem);
  if(s->counter <= 0){
    update_process_state_rr(current(), &(s->q_blocked));
    sched_next_rr();
    return current()->sem_destroyed? -1: 0;
  }else{
    --s->counter;
    return 0;
  }
}

int sys_sem_signal(int n_sem){
  if(!contains(dict_sems, n_sem))return -EINVAL;
  struct sem *s = get_item(dict_sems, n_sem);
  if(!list_empty(&s->q_blocked)){
    update_process_state_rr(list_head_to_task_struct(list_first(&(s->q_blocked))), &readyqueue);
  }else{
    ++s->counter;
  }
  return 1;
}

int sys_sem_destroy(int n_sem){
  if(!contains(dict_sems, n_sem))return -EINVAL;
  struct sem *s = get_item(dict_sems, n_sem);
  if (s->owner != current()->PID) return -1;

  del_item(dict_sems, n_sem);

  while(!list_empty(&(s->q_blocked))){
    struct task_struct *ts = list_head_to_task_struct(list_first(&s->q_blocked));
    ts->sem_destroyed = 1;
    update_process_state_rr(ts, &readyqueue);
  }
  list_add_tail(s, &free_sems);
  
  return 1;
}



int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;
  uchild->task.sem_destroyed=0;
  uchild->task.len_chars_read=0;
  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  // int index  = ((int) get_ebp() - (int) current())/sizeof(int);
  // uchild->stack[index] =(int) ret_from_fork;


  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}


#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  struct list_head *begin = &(dict_sems->info.root);
  struct list_head *lh = begin->next;
  while(lh != begin){
    dict_item *i = (dict_item *) lh;
    struct sem *s = (struct sem *) i->value;
    lh = lh->next;
    if(s->owner == current()->PID){
      sys_sem_destroy(i->key);
    }
  }

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  int index = get_DIR_index(current());
  --dir_pages_n_refs[index];
  if (dir_pages_n_refs[index] == 0){
    for (i=0; i<NUM_PAG_DATA; i++)
    {
      free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
      del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
    }
  }
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i < NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}
