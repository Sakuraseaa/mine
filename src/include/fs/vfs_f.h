#ifndef _VFS_F_H_
#define _VFS_F_H_

spblk_t *mount_fs(char *name, struct Disk_Partition_Table_Entry *DPTE, void *buf);
u64_t register_filesystem(struct file_system_type *fs);
u64_t unregister_filesystem(struct file_system_type *fs);
dir_entry_t *path_walk(char *name, u64_t flags, dir_entry_t **create_file);
long FS_lseek(struct file *filp, long offset, long origin);

void DISK1_FAT32_FS_init(void);
void VFS_init(void);

#endif // _VFS_F_H_