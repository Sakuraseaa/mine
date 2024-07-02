#ifndef __INODE_H__
#define __INODE_H__

#include "types.h"
#include "lib.h"
#include "VFS.h"

inode_t *find_inode(dev_t dev, idx_t nr);

#endif