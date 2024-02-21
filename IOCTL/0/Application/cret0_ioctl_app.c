#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <error.h>
#include <errno.h>
#include "../Device/ioctl.h"

#define IOCTL_DRIVER_NAME "/dev/cret0" // Device-File name

static const int e_FailedDeviceOpen = 1;
static const int e_FailedDeviceClose = 2;
static const int e_FailedIoctlCommand = 4;

static int open_driver(const char *driver_module_name);
static void close_driver(const char *driver_module_name, int driver_fd);

static int open_driver(const char *driver_module_name) {
    printf("* Open Device\n");

    int driver_fd = open(driver_module_name, O_RDWR);  // How to decide flag ?
    if (driver_fd == -1) {
        error(e_FailedDeviceOpen, errno, "Could not open \"%s\".\n", driver_module_name);
        exit(EXIT_FAILURE);
    }
    return driver_fd;
}

static void close_driver(const char *driver_module_name, int driver_fd) {
    printf("* Close Device\n");

    int ret = close(driver_fd);
    if (ret == -1) {
        error(e_FailedDeviceClose, errno, "Could not close \"%s\".\n", driver_module_name);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    int ioctl_device_fd = open_driver(IOCTL_DRIVER_NAME);
    
    uint32_t val;
    if (ioctl(ioctl_device_fd, IOCTL_GET_CONST, &val) < 0) {
        error(e_FailedIoctlCommand, errno, "IOCTL_GET_CONST failed");
        exit(EXIT_FAILURE);
    }
    printf("Value returned from device: %u\n", val); // expect 305419896 if driver val = 0x12345678

    close_driver(IOCTL_DRIVER_NAME, ioctl_device_fd);

    // main() implicitly calls return 0 at end (i.e. cleanup actions). 
    // Therefore cannot reproduce Device open error log on 2nd run.
    return EXIT_SUCCESS;
}