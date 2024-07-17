
CFLAGS := -O0 -m64 -mcmodel=large -fno-stack-protector -Wall -nostdlib\
		-fno-builtin -fno-pie -fno-pic

all: test.bin
	sudo mount /dev/dm-0 /mnt -o uid=$$USER,gid=$$USER
	sudo cp ./test.bin /mnt -v
	sudo umount /mnt

test.bin: test.o lib.o errno.o printf.o  
	ld -b elf64-x86-64 -z muldefs -o test.bin test.o lib.o errno.o printf.o

test.o: test.c
	gcc  $(CFLAGS) -c test.c

lib.o: lib.c
	gcc  $(CFLAGS) -c lib.c

errno.o: errno.c
	gcc  $(CFLAGS) -c errno.c

printf.o: printf.c
	gcc  $(CFLAGS) -c printf.c

clean:
	rm -rf *.o *.s~ *.s *.S~ *.c~ *.h~ Makefile~ User.lds~ system_api_lib test.bin

