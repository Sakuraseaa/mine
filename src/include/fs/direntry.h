#ifndef _DIRENTRY_H_
#define _DIRENTRY_H_

#define FS_ATTR_FILE (1UL << 0)            // 文件
#define FS_ATTR_DIR (1UL << 1)             // 目录
#define FS_ATTR_DEVICE_KEYBOARD (1UL << 2) // 设备文件 - 键盘

// 用于描述文件/目录在文件系统中的层级关系-目录项
typedef struct dir_entry
{
    char *name;      // 文件名
    int name_length; // 文件长度

    // 描述目录项之间的层级关系

    // child_node 来记录我是谁的子文件, 加入到父目录的列表中
    // suddires_list 来记录我的子文件都有谁
    struct List child_node;
    struct List subdirs_list; // subdirectory - 子目录

    struct index_node *dir_inode; // 本目录项描述的文件的inode
    struct dir_entry *parent;     // 父目录项

    struct dir_entry_operations *dir_ops; // 目录项操作方法：
    super_t*    d_sb; // 文件的超级块
}dir_entry_t;

#endif // _DIRENTRY_H_