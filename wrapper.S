#include <asm.h>
#include <segment.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/


#define SYSCALL(syscallID) \
	leal return##syscallID, %eax; \
	push %eax; \
	movl $syscallID, %eax; \
	push %ebp; \
	movl %esp, %ebp; \
	sysenter; \
	return##syscallID: pop %ebp; \
	addl $4, %esp;

#define UPDATE_ERRNO \
      cmpl $0, %eax; \
      jge 1f; \
      negl %eax; \
      leal errno, %ebx; \
      movl %eax, (%ebx); \
      movl $-1, %eax; \
      1:

ENTRY(write)
      pushl %ebp
      movl %esp, %ebp

      pushl %ebx

      movl 8(%ebp), %ebx
      movl 12(%ebp), %ecx
      movl 16(%ebp), %edx
      SYSCALL(4)
/**   movl $4, %eax
      int $0x80**/
      UPDATE_ERRNO

      popl %ebx

      popl %ebp
      ret

ENTRY(gettime)
      pushl %ebp
      movl %esp, %ebp

      SYSCALL(10)
      UPDATE_ERRNO

      popl %ebp
      ret

ENTRY(fork)
      pushl %ebp
      movl %esp, %ebp

      SYSCALL(2)
      UPDATE_ERRNO

      popl %ebp
      ret

ENTRY(get_stats)
	push %ebp
	movl %esp, %ebp
	push %ebx
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	
      SYSCALL(35)
	UPDATE_ERRNO

	pop %ebx
	pop %ebp
	ret



ENTRY(exit)
      pushl %ebp
      movl %esp, %ebp

      SYSCALL(1)
      UPDATE_ERRNO

      popl %ebp
      ret




ENTRY(getpid)
      pushl %ebp
      movl %esp, %ebp

      SYSCALL(20)
      UPDATE_ERRNO

      popl %ebp
      ret
