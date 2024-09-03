#ifndef __SCHED_H__

#define __SCHED_H__

/*
 * cloning flags:
 */


#define CLONE_VM		(1UL << 0)	/* shared Virtual Memory between processes */
#define CLONE_FS		(1UL << 1)	/* shared fs info between processes */
#define CLONE_SIGNAL	(1UL << 2)	/* shared signal between processes */

#endif
