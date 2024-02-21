/**
 * FILE: spockv2p_ioctl_driver.c
 * -----------------------------
 * Implements the core driver logic. 
 * 1. Provides the physical address for a given virtual address. 
 * The virtual-to-physical address translation should be done for the current running process.
 * 2. Given a physical address and a byte value, it should write the value at the memory address.
 */

#include "ioctl.h"                      // ioctl numbers
#include "spockv2p_ioctl_driver.h"      // Driver-Specific header - Contains file operations, device data struct
#include "spockv2p_ioctl_common.h"      // App-Driver common datatypes: pa_val_t
#include <linux/pgtable.h>
#include <linux/mm.h>
#include <linux/highmem.h> // kmap

#define DEVICE_COUNT 1

#define DEVICE_NAME "spockv2p"
char *device_name = DEVICE_NAME; // To pass name to alloc_chrdev_region()

int spockv2p_major = 0;
int spockv2p_minor = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_DESCRIPTION("IOCTL driver: Virtual-To-Physical");

static spockv2p_device_data_t device_data; 

struct file_operations spockv2p_fops = {
    .owner = THIS_MODULE,
    .open = spockv2p_open,
    .unlocked_ioctl = spockv2p_ioctl,
    .release = spockv2p_close
};

/* Private API */
static int spockv2p_device_initdata(spockv2p_device_data_t *device_data) {
    memset(device_data, 0, sizeof(spockv2p_device_data_t));
    atomic_set(&device_data->available, 1);

    // Initialise cdev
    int err = 0;
    dev_t devno = MKDEV(spockv2p_major, spockv2p_minor);

    cdev_init(&device_data->cdev, &spockv2p_fops);   // sets cdev.ops
    device_data->cdev.owner = THIS_MODULE;
    err = cdev_add(&device_data->cdev, devno, DEVICE_COUNT);

    return err;
}

// Walk the page-table of the user-space task
static struct page *walk_page_table(uintptr_t user_va) {
    /* https://www.kernel.org/doc/gorman/html/understand/understand006.html */
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep, pte;
    
    struct page *page = NULL;   // Represents a physical page.
    
    struct mm_struct *mm = current->mm;

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

out:
    return page;
    
}


/* Public API */

// Called on fd open(). After device reg. Passes device data to the inode.
int spockv2p_open(struct inode *inode, struct file *filp) {
    spockv2p_device_data_t *device_data_from_inode;

    device_data_from_inode = container_of(inode->i_cdev, spockv2p_device_data_t, cdev); //  Get cdev from inode.
    filp->private_data = device_data_from_inode;

    if (!atomic_dec_and_test(&device_data_from_inode->available)) {
        atomic_inc(&device_data_from_inode->available);
        pr_alert("Open spockv2p: Device opened already, cannot re-open.\n");
        return -EBUSY;
    }
    return 0;
}

int spockv2p_close(struct inode *inode, struct file *filp) {
    spockv2p_device_data_t *device_data_from_inode = filp->private_data; // cast rhs void *
    atomic_inc(&device_data_from_inode->available); // Release device
    return 0;
}

long spockv2p_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch(cmd) {
        case IOCTL_GET_PHYSICAL_FROM_VIRTUAL_ADDR:
            pr_info("<%s>: IOCTL_GET_PHYSICAL_FROM_VIRTUAL_ADDR\n", DEVICE_NAME);

            // Copy value from user to kernel
            struct va_pa_t get_pa_of;
            memset(&get_pa_of, 0, sizeof(struct va_pa_t));

            if (copy_from_user(&get_pa_of, (struct va_pa_t *)arg, sizeof(struct va_pa_t))) {
                return -EFAULT;
            } 
            pr_info("<%s>: Got VA: %lx\n", DEVICE_NAME, get_pa_of.va); // %p hashes the ptr address, use %lx

            // Walk pages, page->virtual is kernel VA of the page. Use __pa to convert.
            struct page *page_with_kernel_va;
            page_with_kernel_va = walk_page_table(get_pa_of.va);
            
            // https://github.com/davidhcefx/Translate-Virtual-Address-To-Physical-Address-in-Linux-Kernel
            // === unsigned long phys = __pa(page_address(page));
            uintptr_t phys = (page_to_phys(page_with_kernel_va)) | (get_pa_of.va & ~PAGE_MASK);
            get_pa_of.pa = phys;
            pr_info("<%s>: Got Page: %lx with PA: %lx\n", DEVICE_NAME, (uintptr_t)page_with_kernel_va, phys); // %p hashes the ptr address, use %lx

            // Copy to user (__user to, kernel from)
            if (copy_to_user((struct va_pa_t *)arg, &get_pa_of, sizeof(struct va_pa_t))) {
                pr_warn("<%s>: Failed copy-to-user\n", DEVICE_NAME);
                return -EFAULT;
            }
            break;
        
        case IOCTL_SET_VALUE_AT_PHYSICAL_ADDR:
            pr_info("<%s>: IOCTL_SET_VALUE_AT_PHYSICAL_ADDR\n", DEVICE_NAME);
            {
                struct pa_val_t pa_val;
                memset(&pa_val, 0, sizeof(struct pa_val_t));
                if (copy_from_user(&pa_val, (struct pa_val_t *)arg, sizeof(struct pa_val_t))) {
                    return -EFAULT;
                }
                pr_info("Got PA: %lx, Val: %d\n", pa_val.pa, pa_val.val);

                // Write value to physical address
                // if (!phys_addr_valid(pa_val.pa)) {
                //     return -EFAULT; // Physical address is not valid
                // }

                uint8_t *ptr = phys_to_virt(pa_val.pa);
                pr_info("phys-virt difference: %lu\n", (uintptr_t)ptr -(uintptr_t)pa_val.pa);
                if (!ptr) {
                    return -EFAULT; // Failed to map physical address
                }

                *ptr = pa_val.val; // Write value to physical address
            }
            break;
    
        
        case IOCTL_GET_VALUE_AT_PHYSICAL_ADDR:
            pr_info("<%s>: IOCTL_GET_VALUE_AT_PHYSICAL_ADDR\n", DEVICE_NAME);
            {
                struct pa_val_t pa_val;
                memset(&pa_val, 0, sizeof(struct pa_val_t));
                if (copy_from_user(&pa_val, (struct pa_val_t *)arg, sizeof(struct pa_val_t))) {
                    return -EFAULT;
                }
                pr_info("Got PA: %lx, Val: %d\n", pa_val.pa, pa_val.val);
                
                uint8_t *ptr = phys_to_virt(pa_val.pa);
                pr_info("Val (val must be the one set in prev ioctl) = %c\n", *ptr);

                pa_val.val = *ptr;
                if (copy_to_user((struct pa_val_t *)arg, &pa_val, sizeof(struct pa_val_t))) {
                    pr_warn("<%s>: Failed copy-to-user\n", DEVICE_NAME);
                    return -EFAULT;
                }
            }
            break;
    }
    return 0;
}

/* Device Initialisation */
static int __init spockv2p_init(void) {
    dev_t devno = 0;
    int ret = 0;

    /*
     * Begin device registration
    */
    ret = alloc_chrdev_region(&devno, spockv2p_minor, DEVICE_COUNT, device_name);
    spockv2p_major = MAJOR(devno);

    // Init device data
    ret = spockv2p_device_initdata(&device_data);
    if (ret < 0) {
        pr_warn("spockv2p: Error %d adding spockv2p device.\n", ret);
        goto fail;
    }

    pr_info("spockv2p: Module loaded\n");
    return 0;

fail:
    return ret;
    
}

/* Device Removal */
static void __exit spockv2p_exit(void) {
    dev_t devno = MKDEV(spockv2p_major, spockv2p_minor);

    cdev_del(&device_data.cdev);
    unregister_chrdev_region(devno, DEVICE_COUNT);
    pr_info("spockv2p: Module Unloaded.\n");
}

module_init(spockv2p_init);
module_exit(spockv2p_exit);