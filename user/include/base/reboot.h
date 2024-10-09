#ifndef __REBOOT_H__

#define __REBOOT_H__

#define	SYSTEM_REBOOT	(1UL << 0)
#define	SYSTEM_POWEROFF	(1UL << 1)

unsigned long reboot(unsigned long cmd,void * arg);

#endif