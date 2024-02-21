/**
  * Write a kernel module that takes process ID and a virtual address as its input. 
  * Dtermine if the virtual address is mapped, and if so, determine its physical address (pseudo physical address if running in a VM). 
  * Print the pid, virtual address, and corresponding physical address. Name the source file as lkm3.c
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pgtable.h>
#include <linux/mm.h>
#include <linux/highmem.h> // kmap
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

MODULE_DESCRIPTION("LKM3");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_LICENSE("GPL");

// Command-line arguments
static pid_t target_pid = 1;  // Default target process ID
static unsigned long target_virtual_address = 0x1000;  // Default target virtual address

module_param(target_pid, int, S_IRUGO);
MODULE_PARM_DESC(target_pid, "Target process ID");

module_param(target_virtual_address, ulong, S_IRUGO);
MODULE_PARM_DESC(target_virtual_address, "Target virtual address");

// Function to print the physical address corresponding to the given virtual address
static void print_physical_address(struct pid *pid, unsigned long user_va) {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep, pte;
    
    struct page *page = NULL;   // Represents a physical page.

    struct task_struct *task;
    task = pid_task(pid, PIDTYPE_PID);

    struct mm_struct *mm;
    mm = task->mm;

    pgd = pgd_offset(mm, user_va);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        pr_warn("Bad PGD\n");
        goto out;
    }
    pr_info("Valid PGD (Pagetable Global Directory)\n");

    p4d = p4d_offset(pgd, user_va);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        pr_warn("Bad P4D\n");
        goto out;
    }
    pr_info("Valid P4D\n");

    pud = pud_offset(p4d, user_va);
    if (pud_none(*pud) || pud_bad(*pud)) {
        goto out;
    }
    pr_info("Valid PUD (Pagetable Upper Directory)\n");

    pmd = pmd_offset(pud, user_va);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        goto out;
    }
    pr_info("Valid PMD (Pagetable Middle Directory)\n");

    ptep = pte_offset_map(pmd, user_va);
    if (!ptep) {
        goto out;
    }
    pte = *ptep;

    // The macro pte_page() returns the struct page which corresponds to the PTE entry.
    page = pte_page(pte);
    if (page) {
        pr_info("Struct page is at kernel virtual address: %lx\n, page addr: %lx\n", (uintptr_t)page, (uintptr_t)page_address(page));
    }

    uintptr_t phys_addr = (page_to_phys(page)) | (user_va & ~PAGE_MASK);
    pr_info("PID: %d, Virtual Address: %lx, Physical Address: %lx\n", pid, user_va, phys_addr);

out:
    pr_err("VA %lx in not mapped to process with pid %d!\n", target_virtual_address, target_pid);
}

// Module initialization function
static int __init lkm_init(void) {
    pr_info("Mapping virtual address to physical address...\n");
    if (target_pid < 0) {
        pr_err("Need to supply command-line argument: target_pid=<pid>\n");
        return -EINVAL;
    }

    struct pid *base_pid = find_get_pid(target_pid);

    print_physical_address(base_pid, target_virtual_address);
    return 0;
}

// Module cleanup function
static void __exit lkm_exit(void) {
    pr_info("Module unloaded.\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
