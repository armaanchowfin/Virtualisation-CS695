/*
Write a kernel module that takes process ID as input, 
and for each of its child processes, print their pid and process state. 
Name the source file as lkm2.c
*/
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/module.h> 
#include <linux/moduleparam.h> 
#include <linux/printk.h> // printk aliases

static int mypid = 0;

module_param(mypid, int, 0); 
MODULE_PARM_DESC(mypid, "pid of desired process");

static int __init lkm2_init(void){

}

static void __exit lkm1_exit(void){

}

module_init(lkm1_init);
module_exit(lkm1_exit);