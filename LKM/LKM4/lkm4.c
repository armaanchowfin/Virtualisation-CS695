#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_DESCRIPTION("Kernel module to determine memory usage of a process");

static int pid = -1;

module_param(pid, int, 0); // Accept PID as a parameter when loading the module

static int __init lkm4_init(void) {
    struct task_struct *task;
    struct mm_struct *mm;
    unsigned long vsize = 0; // Virtual size
    unsigned long psize = 0; // Physical size

    if (pid == -1) {
        pr_info("PID not provided. Please provide a PID.\n");
        return -EINVAL;
    }

    // Find the task_struct corresponding to the given PID
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        pr_info("Invalid PID provided.\n");
        return -EINVAL;
    }

    mm = task->mm;
    if (!mm) {
        pr_info("No memory map found for the provided PID.\n");
        return -EINVAL;
    }

    // Lock the mm_struct to prevent changes
    down_read(&mm->mmap_lock);
    VMA_ITERATOR(iter, mm, 0);
    struct vm_area_struct *vma;

    for_each_vma(iter, vma) {
        vsize += vma->vm_end - vma->vm_start;
        if (vma->vm_flags & VM_PFNMAP) {
            psize += vma->vm_end - vma->vm_start;
        }
    }
    // Unlock the mm_struct
    up_read(&mm->mmap_lock);

    pr_info("Virtual size of process %d: %lu bytes\n", pid, vsize);
    pr_info("Mapped physical size of process %d: %lu bytes\n", pid, psize);

    return 0;
}

static void __exit lkm4_exit(void) {
    pr_info("Exiting memory usage module\n");
}

module_init(lkm4_init);
module_exit(lkm4_exit);
