/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}

extern struct list_head blocked;

void writeMsr(int msr, long data);

void task_switch(union task_union *new);

unsigned long getEBP();

void setESP(unsigned long *val);

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t)
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t)
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


page_table_entry * allocate_DIR(struct task_struct *t)
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	return (page_table_entry*) &dir_pages[pos];
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	if(!list_empty(&free_queue))
	{
		struct list_head *it_aux = list_first(&free_queue);
		struct task_struct * it_ts = list_head_to_task_struct(it_aux);
		union task_union * it_tu = (union task_union *) it_ts;
		it_ts->PID = 0;
		it_ts->dir_pages_baseAddr = allocate_DIR(it_ts);
		it_tu->stack[KERNEL_STACK_SIZE-1] = (unsigned long) cpu_idle;
		it_tu->stack[KERNEL_STACK_SIZE-2] = (unsigned long) 0;
		it_ts->esp = &it_tu->stack[KERNEL_STACK_SIZE-2];
		idle_task = it_ts;
		list_del(it_aux);
	}
}

void init_task1(void)
{
	if(!list_empty(&free_queue))
	{
		struct list_head *it_aux = list_first(&free_queue);
		struct task_struct * it_ts = list_head_to_task_struct(it_aux);
		union task_union * it_tu = (union task_union *) it_ts;
		it_ts->PID = 1;
		it_ts->dir_pages_baseAddr = allocate_DIR(it_ts);
		set_user_pages(it_ts);
		tss.esp0 = KERNEL_ESP(it_tu);
		writeMSR(0x175, KERNEL_ESP(it_tu));
		set_cr3(it_ts->dir_pages_baseAddr);
		it_ts->esp = &it_tu->stack[KERNEL_STACK_SIZE-3];
	}
}

void init_sched()
{
	INIT_LIST_HEAD(&free_queue);
	for(int i = 0; i < NR_TASKS; i++)
		list_add( &(task[i].task.list), &free_queue);

	INIT_LIST_HEAD(&ready_queue);
}

struct task_struct* current()
{
  int ret_value;

  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void inner_task_switch(union task_union *new)
{
	tss.esp0 = KERNEL_ESP(new);
	writeMSR(0x175, KERNEL_ESP(new));
//	if(current()->dir_pages_baseAddr == new->task.dir_pages_baseAddr)
		set_cr3(new->task.dir_pages_baseAddr);
	current()->esp = getEBP();
	setESP(new->task.esp);
	return;
}
