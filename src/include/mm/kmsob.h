#ifndef _KMSOB_H_
#define _KMSOB_H_

void init_kmsob();
void *kmsob_new(size_t msz);
bool_t kmsob_delete(void *fadrs, size_t fsz);

#endif // _KMSOB_H_