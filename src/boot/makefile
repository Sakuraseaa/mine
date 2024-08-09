SRC_DIR=/home/steven/mine/src/kernel
BOT_DIR=/home/steven/mine/src
BUI_DIR=/home/steven/mine/build/kernel
BOCHS = /home/steven/mine/bochs

ASM   = nasm
CC	  = gcc


ALL: $(BUI_DIR)/boot.bin $(BUI_DIR)/LOADER.BIN 

$(BUI_DIR)/boot.bin:$(BOT_DIR)/boot/boot.asm 
	$(ASM)  -f bin -I $(BOT_DIR)/boot/include/ -o $@ $^
	dd if=$(BUI_DIR)/boot.bin of=$(BOCHS)/boot.img bs=512 count=1 conv=notrunc

$(BUI_DIR)/LOADER.BIN: $(BOT_DIR)/boot/loader.asm
	$(ASM)  -f bin -I $(BOT_DIR)/boot/include/ -o $@ $^
	sudo mount -o loop $(BOCHS)/boot.img /mnt/flo -t vfat
	sudo cp $(BUI_DIR)/LOADER.BIN /mnt/flo -v
	sudo umount /mnt/flo

.PHONY: clean
clean:
	rm -rf $(BUI_DIR)/*
#	rm -rf *.o *.s~ *.s *.S~ *.c~ *.h~ system  Makefile~ Kernel.lds~ kernel.bin