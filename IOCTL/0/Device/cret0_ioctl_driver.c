/**
 * FILE: ioctl_driver.c
 * --------------------
 * Implements the core driver logic. 
 * When loaded, an ioctl call from userspace to 
 * the IOCTL_GET_CONST number will cause driver to write
 * a number to user-space address passed to the ioctl.
 */

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

#include "ioctl.h" // IOCTL_GET_CONST

/* static/global may affect driver working based on linking etc.*/
int cret0_major = 0;
int cret0_minor = 0;
#define DEVICE_COUNT 1

#define DEVICE_NAME "cret0" // constant returner
char *device_name = DEVICE_NAME; // To pass name to alloc_chrdev_region()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_DESCRIPTION("IOCTL driver: Constant Returner");

typedef struct cret0_device_data_t {
    struct cdev cdev;

    /* Synchronization Primitives */ 
    atomic_t available;
} cret0_device_data_t;

cret0_device_data_t cret0_device_data; // Declare global, static.

// file_operations must be declared before usage.
int cret0_open(struct inode *inode, struct file *filp);
int cret0_close(struct inode *inode, struct file *filp);
long cret0_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

struct file_operations cret0_fops = {
    .owner = THIS_MODULE,
    .open = cret0_open,
    .unlocked_ioctl = cret0_ioctl,
    .release = cret0_close
};

/* Private API */
static int cret0_setup_cdev(cret0_device_data_t* cret0_device_data);

// cdev is filled and device is registered after obtaining major, minor
int cret0_setup_cdev(cret0_device_data_t *cret0_device_data) {
    int err = 0;
    dev_t devno = MKDEV(cret0_major, cret0_minor);

    cdev_init(&cret0_device_data->cdev, &cret0_fops);
    cret0_device_data->cdev.owner = THIS_MODULE;
    //cret0_device_data->cdev.ops = &cret0_fops; // What does cdev_init do?: driver works even without this..?
    err = cdev_add(&cret0_device_data->cdev, devno, DEVICE_COUNT);

    return err;
}

/* Public API */

// Called on fd open(). After device reg. Passes device data to the inode.
int cret0_open(struct inode *inode, struct file *filp) {
    cret0_device_data_t *cret0_device_data_from_inode;

    // Need value from inode to track open/close state?
    cret0_device_data_from_inode = container_of(inode->i_cdev, cret0_device_data_t, cdev); //  Get cdev from inode.
    filp->private_data = cret0_device_data_from_inode; 
    // filp->private_data = &cret0_device_data;   // even access global struct works. So why use container_of?

    if (!atomic_dec_and_test(&cret0_device_data_from_inode->available)) {
        atomic_inc(&cret0_device_data_from_inode->available);               // access address of inode field.
        pr_alert("Open cret0: Device opened already, cannot re-open.\n");
        return -EBUSY;
    }
    return 0;
}

// Called on fd close()
int cret0_close(struct inode *inode, struct file *filp) {
    cret0_device_data_t *cret0_device_data = filp->private_data; // cast rhs void *
    atomic_inc(&cret0_device_data->available); // Release device
    return 0;
}

long cret0_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    pr_alert("Testing IOCTL entry\n");
    switch (cmd) {
        case IOCTL_GET_CONST:
            pr_info("<%s> ioctl: IOCTL_GET_CONST\n", DEVICE_NAME);
            uint32_t value = 0x12345678; // to be sent to user-space
            
            if (copy_to_user((uint32_t *)arg, &value, sizeof(value))) { 
                return -EFAULT;
            }
            break;
        
        default:
            break;
    }
    return 0;
}

/* Device Initialisation */
static int __init cret0_init(void) {
    dev_t devno = 0;
    int ret = 0;

    /**
     * Begin Device registration
    */
    ret = alloc_chrdev_region(&devno, cret0_minor, DEVICE_COUNT, device_name);
    
    cret0_major = MAJOR(devno);

    if (ret < 0) {
        pr_warn("cret0: Can't get major number, default = %d\n", cret0_major);
        goto fail;
    }

    // Init the device data if necessary
    memset(&cret0_device_data, 0, sizeof(cret0_device_data_t));
    atomic_set(&(cret0_device_data.available), 1);
    ret = cret0_setup_cdev(&cret0_device_data);

    if (ret < 0) {
        pr_warn("cret0: Error %d adding cret0 device.\n", ret);
        goto fail;
    }

    pr_info("cret0: Module loaded\n");
    return 0;

fail:
    return ret;
}

/* Device Removal */
static void __exit cret0_exit(void) {
    dev_t devno = MKDEV(cret0_major, cret0_minor);

    cdev_del(&cret0_device_data.cdev);
    unregister_chrdev_region(devno, DEVICE_COUNT);
    pr_info("cret0: Module Unloaded.\n");
}

module_init(cret0_init);
module_exit(cret0_exit);
