/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <stats.h>
#include <libc.h>

int MAX_PID = 1;
int remaining_ticks;

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

void update_sched_data_rr (void){
	current()->stats.remaining_ticks = max(0, current()->stats.remaining_ticks - 1);
}

// void init_idle (void)
// {
// 	struct list_head *it_aux = list_first(&freequeue);
// 	list_del(it_aux);
// 	struct task_struct * it_ts = list_head_to_task_struct(it_aux);
// 	union task_union * it_tu = (union task_union *) it_ts;
// 	it_ts->PID = 0;
// 	it_ts->dir_pages_baseAddr = allocate_DIR(it_ts);
// 	it_tu->stack[KERNEL_STACK_SIZE-1] = (unsigned long) cpu_idle;
// 	it_tu->stack[KERNEL_STACK_SIZE-2] = (unsigned long) 0;
// 	it_ts->esp = &it_tu->stack[KERNEL_STACK_SIZE-2];
// 	idle_task = it_ts;
// }

int needs_sched_rr (void){
	return !list_empty(&readyqueue) && current()->stats.remaining_ticks == 0 ;	
}


// void init_task1(void)
// {
// 	struct list_head *it_aux = list_first(&freequeue);
// 	list_del(it_aux);
// 	struct task_struct * it_ts = list_head_to_task_struct(it_aux);
// 	union task_union * it_tu = (union task_union *) it_ts;
// 	it_ts->PID = MAX_PID++;
// 	it_ts->dir_pages_baseAddr = allocate_DIR(it_ts);
// 	set_user_pages(it_ts);

// 	tss.esp0 = KERNEL_ESP(it_tu);
// 	it_ts->esp = KERNEL_ESP(it_tu);
// 	writeMSR(0x175, KERNEL_ESP(it_tu));
// 	it_ts->ticks = 0;
// 	it_ts->quantum = 12;
// 	it_ts->state = ST_RUN;
// 	quantum_left = 12;
// 	set_cr3(it_ts->dir_pages_baseAddr);
// }


void init_idle (void)
{
	struct list_head *ff = list_first(&freequeue);
	list_del(ff);
	struct task_struct *idle_ts = list_head_to_task_struct(ff);
	idle_ts -> PID = 0;
	union task_union *tmp = (union task_union *) idle_ts;
	tmp->stack[KERNEL_STACK_SIZE-1] = (unsigned long) cpu_idle;
	tmp->stack[KERNEL_STACK_SIZE-2] = (unsigned long) 0;
	idle_ts -> esp = &tmp->stack[KERNEL_STACK_SIZE-2];
	idle_ts->dir_pages_baseAddr = allocate_DIR(idle_ts);
	idle_task = idle_ts;
}

void init_task1(void)
{
	struct list_head *ff = list_first(&freequeue);
	list_del(ff);
	
	struct task_struct *task1_ts = list_head_to_task_struct(ff);
	task1_ts -> PID = MAX_PID++;

	task1_ts->dir_pages_baseAddr = allocate_DIR(task1_ts);
	set_user_pages(task1_ts);

	tss.esp0 = (unsigned long) KERNEL_ESP((union task_union *) task1_ts);
	task1_ts -> esp = (unsigned long *) KERNEL_ESP((union task_union *) task1_ts);
	writeMsr(0x175, KERNEL_ESP((union task_union *) task1_ts)); 
	task1_ts -> ticks = 0;
	set_quantum(task1_ts, 12);
	task1_ts -> state = ST_RUN;
	set_cr3(task1_ts->dir_pages_baseAddr); 
}

void init_sched(){
}


void init_free_queue(){
	int i;
	INIT_LIST_HEAD(&freequeue);        
	for (i = 0; i < NR_TASKS; ++i){
		task[i].task.PID = -1;
		list_add_tail(&task[i].task.list, &freequeue);
	}
}

void init_ready_queue(){
	INIT_LIST_HEAD(&readyqueue);
}



void update_process_state_rr (struct task_struct *t, struct list_head *dst_queue){
	
	if( t->state != ST_RUN ) list_del(&t->list);
	
	if( dst_queue == NULL ){
		t->state = ST_RUN;
	}else
	{
		list_add_tail(&t->list, dst_queue);	
		t->state = ST_READY;
	} 
}

void sched_next_rr (void){
	if(!list_empty(&readyqueue)){
		struct list_head* next = list_first(&readyqueue);
		struct task_struct* ts = list_head_to_task_struct(next);
		update_process_state_rr(current(), &readyqueue);
		
		// Update current stats
		unsigned long delta = get_ticks() - current()->stats.elapsed_total_ticks;
		current()->stats.system_ticks += delta;
		current()->stats.remaining_ticks = max(0, current()->stats.remaining_ticks - delta );
		current()->stats.elapsed_total_ticks = get_ticks();

		// 

		++ts->stats.total_trans;
		ts->stats.ready_ticks += get_ticks() - ts->stats.elapsed_total_ticks;
		ts->stats.remaining_ticks = ts->quantum;
		ts->stats.elapsed_total_ticks = get_ticks();
		
		update_process_state_rr(ts, NULL);
		task_switch((union task_union * )ts);
	}else{
		// Update current stats
		current()->stats.system_ticks += get_ticks() - current()->stats.elapsed_total_ticks;
		current()->stats.elapsed_total_ticks = get_ticks();

		task_switch((union task_union * ) idle_task);
	}
}

void update_entry_system(){
	unsigned long delta = get_ticks() - current()->stats.elapsed_total_ticks;
	current()->stats.user_ticks += delta;
	current()->stats.remaining_ticks = max(0, current()->stats.remaining_ticks - delta );
	current()->stats.elapsed_total_ticks = get_ticks();
}

void update_leave_system(){
	unsigned long delta = get_ticks() - current()->stats.elapsed_total_ticks;
	current()->stats.system_ticks += delta;
	current()->stats.remaining_ticks = max(0, current()->stats.remaining_ticks - delta );
	current()->stats.elapsed_total_ticks = get_ticks();
}




int get_quantum (struct task_struct *t){
	return t->quantum;
}
void set_quantum (struct task_struct *t, int new_quantum){
	t->quantum = new_quantum;
	t->stats.remaining_ticks = new_quantum; 
}

// void init_sched()
// {
// 	INIT_LIST_HEAD(&freequeue);
// 	for(int i = 0; i < NR_TASKS; i++){
// 		task[i].task.PID = -1;
// 		list_add( &(task[i].task.list), &freequeue);
// 	}
// 	INIT_LIST_HEAD(&readyqueue);
// }

struct task_struct* current()
{
  int ret_value;

  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void inner_task_switch(union task_union*t){
	tss.esp0 =  KERNEL_ESP(t);
	writeMsr(0x175, (int) KERNEL_ESP(t));
	set_cr3(t -> task.dir_pages_baseAddr);
	current() -> esp = getEbp(); 
	setEsp(t -> task.esp);

	return;          
}      
