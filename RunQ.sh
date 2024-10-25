#! /bin/bash
qemu-system-x86_64 -vga std -hdc /home/steven/mine/bochs/hd.img -fda /home/steven/mine/bochs/boot.img -m 512M -boot a -cpu Broadwell-v1 -chardev stdio,mux=on,id=com1 -serial chardev:com1
