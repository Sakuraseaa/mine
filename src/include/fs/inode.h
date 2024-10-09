#ifndef __INODE_H__
#define __INODE_H__

inode_t *find_inode(dev_t dev, idx_t nr);
inode_t *namei(char* filename);

#endif