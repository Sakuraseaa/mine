#ifndef __SUPER_H__
#define __SUPER_H__

#include "basekit.h"
#include "VFS.h"


extern list_t super_list;
super_t *get_super(dev_t dev);
void super_init();

#endif