#ifndef _INODE_F_H_
#define _INODE_F_H_

inode_t *find_inode(dev_t dev, idx_t nr);
inode_t *namei(str_t filename);

#endif // _INODE_F_H_