#!/bin/bash
sudo losetup /dev/loop18 hd.img
sudo kpartx -av /dev/loop18
sudo mount /dev/dm-0 /mnt -o uid=$USER,gid=$USER
# cp ../src/user/init.bin /mnt
# cp ../src/user/system_api_lib /mnt
# sudo umount /mnt