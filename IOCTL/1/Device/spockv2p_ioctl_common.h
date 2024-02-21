/**
 * FILE: spockv2p_ioctl_common.h
 * --------------------------------
 * Contains definitions of any common compound data-types
 * used by the application-driver pair.
 * Include in user-space and Kernel-Space.
*/

#ifndef __IOCTL_DEFINE_COMMON_H__
#define __IOCTL_DEFINE_COMMON_H__
#include <linux/types.h>
// To pass a Physical Address and value-to-set-at-physical-address from app to driver.
struct pa_val_t {
    uintptr_t pa;
    uint8_t val;           // char is always 1 byte
};

// Driver populates pa and copy-to-user.
struct va_pa_t {
    uintptr_t va;
    uintptr_t pa;
};

#endif
