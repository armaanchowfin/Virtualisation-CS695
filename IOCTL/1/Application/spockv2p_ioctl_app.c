/**
 * FILE: spockv2p_ioctl_app.c
 * Userspace app to test the IOCTL driver.
 * 1. Sends a VA to device to test VA-PA translation.
 * 2. Sends a {PA, val} struct to device to trigger set-val-at-PA
 * 3. Sends a PA to device to test previous set-val-at-PA.
*/

#include <stdlib.h>  // malloc, free
#include <stdio.h>
#include <errno.h>  // errno
#include <error.h>  // error()
#include <fcntl.h>  // open()
#include <unistd.h> // close()
#include <sys/ioctl.h> // ioctl()
#include <assert.h>
#include <string.h>  // memset()
#include <stdint.h>

#include "../Device/spockv2p_ioctl_common.h"
#include "../Device/ioctl.h" // Double include of common.h handled by header guard.

#define IOCTL_DRIVER_NAME "/dev/spockv2p"  // device-file in /dev

/* Error Codes */
static const int e_FailedDeviceOpen = 1;
static const int e_FailedDeviceClose = 2;
static const int e_FailedIoctlCommandV2P = 4;
static const int e_FailedIoctlCommandSetValue = 8;
static const int e_FailedTestSetValue = 9;
static const int e_FailedIoctlCommandGetValue = 16;

static const int e_FailedMalloc = 32;
//static const int e_FailedFree = 64;


static void *safe_malloc(size_t sz) {
    void *ptr = malloc(sz);
    if (!ptr) {
        error(e_FailedMalloc, errno, "Malloc of size %ld failed.\n", sz);
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, sz);
    return ptr;
}

static int open_driver(const char *driver_module_name) {
    printf("** Open Device **\n");

    int driver_fd = open(driver_module_name, O_RDWR);  // How to decide flag ?
    if (driver_fd == -1) {
        error(e_FailedDeviceOpen, errno, "Could not open \"%s\".\n", driver_module_name);
        exit(EXIT_FAILURE);
    }
    return driver_fd;
}

static void close_driver(const char *driver_module_name, int driver_fd) {
    printf("** Close Device **\n");

    int ret = close(driver_fd);
    if (ret == -1) {
        error(e_FailedDeviceClose, errno, "Could not close \"%s\".\n", driver_module_name);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    uint8_t *val = safe_malloc(sizeof(char));
    *val = 6;
    printf("Stored value %d at VA <%p>.\n", *val, val);

    struct va_pa_t *va_pa = safe_malloc(sizeof(struct va_pa_t));
    va_pa->va = (uintptr_t)val;
    assert(!va_pa->pa);

    // access ioctl device file
    int ioctl_device_fd = open_driver(IOCTL_DRIVER_NAME);
    
    /* 
     * Send a VA to device to test VA-PA translation.
     * kernel is entered via a system call i.e. valid process context 
     * the kernel can directly "dereference" user space pointer to get the correct value. 
     */
    if(ioctl(ioctl_device_fd, IOCTL_GET_PHYSICAL_FROM_VIRTUAL_ADDR, va_pa) < 0) {
        error(e_FailedIoctlCommandV2P, errno, "IOCTL 0 failed.\n");
        exit(EXIT_FAILURE);
    }
    printf("VA %lx has PA %p.\n", va_pa->va, (void *)va_pa->pa);

    /* 
     * Send a {PA, val} struct to device to trigger set-val-at-PA 
     */
    struct pa_val_t *pa_val_set = safe_malloc(sizeof(struct pa_val_t));
    pa_val_set->pa = va_pa->pa;                 // set pa
    pa_val_set->val = 5;                        // val to set at pa

    if (ioctl(ioctl_device_fd, IOCTL_SET_VALUE_AT_PHYSICAL_ADDR, pa_val_set) < 0) {
        error(e_FailedIoctlCommandSetValue, errno, "IOCTL 1 failed.\n");
        exit(EXIT_FAILURE); 
    }
    printf("Attempted to set value %d to PA %lx.\n", pa_val_set->val, pa_val_set->pa);

    /* 
     * Send a PA to device to test previous set-val-at-PA 
     */
    struct pa_val_t *pa_val_get = safe_malloc(sizeof(struct pa_val_t));
    pa_val_get->pa = va_pa->pa;                 // set pa
    assert(!pa_val_get->val);                   // val to set at pa
    
    if (ioctl(ioctl_device_fd, IOCTL_GET_VALUE_AT_PHYSICAL_ADDR, pa_val_get) < 0) {
        error(e_FailedIoctlCommandGetValue, errno, "IOCTL 1 failed.\n");
        exit(EXIT_FAILURE); 
    } 
    
    if (pa_val_get->val != pa_val_set->val) {
        printf("%d: Expected: %d, Got: %d\n", e_FailedTestSetValue, pa_val_set->val, pa_val_get->val);
        return EXIT_FAILURE;
    }
    else {
        printf("**!! TEST PASSED !!**\n");
        return EXIT_SUCCESS;
    }

    close_driver(IOCTL_DRIVER_NAME, ioctl_device_fd);

    free(val);
    free(va_pa);
    free(pa_val_set);
    free(pa_val_get);
    
}