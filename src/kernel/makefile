SRC_DIR=/home/steven/mine/src/kernel
BOT_DIR=/home/steven/mine/src
BUI_DIR=/home/steven/mine/src/Build
BOCHS = /home/steven/mine/bochs

PIC   := PIC
ASM   = nasm
CC	  = gcc

CFLAGS = -O0 -m64 -mcmodel=large -fno-stack-protector \
		-fno-builtin -fno-pie -fno-pic  \
		-c -g \

#被链接的文件
OBJS = $(BUI_DIR)/head.o $(BUI_DIR)/entry.o $(BUI_DIR)/main.o $(BUI_DIR)/printk.o \
		$(BUI_DIR)/trap.o $(BUI_DIR)/memory.o $(BUI_DIR)/interrupt.o $(BUI_DIR)/PIC.o \
		$(BUI_DIR)/task.o $(BUI_DIR)/cpu.o $(BUI_DIR)/keyboard.o $(BUI_DIR)/mouse.o \
		$(BUI_DIR)/disk.o  $(BUI_DIR)/time.o $(BUI_DIR)/HEPT.o $(BUI_DIR)/softirq.o \
		$(BUI_DIR)/timer.o $(BUI_DIR)/schedule.o $(BUI_DIR)/semaphore.o $(BUI_DIR)/lib.o \
		$(BUI_DIR)/spinlock.o $(BUI_DIR)/fat32.o $(BUI_DIR)/syscalls.o $(BUI_DIR)/sys.o\
		$(BUI_DIR)/VFS.o 
		
ALL:$(BUI_DIR)/kernel.bin


$(BUI_DIR)/kernel.bin:$(OBJS)
	ld -static -b elf64-x86-64  -z muldefs -o $@ $^ -T Kernel.lds  -Map $(BUI_DIR)/kernel.map
	sudo mount -o loop $(BOCHS)/boot.img /mnt/flo/
	sudo cp $(BUI_DIR)/kernel.bin /mnt/flo/ -v
	sudo umount /mnt/flo/
	@echo '感谢世界!系统编译构建完成！ ^_^'

$(BUI_DIR)/main.o: main.c
#	$(CC) $(CFLAGS) $< -o $@
	$(CC) $(CFLAGS) $< -o $@ -D$(PIC)

$(BUI_DIR)/head.o : head.S
	$(CC) $(CFLAGS) $< -o $@
#	gcc -E head.S > head.s
# as --64 -g -o $@ head.s

$(BUI_DIR)/printk.o: printk.c 
	$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/entry.o : entry.S
	$(CC) $(CFLAGS) $< -o $@
#	gcc -E  entry.S > entry.s
#	as --64 -g -o $@ entry.s

$(BUI_DIR)/trap.o: trap.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/memory.o: memory.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/interrupt.o: interrupt.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/task.o: task.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/cpu.o: cpu.c
	$(CC) $(CFLAGS) $< -o $@
ifeq ($(PIC), APIC)
$(BUI_DIR)/PIC.o: APIC.c
	$(CC) $(CFLAGS) $< -o $@
else
$(BUI_DIR)/PIC.o: 8259A.c
	$(CC) $(CFLAGS) $< -o $@
endif
$(BUI_DIR)/keyboard.o: keyboard.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/mouse.o: mouse.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/disk.o: disk.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/time.o: time.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/HEPT.o: HEPT.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/softirq.o: softirq.c
	$(CC) $(CFLAGS) $< -o $@

$(BUI_DIR)/timer.o: timer.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/SMP.o: SMP.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/schedule.o: schedule.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/semaphore.o: semaphore.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/lib.o: lib.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/spinlock.o: spinlock.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/fat32.o: fat32.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/syscalls.o: syscalls.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/sys.o: sys.c
	$(CC) $(CFLAGS) $< -o $@
$(BUI_DIR)/VFS.o: VFS.c
	$(CC) $(CFLAGS) $< -o $@
.PHONY: clean
clean:
	rm -rf $(BUI_DIR)/*