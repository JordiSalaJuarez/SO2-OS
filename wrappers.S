
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

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %ebx;\
      popl %ecx;\
      popl %edx;\
      popl %esi;\
      popl %edi;\
      popl %ebp;\
      popl %eax;\
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs

#define CHECK_ERROR(end) \
      cmpl $0, %eax; \
      jge end; \
      negl %eax; \
      leal errno, %ebx; \
      movl %eax, (%ebx); \
      movl $-1, %eax;

ENTRY(write)
      push %ebp
      movl %esp, %ebp

      pushl   %ebx

      movl 8(%ebp), %ebx
      movl 12(%ebp), %ecx
      movl 16(%ebp), %edx
      movl $4, %eax
      int $0x80
      CHECK_ERROR(end.write)
end.write:
      popl %ebx
      popl %ebp
      ret


ENTRY(gettime)
      push %ebp
      movl %esp, %ebp

      pushl   %ebx

      movl $10, %eax
      int $0x80
      CHECK_ERROR(end.gettime)
end.gettime:
      popl %ebx
      popl %ebp
      ret
