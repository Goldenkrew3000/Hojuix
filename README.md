# 호주ix - A 64bit hobbyist OSDev project

# NOTICE
Paging is too complicated, and I have given up on it for now. OSDev continues, just not paging/vmm...

# Notes
- Libk is currently built separate from the 64 bit kernel, but does not have to be moved
- Probably going to port the build system to CMake in the future

# Done
- GDT
- IDT
- Keyboard (Functional but needs more work)
- Framebuffer (Scrolling needs improving, currently just using memcpy)
- PIT Timer (@ 100hz)
- Physical Memory Manager

# Todo
- Virtual Memory Management / Paging
- Serial Driver
- Completed Keyboard Driver
- Mouse Driver (Not planning to do anything with it, but it's there if I want to)

# Personal Resources
## C UINT Sizes
https://educoder.tistory.com/entry/cc-int8t-uint8t-int16t-uint16t

## Limine Spec
https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md

## Great list of required things
https://github.com/149segolte/x86_64-kernel
