rm iso_dir/boot/kernel.macho
cp kernel/kernel.macho iso_dir/boot/
xorriso -as mkisofs --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image iso_dir -o qemu/hojuix.iso
