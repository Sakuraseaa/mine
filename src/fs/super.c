#include "toolkit.h"
#include "fskit.h"
list_t super_list;

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
// 获得设备 dev 的超级块
spblk_t *get_super(dev_t dev)
{
    list_t* ptr = super_list.next;
    while (ptr != &super_list)
    {
        spblk_t *super = container_of(ptr, spblk_t, node);
        if(super->dev == dev)
            return super;

        ptr = ptr->next;
    }
    return nullptr;
}

void super_init() {
    list_init(&super_list);
}