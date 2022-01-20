<b>XIS</b> operating system needs only 2MB of memory to run.<br>

It supports boot loader, kernel with protected mode, paging, E820 enumeration, interrupt and exception handling, system calls, user mode, local APIC, ACPI and multiprocessor.

<b>Instructions for building kernel -</b><br>
cat krnlconst.hdr | awk 'BEGIN{print "#ifndef _KERNEL_CONST_H"}; { if(substr($1, 0, 1) != "#") {print "#define " $1 " " $2}; } END{print "#endif";}' > krnlconst.h<br>
cat krnlconst.hdr | awk '{ if(substr($1, 0, 1) != "#") {print ".equ " $1 ", " $2}; }' > krnlconst.S<br>

as --32 um.S -o um.o<br>
ld --static -T um.ld -m elf_i386 -nostdlib --nmagic -o um.elf um.o<br>
objcopy -O binary um.elf um.bin<br>

as --32 mpinit.S -o mpinit.o<br>
ld -static -T mpinit.ld -m elf_i386 -nostdlib --nmagic -o mpinit.elf mpinit.o<br>
objcopy -O binary mpinit.elf mpinit.bin<br>

as --32 boot32.S -o boot32.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c util.c -o util.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c system.c -o system.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c apic.c -o apic.o<br>
as --32 handlr32.S -o handlr32.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c smp.c -o smp.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c setup32.c -o setup32.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c start.c -o start.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c memory.c -o memory.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c page32.c -o page32.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c usermode.c -o usermode.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c lock.c -o lock.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c debug.c -o debug.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c acpi.c -o acpi.o<br>
gcc -m32 -std=gnu99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding -fno-pic -Wall -Wextra -Werror -c interrupt.c -o interrupt.o<br>

ld -static -T kernel32.ld -m elf_i386 -nostdlib --nmagic boot32.o util.o system.o apic.o smp.o setup32.o handlr32.o memory.o page32.o usermode.o lock.o acpi.o debug.o interrupt.o start.o -o xiskernel.elf<br>

objdump -x xiskernel.elf | grep _end_kernel_initial_pg_table | awk -Wposix '{cmd="printf %d 0x" $1; cmd | getline decimal; close(cmd); if (decimal > 4294963200) print "STOP: Not enough memory to map pagetables. Upgrade to 64bit kernel.";}'<br>
objcopy -O binary xiskernel.elf xiskernel.bin<br>
echo ".equ KERNEL_SIZE, `ls -l xiskernel.bin | cut -f5 -d\ `" > krnlsize.S<br>
awk '/KERNEL_SIZE/{var=$3; cmd="objdump -x xiskernel.elf | grep INITIAL_KMAPPED_MEMORY | cut -f1 -d\\ "; cmd | getline hex;close(cmd); cmd="printf %d 0x" hex; cmd | getline decimal; if ((var-4096) > decimal) printf("STOP: Map atleast 0x%x bytes in boot32.S. Currently mapped 0x%x bytes", var, decimal);}' krnlsize.S<br>

<b>Instructions for building boot loader -</b><br>
cat krnlconst.hdr | awk '{ if(substr($1, 0, 1) != "#") {print ".equ " $1 ", " $2}; }' > krnlconst.S<br>
echo ".equ KERNEL_SIZE, &#96;ls -l xiskernel.bin | cut -f5 -d\ &#96;" > krnlsize.h<br>
awk '/KERNEL_SIZE/{var=$3; cmd="objdump -x xiskernel.elf | grep INITIAL_KMAPPED_MEMORY | cut -f1 -d\\ "; cmd | getline hex;close(cmd); cmd="printf %d 0x" hex; cmd | getline decimal; if ((var-4096) > decimal) printf("STOP: Map atleast 0x%x bytes in boot32.S. Currently mapped 0x%x bytes", var, decimal);}' krnlsize.S<br>

as --32 bootldr.S -o bootldr.o<br>
ld -static -T bootldr.ld -m elf_i386 -nostdlib --nmagic -o bootldr.elf bootldr.o<br>
objcopy -O binary bootldr.elf bootldr.bin<br>

<b>Creating QEMU image -</b><br>
dd if=bootldr.bin of=hda.raw<br>
dd if=xiskernel.bin of=hda.raw seek=1<br>
qemu-img convert -O qcow2 hda.raw hda.qcow2<br>

<b>Running using QEMU -</b><br>
sudo qemu-system-i386 -smp 2 -hda hda.qcow2 -m 2 --enable-kvm -cpu host<br>

<b>Contact -</b>

shoily@gmail.com
