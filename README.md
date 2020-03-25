XIS kernel needs only 2MB of memory to run.

Instructions for compiling -

as --32 bootldr.S -o bootldr.o<br>
as --32 boot32.S -o boot32.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c util.c -o util.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c start.c -o start.o<br>
ld -static -T bootldr.ld -m elf_i386 -nostdlib --nmagic -o bootldr.elf bootldr.o<br>
ld -static -T kernel32.ld -m elf_i386 -nostdlib --nmagic boot32.o util.o start.o -o xiskernel.elf<br>


Creating image -

objcopy -O binary bootldr.elf bootldr.bin<br>
objcopy -O binary xiskernel.elf xiskernel.bin<br>
dd if=bootldr.bin of=hda.raw<br>
dd if=xiskernel.bin of=hda.raw seek=1<br>
qemu-img convert -O qcow2 hda.raw hda.qcow2<br>


Running using QEMU -

sudo qemu-system-i386 -hda hda.qcow2 -m 2 --enable-kvm -cpu host<br>

Contact -

shoily@gmail.com
