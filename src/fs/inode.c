#include "VFS.h"
#include "lib.h"
#include "fs.h"
#include "inode.h"
#include "super.h"
list_t super_list;

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
// 获得设备 dev 的超级块
inode_t *find_inode(dev_t dev, idx_t nr)
{
    super_t *super = get_super(dev);
    list_t *list = &super->inode_list;

    for (list_t *node = list->next; node != list; node = node->next)
    {
        inode_t *inode = container_of(node, inode_t, i_sb_list);
        if (inode->nr == nr)
        {
            return inode;
        }
    }
    return NULL;
}

/**
 * @brief 找到文件名filename 对应的inode
 * 
 * @param filename 
 * @return inode_t* 
 */
inode_t *namei(char* filename) {
    // 由于filename可能是绝对路径的原因 所以我要拼凑出完整路径
    // 使用 filename 和 getcwd()的方式拼接，但布置到完成路径长度申请多长缓冲合适

    // 如果是完整路径 我们可以使用path_walk得到该文件的目录项，从而得到 inode;
}
