#ifndef _VFS_F_H_
#define _VFS_F_H_

spblk_t *mount_fs(str_t name, struct Disk_Partition_Table_Entry *DPTE, void *buf);
u64_t register_filesystem(struct file_system_type *fs);
u64_t unregister_filesystem(struct file_system_type *fs);
dir_entry_t *path_walk(str_t name, u64_t flags, dir_entry_t **create_file);
s64_t fs_lseek(file_t *filp, s64_t offset, s64_t origin);

void DISK1_FAT32_FS_init(void);
void VFS_init(void);

#endif // _VFS_F_H_