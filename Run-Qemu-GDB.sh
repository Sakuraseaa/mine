#! /bin/bash
# qemu-system-x86_64 -vga std -hdc /home/steven/mine/bochs/hd.img -fda /home/steven/mine/bochs/boot.img -m 512M -boot a -s -S -cpu Broadwell-v1 -chardev stdio,mux=on,id=com1 -serial chardev:com1

# Logfile
# timestamp=$(date +%Y%m%d_%H%M%S)
qemu-system-x86_64 \
    -vga std \
    -hdc /home/steven/mine/bochs/hd.img \
    -fda /home/steven/mine/bochs/boot.img \
    -m 512M \
    -boot a \
    -s -S \
    -cpu Broadwell-v1 \
    -chardev file,id=com1,path="/home/steven/mine/log/mine.log" \
    -serial chardev:com1
#    -chardev file,id=com1,path="/home/steven/mine/log/mine_${timestamp}.log" \

# Terminal
# qemu-system-x86_64 \
#     -vga std \
#     -hdc /home/steven/mine/bochs/hd.img \
#     -fda /home/steven/mine/bochs/boot.img \
#     -m 512M \
#     -boot a \
#     -s -S \
#     -cpu Broadwell-v1 \
#     -chardev stdio,id=serial0,mux=on \
#     -serial chardev:serial0 \
#     -mon chardev=serial0,mode=readline \



