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