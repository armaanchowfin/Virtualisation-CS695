/**
 * FILE: ioctl.h
 * -------------
 * Define the IOCTL command that are recognised by the 
 * doom device. Include this file in the kernel module.
 * and user-space application. Header guards are named
 * to prevent clashes with kernel ioctl.h
*/

#ifndef __IOCTL_DEFINE_H__
#define __IOCTL_DEFINE_H__
#include <linux/types.h> // pid_t

#define IOCTL_MAGIC 'W'

/* IOCTL Commands */
#define IOCTL_CMD_MAKE_PARENT _IOW(IOCTL_MAGIC, 0, pid_t)   // Kernel copy_from user and updates kernel-space struct. No copy-to user.

#endif