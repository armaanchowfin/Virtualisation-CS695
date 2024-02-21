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
#include <sys/types.h>
#include <error.h>

// Include header or define the IOCTL call interface and devide name
#include "../Device/ioctl.h"

#define IOCTL_DEVICE_NAME "/dev/doom"

/* Error Codes */
static const int e_FailedDeviceOpen = 1;
static const int e_FailedDeviceClose = 2;
static const int e_FailedIoctlCommandMakeParent = 4;

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


int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <parent_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = atoi(argv[1]); // pass to driver.
    
    printf("* At Soldier init:\nPID of soldier: %d, PPID of soldier: %d *\n\n", getpid(), getppid());
    printf("** Setting PPID of soldier to: %d **\n", parent_pid);


    // open ioctl driver
    int ioctl_device_fd = open_driver(IOCTL_DEVICE_NAME); 

    // call ioctl with parent pid as argument to change the parent
    if (ioctl(ioctl_device_fd, IOCTL_CMD_MAKE_PARENT, (void *)&parent_pid) < 0) {
        error(e_FailedIoctlCommandMakeParent, errno, "IOCTL %d failed.\n", IOCTL_CMD_MAKE_PARENT);
        exit(EXIT_FAILURE);
    }

    // parent of current process must be the pid of the control station.
    assert(parent_pid == getppid());
    printf("!!*** TEST PASSED ***!!\n");

    close_driver(IOCTL_DEVICE_NAME, ioctl_device_fd);

	return EXIT_SUCCESS;
}


