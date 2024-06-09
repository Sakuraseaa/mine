SRC_DIR=/home/steven/mine/src
BUI_DIR=/home/steven/mine/src/Build
BOCHS = /home/steven/mine/bochs
INC_DIR = /home/steven/mine/src/include

############# test ########
USR_SRC_DIR=/home/steven/mine/src/user
USR_BUI_DIR=/home/steven/mine/src/user/Build
USR_INC_DIR = /home/steven/mine/src/user/include
##########################
PIC   := PIC
ASM   = nasm
CC	  = gcc

CFLAGS = -O0 -m64 -mcmodel=large -fno-stack-protector -fno-common -std=gnu89 -nostartfiles \
		-fno-builtin -fno-pie -fno-pic  -Wno-address-of-packed-memeber \
		-c -g -I $(INC_DIR)/ -I $(INC_DIR)/drivers/ -I $(INC_DIR)/lib/ -I $(INC_DIR)/base/ \
		-I $(SRC_DIR)/fs/ -I $(SRC_DIR)/fs/FAT32/ -I $(INC_DIR)/usr/ 
#被链接的文件
OBJS = $(BUI_DIR)/head.o $(BUI_DIR)/entry.o $(BUI_DIR)/main.o $(BUI_DIR)/printk.o \
		$(BUI_DIR)/trap.o $(BUI_DIR)/memory.o $(BUI_DIR)/interrupt.o $(BUI_DIR)/PIC.o \
		$(BUI_DIR)/task.o $(BUI_DIR)/cpu.o $(BUI_DIR)/keyboard.o $(BUI_DIR)/mouse.o \
		$(BUI_DIR)/disk.o  $(BUI_DIR)/time.o $(BUI_DIR)/HEPT.o $(BUI_DIR)/softirq.o \
		$(BUI_DIR)/timer.o $(BUI_DIR)/schedule.o $(BUI_DIR)/semaphore.o $(BUI_DIR)/lib.o \
		$(BUI_DIR)/spinlock.o $(BUI_DIR)/fat32.o $(BUI_DIR)/syscalls.o $(BUI_DIR)/sys.o\
		$(BUI_DIR)/VFS.o  $(BUI_DIR)/waitqueue.o  $(BUI_DIR)/usr_lib.o  $(BUI_DIR)/switch_to.o\
		$(BUI_DIR)/usr_printf.o $(BUI_DIR)/init.o $(BUI_DIR)/usr_dirent.o $(BUI_DIR)/usr_wait.o \

		
ALL:$(BUI_DIR)/kernel.bin


$(BUI_DIR)/kernel.bin:$(OBJS)
	ld -static -b elf64-x86-64  -z muldefs -o $@ $^ -T Kernel.lds  -Map $(BUI_DIR)/kernel.map
	sudo mount -o loop $(BOCHS)/boot.img /mnt
	sudo cp $(BUI_DIR)/kernel.bin /mnt -v
	sudo umount /mnt
	@echo '感谢世界!系统编译构建完成！ ^_^'

$(BUI_DIR)/main.o: $(SRC_DIR)/kernel/main.c
#	$(CC) $(CFLAGS) $< -o $@
	$(CC) $(CFLAGS) $< -o $@ -D$(PIC)

$(BUI_DIR)/head.o : $(SRC_DIR)/kernel/head.S
	$(CC) $(CFLAGS) $< -o $@
#	gcc -E head.S > head.s
# as --64 -g -o $@ head.s

$(BUI_DIR)/printk.o: $(SRC_DIR)/lib/printk.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/entry.o : $(SRC_DIR)/kernel/entry.S
	$(CC) $(CFLAGS) $< -o $@
#	gcc -E  entry.S > entry.s
#	as --64 -g -o $@ entry.s

$(BUI_DIR)/trap.o: $(SRC_DIR)/kernel/trap.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/memory.o: $(SRC_DIR)/kernel/memory.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/interrupt.o: $(SRC_DIR)/kernel/interrupt.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/switch_to.o: $(SRC_DIR)/kernel/switch_to.S
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/task.o: $(SRC_DIR)/kernel/task.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/cpu.o: $(SRC_DIR)/drivers/cpu.c
	$(CC) $(CFLAGS) $< -o $@
ifeq ($(PIC), APIC)
$(BUI_DIR)/PIC.o: $(SRC_DIR)/drivers/APIC.c
	$(CC) $(CFLAGS) $< -o $@
else
$(BUI_DIR)/PIC.o: $(SRC_DIR)/drivers/8259A.c
	$(CC) $(CFLAGS) $< -o $@
endif
$(BUI_DIR)/keyboard.o: $(SRC_DIR)/drivers/keyboard.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/mouse.o: $(SRC_DIR)/drivers/mouse.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/disk.o: $(SRC_DIR)/drivers/disk.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/time.o: $(SRC_DIR)/kernel/time.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/HEPT.o: $(SRC_DIR)/drivers/HEPT.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/softirq.o: $(SRC_DIR)/kernel/softirq.c
	$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/timer.o: $(SRC_DIR)/lib/timer.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/SMP.o: $(SRC_DIR)/drivers/SMP.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/schedule.o: $(SRC_DIR)/kernel/schedule.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/semaphore.o: $(SRC_DIR)/lib/semaphore.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/lib.o: $(SRC_DIR)/lib/lib.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/spinlock.o: $(SRC_DIR)/lib/spinlock.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/fat32.o: $(SRC_DIR)/fs/FAT32/fat32.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/syscalls.o: $(SRC_DIR)/kernel/syscalls.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/sys.o: $(SRC_DIR)/kernel/sys.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/VFS.o: $(SRC_DIR)/fs/VFS.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/waitqueue.o: $(SRC_DIR)/lib/waitqueue.c
	$(CC) $(CFLAGS) $< -o $@
# ============= DEBUGE ============
$(BUI_DIR)/init.o: $(SRC_DIR)/kernel/init.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_lib.o: $(SRC_DIR)/kernel/usr_lib.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_printf.o: $(SRC_DIR)/kernel/usr_printf.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_dirent.o: $(SRC_DIR)/kernel/usr_dirent.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/usr_wait.o: $(SRC_DIR)/kernel/usr_wait.c
	$(CC) $(CFLAGS) $< -o $@
# ============= DEBUGE ============
.PHONY: clean
clean:
	rm -rf $(BUI_DIR)/*