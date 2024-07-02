#include "VFS.h"
#include "lib.h"
#include "fs.h"

list_t super_list;

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
// 获得设备 dev 的超级块
super_t *get_super(dev_t dev)
{
    list_t* ptr = super_list.next;
    while (ptr != &super_list)
    {
        super_t *super = container_of(ptr, super_t, node);
        if(super->dev == dev)
            return super;

        ptr = ptr->next;
    }
    return NULL;
}

void super_init() {
    list_init(&super_list);
}