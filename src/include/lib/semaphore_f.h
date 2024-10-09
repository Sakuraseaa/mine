#ifndef _SEMAPHORE_F_H_
#define _SEMAPHORE_F_H_

void semaphore_down(semaphore_t *semaphore);
void semaphore_up(semaphore_t *semaphore);
void semaphore_init(semaphore_t *semaphore, unsigned long count);
void wait_queue_init(wait_queue_t *wait_queue, struct task_struct *tsk);

#endif // _SEMAPHORE_F_H_