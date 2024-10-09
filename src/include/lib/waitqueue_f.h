#ifndef _WAITQUEUE_F_H_
#define _WAITQUEUE_F_H_

bool wait_queue_is_empty(wait_queue_t* wait_queue);
void wait_queue_init(wait_queue_t *wait_queue, struct task_struct *tsk);
void sleep_on(wait_queue_t *wait_queue_head);
void interruptible_sleep_on(wait_queue_t *wait_queue_head);
void wakeup(wait_queue_t *wait_queue_head, long state);
void wakeup_pid(wait_queue_t *wait_queue_head, long state, long pid);

#endif // _WAITQUEUE_F_H_