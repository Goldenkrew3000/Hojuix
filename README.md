# 호주ix - A hobbyist OSDev project

# Notes
- Libk is currently built separate from the 64 bit kernel, but does not have to be moved
- Probably going to port the build system to CMake in the future

# 64OS and 32OS
32OS is the legacy 32 bit kernel using GRUB Multiboot, whereas 64OS is a new 64 bit Higher Half kernel utilizing Limine. <br>
32OS is being combined into 64OS, and when that is complete, 32OS will be deleted. <br>

# Testing
The testing folder contains some stuff I was testing under linux, and am yet to put into the OS

# MUSL Reference 1.2.5
The libc implementation in this OS is probably going to be based off of MUSL, so it's there as reference

# The deal
Okay so due to some quirks with 32 bit and grub. I am porting to x86_64 with the Limine Bootloader <br>
like yeah copy much lmao but easier to deal with mm

# Personal Resources
## C UINT Sizes
https://educoder.tistory.com/entry/cc-int8t-uint8t-int16t-uint16t

## Limine Spec
https://github.com/limine-bootloader/limine/blob/v8.x/PROTOCOL.md

## Great list of required things
https://github.com/149segolte/x86_64-kernel
