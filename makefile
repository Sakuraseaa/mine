SRC_DIR= ./src
BUI_DIR= ./build/kernel
BOCHS = ./bochs
INC_DIR = ./src/include
SCI_DIR = ./script
TET_DIR = ./test

PIC   := PIC
ASM   = nasm
CC	  = gcc

CFLAGS = -O0 -m64 -mcmodel=large -fno-common -std=gnu99 -nostartfiles -fno-stack-protector -Wall \
		-fno-builtin -fno-pie -fno-pic -nostdlib \
		-c -g -I $(INC_DIR)/ -I $(INC_DIR)/drivers/ -I $(INC_DIR)/lib/ -I $(INC_DIR)/base/ \
		-I $(INC_DIR)/fs/ -I $(SRC_DIR)/fs/FAT32/ -I $(INC_DIR)/usr/ -I $(SRC_DIR)/fs/minix/	\
		-I $(SRC_DIR)/mm/ \
#被链接的文件
OBJS = $(BUI_DIR)/head.o $(BUI_DIR)/entry.o $(BUI_DIR)/main.o $(BUI_DIR)/printk.o \
		$(BUI_DIR)/trap.o $(BUI_DIR)/memory.o $(BUI_DIR)/interrupt.o $(BUI_DIR)/PIC.o \
		$(BUI_DIR)/task.o $(BUI_DIR)/cpu.o $(BUI_DIR)/keyboard.o $(BUI_DIR)/mouse.o \
		$(BUI_DIR)/disk.o  $(BUI_DIR)/time.o $(BUI_DIR)/HEPT.o $(BUI_DIR)/softirq.o \
		$(BUI_DIR)/timer.o $(BUI_DIR)/schedule.o $(BUI_DIR)/semaphore.o $(BUI_DIR)/lib.o \
		$(BUI_DIR)/spinlock.o $(BUI_DIR)/fat32.o $(BUI_DIR)/syscalls.o $(BUI_DIR)/sys.o\
		$(BUI_DIR)/VFS.o  $(BUI_DIR)/waitqueue.o   $(BUI_DIR)/switch_to.o $(BUI_DIR)/serial.o\
		$(BUI_DIR)/SMP.o $(BUI_DIR)/signal.o  $(BUI_DIR)/execv.o $(BUI_DIR)/debug.o $(BUI_DIR)/buffer.o\
		$(BUI_DIR)/bitmap.o $(BUI_DIR)/minix.o  $(BUI_DIR)/device.o $(BUI_DIR)/super.o $(BUI_DIR)/inode.o\
		$(BUI_DIR)/usr_printf.o $(BUI_DIR)/usr_init.o  $(BUI_DIR)/usr_LIB.o $(BUI_DIR)/usr_lib.o \
		$(BUI_DIR)/mem_test.o $(BUI_DIR)/memmgrinit.o $(BUI_DIR)/msadsc.o $(BUI_DIR)/memarea.o 	\


OBJS_SCRIPT = $(BUI_DIR)/kallsyms.o
		
ALL:$(BUI_DIR)/Kernel.bin 

$(BUI_DIR)/Kernel.bin:$(OBJS) $(OBJS_SCRIPT)
	@ld -static -b elf64-x86-64  -z muldefs -o $@ $^ -T  ./Kernel.lds  -Map $(BUI_DIR)/../Kernel.map
	@sudo mount -o loop $(BOCHS)/boot.img /mnt
	@sudo cp $(BUI_DIR)/Kernel.bin /mnt -v
	@sudo umount /mnt
	@echo '感谢世界!系统编译构建完成！ ^_^'

$(BUI_DIR)/kernel.bin:$(OBJS)
	@ld -static -b elf64-x86-64  -z muldefs -o $@ $^ -T ./Kernel.lds -Map $(BUI_DIR)/../kernel.map

$(BUI_DIR)/main.o: $(SRC_DIR)/kernel/main.c
#	$(CC) $(CFLAGS) $< -o $@
	@$(CC) $(CFLAGS) $< -o $@ -D$(PIC)

$(BUI_DIR)/head.o : $(SRC_DIR)/kernel/head.S
	@$(CC) $(CFLAGS) $< -o $@
#	gcc -E head.S > head.s
# as --64 -g -o $@ head.s

$(BUI_DIR)/printk.o: $(SRC_DIR)/lib/printk.c 
	@$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/signal.o: $(SRC_DIR)/lib/signal.c 
	@$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/entry.o : $(SRC_DIR)/kernel/entry.S
	@$(CC) $(CFLAGS) $< -o $@
#	gcc -E  entry.S > entry.s
#	as --64 -g -o $@ entry.s

$(BUI_DIR)/trap.o: $(SRC_DIR)/kernel/trap.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/memory.o: $(SRC_DIR)/kernel/memory.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/interrupt.o: $(SRC_DIR)/kernel/interrupt.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/switch_to.o: $(SRC_DIR)/kernel/switch_to.S
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/task.o: $(SRC_DIR)/kernel/task.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/cpu.o: $(SRC_DIR)/drivers/cpu.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/SMP.o: $(SRC_DIR)/drivers/SMP.c
	@$(CC) $(CFLAGS) $< -o $@
ifeq ($(PIC), APIC)
$(BUI_DIR)/PIC.o: $(SRC_DIR)/drivers/APIC.c
	@$(CC) $(CFLAGS) $< -o $@
else
$(BUI_DIR)/PIC.o: $(SRC_DIR)/drivers/8259A.c
	@$(CC) $(CFLAGS) $< -o $@
endif
$(BUI_DIR)/keyboard.o: $(SRC_DIR)/drivers/keyboard.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/mouse.o: $(SRC_DIR)/drivers/mouse.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/disk.o: $(SRC_DIR)/drivers/disk.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/device.o: $(SRC_DIR)/drivers/device.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/serial.o: $(SRC_DIR)/drivers/serial.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/time.o: $(SRC_DIR)/lib/time.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/HEPT.o: $(SRC_DIR)/drivers/HEPT.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/softirq.o: $(SRC_DIR)/lib/softirq.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/timer.o: $(SRC_DIR)/lib/timer.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/schedule.o: $(SRC_DIR)/kernel/schedule.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/semaphore.o: $(SRC_DIR)/lib/semaphore.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/lib.o: $(SRC_DIR)/lib/lib.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/debug.o: $(SRC_DIR)/lib/debug.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/spinlock.o: $(SRC_DIR)/lib/spinlock.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/bitmap.o: $(SRC_DIR)/lib/bitmap.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/fat32.o: $(SRC_DIR)/fs/FAT32/fat32.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/minix.o: $(SRC_DIR)/fs/minix/minix.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/syscalls.o: $(SRC_DIR)/kernel/syscalls.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/sys.o: $(SRC_DIR)/kernel/sys.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/VFS.o: $(SRC_DIR)/fs/VFS.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/buffer.o: $(SRC_DIR)/fs/buffer.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/super.o: $(SRC_DIR)/fs/super.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/inode.o: $(SRC_DIR)/fs/inode.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/waitqueue.o: $(SRC_DIR)/lib/waitqueue.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/execv.o: $(SRC_DIR)/lib/execv.c
	@$(CC) $(CFLAGS) $< -o $@

# =========== try mm =============
$(BUI_DIR)/msadsc.o: $(SRC_DIR)/mm/msadsc.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/memarea.o: $(SRC_DIR)/mm/memarea.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/memmgrinit.o: $(SRC_DIR)/mm/memmgrinit.c
	@$(CC) $(CFLAGS) $< -o $@
# ================================

# =============  SCRIPT ============
$(BUI_DIR)/kallsyms.o: $(SCI_DIR)/kallsyms.c $(BUI_DIR)/kernel.bin
	@$(CC) -o $(BUI_DIR)/kallsyms $(SCI_DIR)/kallsyms.c
	@nm -n $(BUI_DIR)/kernel.bin | $(BUI_DIR)/kallsyms > $(SCI_DIR)/kallsyms.S
	@$(CC) -c -o $(BUI_DIR)/kallsyms.o $(SCI_DIR)/kallsyms.S
# ============= SCRIPT ============


# ============= DEBUGE ============
$(BUI_DIR)/usr_init.o: $(SRC_DIR)/kernel/usr_init.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_lib.o: $(SRC_DIR)/kernel/usr_lib.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_printf.o: $(SRC_DIR)/kernel/usr_printf.c
	@$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_LIB.o: $(SRC_DIR)/kernel/usr_LIB.c
	@$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/mem_test.o: $(SRC_DIR)/../test/mem_test.c
	@$(CC) $(CFLAGS) $< -o $@

a ?= 0
n ?= 0
# ============= DEBUGE ============
.PHONY: clean a2l hex
clean:
	@rm -rf $(BUI_DIR)/*
a2l:
	addr2line -e $(BUI_DIR)/Kernel.bin $(a)

hex:
	@python3 $(TET_DIR)/hex.py $(n)