/**
 * FILE: ioctl.h
 * --------------
 * Contains the #defined ioctl numbers supported by
 * the driver. Include this file in the kernel module
 * and user-space application. Header guards are named
 * to prevent clashes with kernel ioctl.h
*/
#ifndef __IOCTL_DEFINE_H__
#define __IOCTL_DEFINE_H__
#include "spockv2p_ioctl_common.h" // struct pa_val_t

#define IOCTL_MAGIC 'W'

/* Create IOCTL numbers for 3 userspace request-types */
#define IOCTL_GET_PHYSICAL_FROM_VIRTUAL_ADDR    _IOWR(IOCTL_MAGIC, 0, struct va_pa_t)         // Kernel copy_from to store VA and copy_to to return PA.
#define IOCTL_SET_VALUE_AT_PHYSICAL_ADDR        _IOW(IOCTL_MAGIC, 1, struct pa_val_t)   // Kernel copy_from to store pa_val, returns nothing to user.
#define IOCTL_GET_VALUE_AT_PHYSICAL_ADDR        _IOWR(IOCTL_MAGIC, 2, struct pa_val_t)         // Kernel copy_from to store PA and copy_to to return value at PA.      

#endif