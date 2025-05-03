#!/bin/bash
qemu-system-x86_64 \
    -vga std \
    -hdc /home/steven/mine/bochs/hd.img \
    -fda /home/steven/mine/bochs/boot.img \
    -m 512M \
    -boot a \
    -cpu Broadwell-v1 \
    -chardev stdio,id=serial0,mux=on \
    -serial chardev:serial0 \
    -mon chardev=serial0,mode=readline \


# 该命令执行的系统使用了x2apic功能, 使用前请进行正确的make编译
# 使用apic过程中，在-machine q35的条件下，使用IED硬盘中断，在qemu无法成功，
# 请学习PCIe,等知识后，再来学习
