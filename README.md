# 호주ix - A 64bit hobbyist OSDev project

# Sub-Projects
- OS - Hojuix for x86_64 (The main project)
- PiOS - Experiments with OSDev on AArch64 (Specifically for Raspberry Pi 3/4, only used for referencing)
- RiscV - My personal exploration into RiscV (RV64, abandoned)

# Notes
- Libk is currently built separate from the 64 bit kernel, but does not have to be moved
- Going to port the Build System to CMake in the future

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
- Port a shell (Probably SH)
- VFS
- Allow the VMM to free memory (Can only allocate now lmao)

# Funny things Todo
- (Funny but going to do it): Native linux kexec support in terminal (Treat a vmlinuz file as an executable)

# Personal Resources
## C UINT Sizes
https://educoder.tistory.com/entry/cc-int8t-uint8t-int16t-uint16t

## Limine Spec
https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md

## Great list of required things
https://github.com/149segolte/x86_64-kernel
