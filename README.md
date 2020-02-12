XIS kernel needs only 2MB of memory to run.

Instructions for compiling -

as --32 bootldr.S -o bootldr.o
as --32 boot32.S -o boot32.o
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c start.c -o start.o
ld -static -T bootldr.ld -m elf_i386 -nostdlib --nmagic -o bootldr.elf bootldr.o
ld -static -T boot32.ld -m elf_i386 -nostdlib --nmagic boot32.o start.o -o boot32.elf


Creating image -

objcopy -O binary bootldr.elf bootldr.bin
objcopy -O binary boot32.elf boot32.bin
dd if=bootldr.bin of=hda.raw
dd if=boot32.bin of=hda.raw seek=1
qemu-img convert -O qcow2 hda.raw hda.qcow2


Running using QEMU -

sudo qemu-system-i386 -hda hda.qcow2 -m 2 --enable-kvm -cpu host

