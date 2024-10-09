#ifndef _SEMAPHORE_F_H_
#define _SEMAPHORE_F_H_

void semaphore_down(semaphore_t *semaphore);
void semaphore_up(semaphore_t *semaphore);
void semaphore_init(semaphore_t *semaphore, u64_t count);

#endif // _SEMAPHORE_F_H_