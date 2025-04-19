# 호주ix - A 64bit hobbyist OSDev project

# NOTICE
In the state that this project is in, PLEASE DO NOT use this as a reference for your own operating system.<br>
It is a miracle that this codebase works at all.<br>
This notice will be removed ONLY when I am confident that this would be a good resource to others.

# Notes
- Libk is currently built separate from the 64 bit kernel, but does not have to be moved

# Done
- GDT / IDT / ISR
- Keyboard (Functional but needs more work)
- Framebuffer (Not double-buffered, yet)
- PIT Timer (@ 100hz)
- Physical Memory Manager
- Virtual Memory Manager (4 Level Paging) - WIP (Weird things happen with some addresses and not affecting the TLB)
- Basic RS232 Driver - WIP (Works on real hardware... sometimes)

# WIP
- Usermode! Works (Can execute ASM and C programs), but cannot return after finishing (no exit syscall) and syscalls are funky
- Fix new interrupt handler (Assembly based to handle interrupt driven syscalls)
- Add new masking method for PS/2 Keyboard and PIT
- Finish ATA PIO driver
- FAT16 support

# Todo
- Completed Keyboard Driver
- PS/2 Mouse Driver (Not planning to do anything with it currently, but want support)
- ELF Parser
- Scheduling
- 'Port' a shell (Probably XV6's SH)
- VFS
- Allow the VMM to free memory (Can only allocate now lmao)
- (MAYBE) Port build system to CMake

# Funny list of things todo
- Native linux kexec support in terminal (Treat a vmlinuz file as an executable)
