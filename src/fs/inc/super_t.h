#ifndef _SUPER_T_H_
#define _SUPER_T_H_

spblk_t *get_super(dev_t dev);
bool register_super(spblk_t * sb);
void super_init();

#endif // _SUPER_T_H_