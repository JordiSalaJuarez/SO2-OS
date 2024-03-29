/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */

#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>
#include <errno.h>

int write(int fd, char *buffer, int size);

int gettime();

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

static const char EBADF_msg[] = "Bad file descriptor identifier.";
static const char EFAULT_msg[] = "Buf is outside your accessible address space.";
static const char EINVAL_msg[] = "Buffer size contains an invalid value.";
static const char ENOSYS_msg[] = "System call not defined";

void perror(void);

#endif  /* __LIBC_H__ */
