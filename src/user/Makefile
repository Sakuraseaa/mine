#/***************************************************
#		版权声明
#
#	本操作系统名为：MINE
#	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
#	只允许个人学习以及公开交流使用
#
#	代码最终所有权及解释权归田宇所有；
#
#	本模块作者：	田宇
#	EMail:		345538255@qq.com
#
#
#***************************************************/
SRC_DIR=/home/steven/mine/src/user
BUI_DIR=/home/steven/mine/src/user/Build
INC_DIR = /home/steven/mine/src/user/include

CFLAGS = -O0 -m64 -mcmodel=large -fno-stack-protector \
		-fno-builtin -fno-pie -fno-pic  -I $(INC_DIR) \
		-c\

#被链接的文件
OBJS =    $(BUI_DIR)/init.o $(BUI_DIR)/lib.o $(BUI_DIR)/errno.o  \
		$(BUI_DIR)/malloc.o $(BUI_DIR)/keyboard.o $(BUI_DIR)/printf.o \
		$(BUI_DIR)/string.o

all: $(BUI_DIR)/system_api_lib
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $(BUI_DIR)/system_api_lib $(BUI_DIR)/init.bin
	sudo mount /dev/dm-0 /mnt -o uid=$$USER,gid=$$USER
	sudo cp $(BUI_DIR)/init.bin /mnt -v
	sudo cp $(BUI_DIR)/system_api_lib /mnt -v
	sudo umount /mnt

$(BUI_DIR)/system_api_lib: $(OBJS)
	ld -b elf64-x86-64 -z muldefs -o $@ $^  -T User.lds
$(BUI_DIR)/init.o: $(SRC_DIR)/init.c
	gcc  $(CFLAGS) $< -o $@
$(BUI_DIR)/lib.o: $(SRC_DIR)/lib.c
	gcc  $(CFLAGS)  $< -o $@
$(BUI_DIR)/errno.o: $(SRC_DIR)/errno.c
	gcc  $(CFLAGS)  $< -o $@
$(BUI_DIR)/malloc.o: $(SRC_DIR)/malloc.c
	gcc  $(CFLAGS)  $< -o $@
$(BUI_DIR)/printf.o: $(SRC_DIR)/printf.c
	gcc  $(CFLAGS)  $< -o $@
$(BUI_DIR)/keyboard.o: $(SRC_DIR)/keyboard.c
	gcc  $(CFLAGS)  $< -o $@
$(BUI_DIR)/string.o: $(SRC_DIR)/string.c
	gcc  $(CFLAGS)  $< -o $@
Hd_Path=/home/steven/mine/bochs/hd.img
.PHONY: clean update
clean:
	rm -rf $(BUI_DIR)/*
update:
	sudo losetup /dev/loop32  $(Hd_Path)
	sudo kpartx -av /dev/loop32