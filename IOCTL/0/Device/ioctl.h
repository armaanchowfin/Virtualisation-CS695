/**
 * FILE: ioctl.h
 * --------------
 * Contains the #defined ioctl numbers supported by
 * the driver. Include this file in the kernel module
 * and user-space application. Header guards are named
 * to prevent clashes with kernel ioctl.h
*/
#include <linux/ioctl.h>
#ifndef __IOCTL_DEFINE_H__
#define __IOCTL_DEFINE_H__

#define IOCTL_TYPE 'W'                          // IOCTL Magic Number
#define IOCTL_GET_CONST _IO(IOCTL_TYPE, 1)      // Sets the IOCTL number for "simple" IOCTLS 

#endif