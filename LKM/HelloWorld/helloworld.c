/*
* helloworld.c - simplest module 
Call make kbuild to build helloworld.ko
*/

#include <linux/module.h>
#include <linux/printk.h> /* for pr_info, pr_debug */
#include <linux/init.h> /* module_init(), module_exit() macros*/

MODULE_DESCRIPTION("Hello World");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_LICENSE("GPL");

static int helloworld_init(void){
    pr_info("Hello World\n");
    return 0;
}

static void helloworld_exit(void){
    pr_info("Goodbye World\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);