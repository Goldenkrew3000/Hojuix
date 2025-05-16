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
- Virtual Memory Manager (4 Level Paging)
- Basic RS232 Driver - WIP (Works on real hardware... sometimes)
- Usermode (although more work required)
- AHCI Driver

# WIP
- Fix new interrupt handler (Assembly based to handle interrupt driven syscalls)
- Add new masking method for PS/2 Keyboard and PIT
- FAT16 support
- NVMe Support
- HDA Support
- ELF Parser
- Allow the VMM to free memory

# Todo
- Completed Keyboard Driver
- PS/2 Mouse Driver (Not planning to do anything with it currently, but want support)
- Scheduling
- 'Port' a shell (Probably XV6's SH)
- VFS
- (MAYBE) Port build system to CMake
- Drop ATA PIO support

# Funny list of things todo
- Native linux kexec support in terminal (Treat a vmlinuz file as an executable)

# List of bugs/issues to fix
- Clean up the PMMGR (And fix the slow allocation speed on 12th gen Intel)
- Rewrite the VMMGR
- Add more verbose errors to the VMMGR
- Add a way to free physical memory from the VMMGR (Traverse the TLB)
- Add a way for the VMMGR to allocate memory and send a random address back in a range
- Properly implement the new interrupt handler
- Switch syscall handler from interrupt based to 'syscall' instruction way
- Actually add a proper list of syscalls instead of whatever the hell I have now (Base it off of 4.4BSD maybe)
- Move assembler functions to the correct files in the ```i386``` directory
- Add a way to the new interrupt handler to allow interrupt devices (PS/2 Keyboard, etc)
