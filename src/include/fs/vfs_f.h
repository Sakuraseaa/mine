#ifndef _VFS_F_H_
#define _VFS_F_H_

struct super_block *mount_fs(char *name, struct Disk_Partition_Table_Entry *DPTE, void *buf);
unsigned long register_filesystem(struct file_system_type *fs);
unsigned long unregister_filesystem(struct file_system_type *fs);
struct dir_entry *path_walk(char *name, unsigned long flags, struct dir_entry **create_file);
long FS_lseek(struct file *filp, long offset, long origin);

void DISK1_FAT32_FS_init(void);
void VFS_init(void);

#endif // _VFS_F_H_