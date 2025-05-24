rm iso_dir/boot/vmbsd
cp kernel/vmbsd iso_dir/boot/
xorriso -as mkisofs --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image iso_dir -o qemu/hojuix.iso
