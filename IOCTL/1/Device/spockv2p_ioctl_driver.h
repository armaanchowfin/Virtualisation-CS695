/**
 * FILE: spockv2p_ioctl_driver.h
 * --------------------------------
 * Contains Driver Public API declarations and helper structs
 * used by the driver module
*/
#ifndef __IOCTL_DEFINE_DRIVER_H__
#define __IOCTL_DEFINE_DRIVER_H__
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/cdev.h>	
#include <linux/uaccess.h>

// To store driver data. Private to module.
typedef struct spockv2p_device_data_t {
    struct cdev cdev;

    /* Synchronization Primitives */ 
    atomic_t available;
} spockv2p_device_data_t;

/* Public API */
int spockv2p_open(struct inode *inode, struct file *filp);
int spockv2p_close(struct inode *inode, struct file *filp);
long spockv2p_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

/* Private API */

#endif