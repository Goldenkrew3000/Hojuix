#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <stdint.h>

/*
// Syscall List
*/
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

typedef struct {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t isrNumber;
	uint64_t errorCode;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} __attribute__((packed)) syscall_regs_t;

void syscall_handler(syscall_regs_t* r);
int syscall_print(char* c);
void syscall_exit(int rc);

void syscall_open();
void syscall_exec(char* rdi);

#endif
