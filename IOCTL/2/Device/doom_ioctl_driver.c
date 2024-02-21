/**
 * FILE: doom_ioctl_driver.c
 * -------------------------
 * doom driver source. Receives pid from user-space and changes pid of
 * `parent` of current soldier process from bash to the pid of the 
 * running the control_station process.
*/
#include "ioctl.h"
#include "doom_ioctl_driver.h"
#include <linux/sched.h>

#define DEVICE_COUNT 1

#define DEVICE_NAME "doom"
char *device_name = DEVICE_NAME; // pass to alloc_chrdev_region

static int doom_minor = 0;
static int doom_major = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_DESCRIPTION("IOCTL driver: Establish Parent-Child Relationship");

static doom_device_data_t device_data;

struct file_operations doom_fops = {
    .owner = THIS_MODULE,
    .open = doom_open,
    .unlocked_ioctl = doom_ioctl,
    .release = doom_close
};

/* Private API */
static int doom_device_initdata(doom_device_data_t *device_data) {
    memset(device_data, 0, sizeof(doom_device_data_t));
    atomic_set(&device_data->available , 1);

    // init cdev
    int err = 0;
    dev_t devno = MKDEV(doom_major, doom_minor);

    cdev_init(&device_data->cdev,  &doom_fops); // set cdec.ops
    device_data->cdev.owner = THIS_MODULE;
    err = cdev_add(&device_data->cdev, devno, DEVICE_COUNT);

    return err;
}

/* Public API */
int doom_open(struct inode *inode, struct file *filp) {
    doom_device_data_t *device_data_from_inode;
    device_data_from_inode = container_of(inode->i_cdev, doom_device_data_t, cdev); // get cdev from inode
    filp->private_data = device_data_from_inode;

    if (!atomic_dec_and_test(&device_data_from_inode->available)) {
        atomic_inc(&device_data_from_inode->available);
        pr_alert("Open doom: Device opened already, cannot reopen.\n");
        return -EBUSY;
    }
    return 0;
}

int doom_close(struct inode *inode, struct file *filp) {
    doom_device_data_t *device_data_from_inode = filp->private_data;
    atomic_inc(&device_data_from_inode->available);
    return 0;
}

long doom_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_CMD_MAKE_PARENT:
            pr_info("<%s>: IOCTL_GET_PHYSICAL_FROM_VIRTUAL_ADDR\n", DEVICE_NAME);
            {
                // Is current process always the IOCTL caller?
                pr_debug("(before)\nCurrent PID: %d\nParent PID: %d\nReal Parent PID: %d\n", current->pid, current->parent->pid, current->real_parent->pid);

                // Get desired PID from user-space: copy from
                pid_t new_ppid;
                // if (copy_from_user(&new_ppid, (pid_t *)arg, sizeof(pid_t))) {
                //     return -EFAULT;
                // }
                
                // Faster than cfu for simple value
                if (get_user(new_ppid, (pid_t *)arg)) {
                    return -EFAULT;
                }
                pr_debug("<%s>: Got PID: %d\n", DEVICE_NAME, new_ppid);

                struct task_struct *child_task = current;

                struct task_struct *new_parent = pid_task(find_vpid(new_ppid), PIDTYPE_PID);
                pr_debug("\nnew parent PID: %d\nnew parent PPID: %d\nnew parent real pPPID: %d\n", new_parent->pid, new_parent->parent->pid, new_parent->real_parent->pid);
                
                // Find the task_struct of the new parent process
                if (!new_parent)
                    return -ESRCH; // No such process

                // Update the task structure of the current process to change its parent
                rcu_read_lock();
                if (current->parent)
                    put_task_struct(current->parent);
                current->parent = new_parent;
                get_task_struct(new_parent);
                rcu_read_unlock();

                pr_debug("(after)\nCurrent PID: %d\nParent PID: %d\nReal Parent PID: %d\n", current->pid, current->parent->pid, current->real_parent->pid);
                
            }
            break;

    }
    return 0;

}

static int __init doom_init(void) {
    dev_t devno = 0;
    int ret = 0;

    /*
     * Begin device registration
    */
    ret = alloc_chrdev_region(&devno, doom_minor, DEVICE_COUNT, device_name);
    doom_major = MAJOR(devno);

    // Init device data
    ret = doom_device_initdata(&device_data);
    if (ret < 0) {
        pr_warn("doom: Error %d adding doom device.\n", ret);
        goto fail;
    }
    pr_info("doom: Module loaded\n");
    return 0;

fail:
    return ret;
}

/* Device Removal */
static void __exit doom_exit(void) {
    dev_t devno = MKDEV(doom_major, doom_minor);

    cdev_del(&device_data.cdev);
    unregister_chrdev_region(devno, DEVICE_COUNT);
    pr_info("doom: Module Unloaded.\n");
}

module_init(doom_init);
module_exit(doom_exit);
