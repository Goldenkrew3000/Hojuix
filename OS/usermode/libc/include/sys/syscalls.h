/*
// Hojuix LibC
// Syscall Interface
*/

#ifndef _HOJUIX_SYSCALLS_H
#define _HOJUIX_SYSCALLS_H

// Reading and writing
#define SYS_WRITE 1
#define SYS_READ 2

// Opening and closing
#define SYS_OPEN 3
#define SYS_CLOSE 4

// Process management
#define SYS_FORK 5
#define SYS_EXEC 6
#define SYS_EXIT 7
#define SYS_KILL 8
#define SYS_GETPID 9

// Device Management
#define SYS_IOCTL 10

// Memory management
#define SYS_MMAP 11
#define SYS_MUNMAP 12

// Kernel Management
#define SYS_REBOOT 13
#define SYS_SHUTDOWN 14

// Temporary Syscalls
#define SYS_PRINTF 101
#define SYS_KBD_INPUT 102

int libc_syscall(int rax, ...);

// Temporary interfaces to interface directly with the syscalls
char syscall_input();

#endif
