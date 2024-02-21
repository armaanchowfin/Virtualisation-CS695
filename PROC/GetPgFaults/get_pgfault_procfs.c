/**
 * FILE: get_pgfault_procfs.c
 * --------------------------
 * 
 */
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> 
#include <linux/version.h> 
#include <linux/mm.h>           /* Needed for vmstat.h */
#include <linux/vmstat.h>       /* Needed for all_vm_events */

#define DRIVER_AUTHOR "Armaan Chowfin"
#define DRIVER_DESC   "return total pgfaults to useerspace"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	    
MODULE_DESCRIPTION(DRIVER_DESC);	

// proc_ops introduced after v5.6.0 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0) 
#define HAVE_PROC_OPS 
#endif 


#define procfs_name "get_pgfault_procfs"

static unsigned long *counter_buf;     // a place to copy the VM stats
static struct proc_dir_entry *new_proc_file;  // our /proc entry

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, 
                             size_t buffer_length, loff_t *offset) {
    int ret;
    unsigned long pfsz;
    struct mm_struct *mm;
    struct vm_area_struct *vma;

    if (*offset > 0) {
        ret = 0;
    } else {
        VMA_ITERATOR(iter, mm, 0);
        for_each_vma(iter, vma) {
            all_vm_events(counter_buf);
        }
    }
    int len = sizeof(counter_buf);
    int i;
    for (i = 0; i < len; i++) {
        pfsz += counter_buf[i];
    }

    ret = snprintf(buffer, buffer_length, "Total Page Faults: %lu\n", pfsz);
    if (copy_to_user(buffer, counter_buf, len)) {
        pr_info("copy to user failed.\n");
    } else {
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name); 
        *offset += len;   
    }
    return ret;
}


#ifdef HAVE_PROC_OPS 
static const struct proc_ops proc_file_fops = { 
    .proc_read = procfile_read, 
}; 
#else 
static const struct file_operations proc_file_fops = { 
    .read = procfile_read, 
}; 
#endif 

// Allocate space to store vma stats.
static int __init get_pgfault_init(void) {
       // allocate space
        counter_buf = kmalloc(NR_VM_ZONE_STAT_ITEMS * sizeof(unsigned long)
                                + sizeof(struct vm_event_state) + 100,
                              GFP_KERNEL);  // extra 100 just in case
        if (counter_buf == NULL) {
            pr_err("kmalloc Failed.\n");
        }

	new_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
	
	if (new_proc_file == NULL) {
		proc_remove(new_proc_file);
        pr_alert("Error:Could not initialize /proc/%s\n", procfs_name); 
        return -ENOMEM; 
	}

    pr_info("/proc/%s created\n", procfs_name); 
    return 0; 
}

// this gets called when our module is removed.  We will remove the
// proc entry, and free our allocated memory.
static void __exit get_pgfault_exit(void) {
    pr_info("/proc/%s removed\n", procfs_name); 
    if (counter_buf != NULL) {
        kfree(counter_buf);
        counter_buf = NULL;
    }
}

module_init(get_pgfault_init); 
module_exit(get_pgfault_exit); 
 
MODULE_LICENSE("GPL");