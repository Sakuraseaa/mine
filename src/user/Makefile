USR_SRC_DIR=/home/steven/mine/src/user
USR_BUI_DIR=/home/steven/mine/src/user/Build
USR_INC_DIR = /home/steven/mine/src/user/include

ASM   = nasm
CC	  = gcc

CFLAGS = -O0 -m64 -mcmodel=large -fno-stack-protector \
		-fno-builtin -fno-pie -fno-pic  -I $(USR_INC_DIR) \
		-c\

#被链接的文件
OBJS =    $(USR_BUI_DIR)/init.o $(USR_BUI_DIR)/lib.o $(USR_BUI_DIR)/errno.o  \
		$(USR_BUI_DIR)/malloc.o $(USR_BUI_DIR)/keyboard.o $(USR_BUI_DIR)/printf.o \
		$(USR_BUI_DIR)/string.o $(USR_BUI_DIR)/dirent.o $(USR_BUI_DIR)/wait.o \

all: $(USR_BUI_DIR)/system_api_lib
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $(USR_BUI_DIR)/system_api_lib $(USR_BUI_DIR)/init.bin
	sudo mount /dev/dm-0 /mnt -o uid=$$USER,gid=$$USER
	sudo cp $(USR_BUI_DIR)/init.bin /mnt -v
	sudo cp $(USR_BUI_DIR)/system_api_lib /mnt -v
	sudo umount /mnt

$(USR_BUI_DIR)/system_api_lib: $(OBJS)
	ld -b elf64-x86-64 -z muldefs -o $@ $^  -T User.lds
$(USR_BUI_DIR)/init.o: $(USR_SRC_DIR)/init.c
	gcc  $(CFLAGS) $< -o $@
$(USR_BUI_DIR)/lib.o: $(USR_SRC_DIR)/lib.c
	gcc  $(CFLAGS)  $< -o $@
$(USR_BUI_DIR)/errno.o: $(USR_SRC_DIR)/errno.c
	gcc  $(CFLAGS)  $< -o $@
$(USR_BUI_DIR)/malloc.o: $(USR_SRC_DIR)/malloc.c
	gcc  $(CFLAGS)  $< -o $@
$(USR_BUI_DIR)/printf.o: $(USR_SRC_DIR)/printf.c
	gcc  $(CFLAGS)  $< -o $@
$(USR_BUI_DIR)/keyboard.o: $(USR_SRC_DIR)/keyboard.c
	gcc  $(CFLAGS)  $< -o $@
$(USR_BUI_DIR)/string.o: $(USR_SRC_DIR)/string.c
	gcc  $(CFLAGS)  $< -o $@
$(USR_BUI_DIR)/dirent.o: $(USR_SRC_DIR)/dirent.c
	$(CC) $(CFLAGS) $< -o $@
$(USR_BUI_DIR)/wait.o: $(USR_SRC_DIR)/wait.c
	$(CC) $(CFLAGS) $< -o $@
Hd_Path=/home/steven/mine/bochs/hd.img
.PHONY: clean update
clean:
	rm -rf $(USR_BUI_DIR)/*
update:
	sudo losetup /dev/loop32  $(Hd_Path)
	sudo kpartx -av /dev/loop32