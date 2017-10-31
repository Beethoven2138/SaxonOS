GCCPARAMS = -std=gnu99 -ffreestanding  -static-libgcc -lgcc -Iinclude

objects = obj/boot.o \
	obj/kernel.o \
	obj/gdt.o \
	obj/idt.o \
	obj/irq.o \
	obj/common.o \
	obj/asm/idt.o \
	obj/asm/gdt.o \
	obj/asm/tss.o \
	obj/drivers/keyboard.o \
	obj/drivers/timer.o \
	obj/mm/frame.o \
	obj/mm/paging.o\
	obj/mm/enable_paging.o \
	obj/mm/heap.o \
	obj/drivers/pci.o \
	obj/switch.o \
	obj/task.o \
	obj/drivers/ata.o \
	obj/fs/ext2/ext2.o \
	obj/fs/mbr.o \
	obj/tss.o \
	obj/userspace/syscall.o \
	obj/userspace/sys_read.o \
	obj/userspace/sys_write.o \
	obj/userspace/sys_time.o \
	obj/fs/vfs/vfs.o \
	obj/fs/devfs/buffer.o \
	obj/fs/devfs/devfs.o \
	obj/drivers/rtc.o \

run: os.iso

obj/%.o: src/%.c
	mkdir -p $(@D)
	/opt/cross/bin/i686-elf-gcc $(GCCPARAMS) -c -o $@ $<

obj/%.o: src/%.asm
	mkdir -p $(@D)
	nasm -felf32 -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	/opt/cross/bin/i686-elf-gcc $(GCCPARAMS) -c -o $@ $<

os.bin: linker.ld $(objects)
	/opt/cross/bin/i686-elf-gcc -T linker.ld -o os.bin -ffreestanding -O2 -nostdlib $(objects)

os.iso: os.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp    os.bin iso/boot/os.bin
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "SaxonOS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/os.bin'    >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=SaxonOS.iso iso
	rm -rf iso

clean:
	rm -rf obj *.bin
