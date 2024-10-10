#include "toolkit.h"
#include "fskit.h"
#include "devkit.h"
#include "mmkit.h"
#include "kernelkit.h"
struct file_system_type filesystem = {"filesystem", 0};
#define MAX_FILE_NAME_LEN PAGE_4K_SIZE

spblk_t * sb_vec[4];
// 当前文件系统的超级块
spblk_t *current_sb = nullptr;
#if 0
// Slab_cache_t* Dir_Entry_Pool = nullptr;
static void* dir_entry_consturctor(void* Vaddr, u64 arg) { // 目录项构造函数。

    dir_entry_t* dir = Vaddr;
    memset(dir, 0, sizeof(dir_entry_t));

    list_init(&dir->child_node);
    list_init(&dir->subdirs_list);
    
    return (void*)dir;

}

static void* dir_entry_desturctor(void* Vaddr, u64 arg) { // 目录项析构函数。

    dir_entry_t* dir = Vaddr;
    if(dir->name_length)
        kdelete(dir->name, dir->name_length);
    
    list_del(&dir->child_node);
    list_del(&dir->subdirs_list);

    return (void*)dir;

}
#endif
void VFS_init(void) {
    // Dir_Entry_Pool = slab_create(sizeof(dir_entry_t), dir_entry_consturctor, dir_entry_desturctor, 0);
}

// 文件系统的注册
/**
 * @brief
 *
 * @param fs 文件系统类型元素
 * @return u64_t
 */
u64_t register_filesystem(struct file_system_type *fs)
{
    struct file_system_type *p = nullptr;
    // 阻止重复注册相同类型文件系统
    for (p = &filesystem; p; p = p->next)
        if (!strcmp(fs->name, p->name))
            return 0;
    // 链表的头插法 - 把fs文件类型加入文件类型链表
    fs->next = filesystem.next;
    filesystem.next = fs;
    return 1;
}

/**
 * @brief 文件系统的挂载函数- function mount_root, 挂载名为name的文件系统 == 建立文件系统对应的超级块结构体super block
 *        调用该文件系统专有的 read_superblock 函数
 * @param name 文件系统名
 * @param DPTE 描述文件系统的分区表项
 * @param buf  FAT32文件系统的引导扇区
 * @return spblk_t* （fat32）文件系统的超级块
 */
spblk_t *mount_fs(str_t name, struct Disk_Partition_Table_Entry *DPTE, void *buf)
{
    struct file_system_type *p = nullptr;
    spblk_t*  sb = nullptr;
    for (p = &filesystem; p; p = p->next)
        if (!strcmp(p->name, name)) {
            sb = p->read_superblock(DPTE, buf);
            list_add_to_behind(&super_list, &sb->node);
            break;
        }
    return sb;
}

// 文件系统的注销
u64_t unregister_filesystem(struct file_system_type *fs)
{
    struct file_system_type *p = &filesystem;

    while (p->next)
        if (p->next == fs)
        {
            p->next = p->next->next;
            fs->next = nullptr;
            return 1;
        }
        else
            p = p->next;
    return 0;
}

/**
 * @brief path_parse用于获得文件路径pathname的顶层路径, 顶层路径存入到name_store中
 *
 * @example pathname="/home/sk", char name_store[10]
 *          path_parse(pathname, name_store) -> return "/sk", *name_store="home"
 *
 * @param pathname 需要解析的文件路径
 * @param name_store 主调函数提供的缓冲区
 * @return char* 指向除顶层路径之外的子路径字符串的地址
 */
static str_t path_parse(str_t pathname, str_t name_store)
{
    // 根目录不需要解析, 跳过即可
    if (pathname[0] == '/')
        while (*(++pathname) == '/')
            ; // 跳过'//a', '///b'

    while (*pathname != '/' && *pathname != 0)
        *name_store++ = *pathname++;

    if (pathname[0] == 0) // pathname为空, 则表示路径已经结束了, 此时返回NULL
        return nullptr;

    return pathname;
}

/**
 * @brief path_depth_cnt用于返回路径pathname的深度. 注意, 所谓路径的深度是指到文件的深度, 例如: /a的深度是1, /a/b的深度
 *        是2, /a/b/c/d/e的深度是5
 *
 * @param pathname 需要判断深度的路径名
 * @return uint32_t 路径的深度
 */
static s32_t path_depth_cnt(str_t pathname)
{
    str_t p = pathname;
    char_t name[MAX_FILE_NAME_LEN];
    u32_t depth = 0;
    p = path_parse(p, name);
    while (name[0])
    {
        depth++;
        memset(name, 0, MAX_FILE_NAME_LEN);
        if (p)
            p = path_parse(p, name);
    }
    return depth;
}


/**
 * @brief 依据链表缓冲情况，在目录parent中寻找名称为path的文件。 
 *      PS: Linux中此处的查找使用了链表 + 哈希表 + 目录项状态 等标志完成了目录项的缓冲标志，emmm
 *          我先使用纯链表吧，然后先把 slab 融入到 目录项 和 inode 比较好
 * @param parent 父目录项
 * @param path 需要查找的文件名称
 * @return dir_entry* 返回path文件的目录项
 */
static dir_entry_t* find_dir_childern(dir_entry_t* parent, str_t path, u32_t pathLen) {
    list_t* head = &parent->subdirs_list;
    dir_entry_t* det = nullptr;
    for(list_t* node = head->next; node != head; node = node->next) {
        det = container_of(node, dir_entry_t, child_node);
        if((det->name_length == pathLen) && (strncmp(path, det->name, pathLen) == 0))
            return det;
    }
    return nullptr;
}


/**
 * @brief 搜索文件name。
 *
 * @param name 文件名称
 * @param flags 当形参flags = 1时, 表示创建文件.此时path_walk函数返回name's父目录的目录项，否则返回 name's目录项
 *              当形参flags = 2时, 表示只是获得文件的目录项 和 文件目录的目录项
 *  
 * @param create_file 只有在sys_open中创建文件的时候，该参数才有效。这是传出参数。其中记录新文件的目录项信息
 * @return dir_entry_t* 搜索失败返回NULL, dir_entry和dentry动态申请的内存，由上层调用者释放
 */
dir_entry_t *path_walk(str_t name, u64_t flags, dir_entry_t **create_file)
{
    str_t tmpname = nullptr;
    s32_t tmpnamelen = 0, nameDep = 0, Count = 0;
    dir_entry_t *parent = current_sb->root; // 父目录项
    dir_entry_t *path = nullptr;      // 子目录项
    char_t filename[64];
    // 越过路径前的 '/'
    while (*name == '/')
        name++;
    // 路径若为空，则返回根目录
    if (!*name)
        return parent;

    nameDep = path_depth_cnt(name); // 计算文件名深度

    // 此处为路径上的一系列文件，都创建了dir_entry结构体
    for (;;)
    {   
        Count++;
        // 取得一层目录名字,存入dentryname缓冲区
        tmpname = name;
        while (*name && (*name != '/'))
            name++;
        tmpnamelen = name - tmpname;

        // 去缓存中找目录项
        memcpy(tmpname, filename, tmpnamelen);
        path = find_dir_childern(parent, filename, tmpnamelen);
        if(path != nullptr)
            goto next_floder;
        
        path = (dir_entry_t *)knew(sizeof(dir_entry_t), 0);

        // 准备好要找的文件名
        path->name = knew(tmpnamelen + 1, 0);
        memset(path->name, 0, tmpnamelen + 1);
        memcpy(tmpname, path->name, tmpnamelen);
        path->name_length = tmpnamelen;
        list_init(&path->child_node);
        list_init(&path->subdirs_list);

        // lookup函数从当前目录中搜索与目标名想匹配的目录项。
        // 如果匹配成功，那么lookup函数将返回目标名的短目录项，失败返回NULL
        // 注意此处的Path是一个传出参数，如果在parent中，寻找成功。
        // 那么path中记录目标文件的index_node, 通过inode可以获得该文件的所有信息
        if (parent->dir_inode->inode_ops->lookup(parent->dir_inode, path) == nullptr)
        { // 查找失败，释放申请的内存资源，返回
            if ((flags & 1) && (Count == nameDep))
            {
                goto continue_for;
            }

            DEBUGK("can not find file or dir:%s\n", path->name);
            kdelete(path->name, path->name_length);
            kdelete(path, sizeof(dir_entry_t));

            return nullptr;
        }
    continue_for:
        // child_node 来记录我是谁的子文件, 加入到父目录的列表中
        // suddires_list 来记录我的子文件都有谁
        path->parent = parent; 
        list_add_to_behind(&parent->subdirs_list, &path->child_node);
        path->dir_ops = parent->dir_ops;
        path->d_sb = parent->d_sb;

    next_floder:
        if (!*name) // 检测字符串是否结束
            goto last_component;
        while (*name == '/') // 递增name到下一个路径名称
            name++;
        if (!*name)
            goto last_slash;

        parent = path;
    }

last_component: // 最后的组成成分
last_slash:     // 最后的斜杠

    // 当形参flags = 1时, path_walk函数返回目标父目录的目录项，并且填冲要创建文件的inode， 表示创建文件
    if (flags == 1)
    {
        if (path->dir_inode)
        {
            DEBUGK("File already exit!!!");
            return nullptr;
        }
        *create_file = path;
        return parent;
    }
    
    // 当形参flags = 2时, 表示获得文件的目录项 和 文件目录的目录项
    if (flags ==  2)
    {
        *create_file = path;
        return parent;
    }

    // 其余情况返回文件的目录项
    return path;
}

// 设置文件指针的位置。这个函数的使用能否提升到VFS层面?
s64_t FS_lseek(file_t *filp, s64_t offset, s64_t origin)
{

    s64_t pos = 0;

    switch (origin)
    {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = filp->position + offset;
        break;
    case SEEK_END:
        pos = filp->dentry->dir_inode->file_size + offset;
        break;
    default:
        return -EINVAL;
        break;
    }

    if (pos < 0 || pos > filp->dentry->dir_inode->file_size)
        return -EOVERFLOW; // 访问位置不正确

    filp->position = pos;
    // color_printk(GREEN, BLACK, "FAT32 FS(lseek) alert position:%d\n", filp->position);
    return pos;
}


ide_part_t part[4];
dev_t DEV[4];
extern struct file_system_type FAT32_fs_type;
extern struct file_system_type MINIX_fs_type;

void DISK1_FAT32_FS_init() // 该函数不应该出现在这里
{
    // --------VFS_init-----------
    super_init();
    // --------------------

    u8_t buf[512];

    // 在VFS中注册FAT32文件系统
    register_filesystem(&FAT32_fs_type);
    register_filesystem(&MINIX_fs_type);

    // 读MBR
    memset(buf, 0, 512);
    IDE_device_operation.transfer(ATA_READ_CMD, 0x0, 1, (u8_t *)buf);
    struct Disk_Partition_Table DPT = *(struct Disk_Partition_Table *)buf;
    
    for(u8_t i = 0; i < 4; i++) {
        
        if(DPT.DPTE[i].start_LBA != 0) {
            DEBUGK("DPTE[%d] start_LBA:%#lx\ttype:%#lx\tsectors:%#lx\n", i, DPT.DPTE[i].start_LBA, DPT.DPTE[i].type, DPT.DPTE[i].sectors_limit);
            sprintf(part[i].name, "1_part%d", i);
            part[i].disk = nullptr;
            part[i].start = DPT.DPTE[i].start_LBA;
            part[i].count = DPT.DPTE[i].sectors_limit;
            part[i].system = DPT.DPTE[i].type;
            DEV[i] = device_install(DEV_BLOCK, DEV_IDE_PART, &part[i], part[i].name, 0, &IDE_device_operation);
        }
    }
    // 读 FAT32文件系统的引导扇区
    memset(buf, 0, 512);
    IDE_device_operation.transfer(ATA_READ_CMD, DPT.DPTE[0].start_LBA, 1, (u8_t *)buf);

    // 挂载fat32系统
    sb_vec[0] = current_sb = mount_fs("FAT32", &DPT.DPTE[0], buf);

    // 挂载minix系统
    sb_vec[1] = mount_fs("MINIX", &DPT.DPTE[1], 0);
    
    // 把 minix 系统挂载在 /mnt
    // 这样的挂载方式 我无法判断谁在那里挂载了哎
    dir_entry_t* catalogue  = path_walk("/mnt" , 0, nullptr);
    assert(catalogue != nullptr);
    
    catalogue->dir_inode = sb_vec[1]->root->dir_inode;
    catalogue->d_sb = sb_vec[1];
    catalogue->d_sb->s_flags = true;
    catalogue->dir_ops = sb_vec[1]->root->dir_ops;

    // 设置init进程的当前文件系统路径 和 跟目录
    current->parent->i_pwd = current->i_pwd = sb_vec[0]->root;
    current->parent->i_root = current->i_root = sb_vec[0]->root;
}

void VFS_destory(void) {

}
