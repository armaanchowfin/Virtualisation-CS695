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
#include <linux/pid.h>

MODULE_DESCRIPTION("LKM2");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_LICENSE("GPL");

static int p_id = -1;

module_param(p_id, int, 0); 
MODULE_PARM_DESC(p_id, "pid of desired process");

static void get_child_processes(struct task_struct *task) {
    struct list_head *position;

    list_for_each(position, &(task->children)) {
        struct task_struct *child_task = list_entry(position, struct task_struct, sibling);

        pr_info("[%d]:%s:[%d]\n", child_task->pid, child_task->comm, child_task->__state);

        get_child_processes(child_task);
    }
}

static int __init lkm2_init(void) {

    if (p_id < 0) {
        pr_err("Need to supply command-line argument: p_id=<pid>\n");
        return -EINVAL;
    }

    struct pid *base_pid = find_get_pid(p_id);

    struct task_struct *base_task = pid_task(base_pid, PIDTYPE_PID);
    if (!base_pid) {
        pr_err("Task struct for PID %d not found\n", p_id);
        return -EINVAL;
    }

    pr_info("[%d]:%s: [%d]\n", base_task->pid, base_task->comm, base_task->__state);

    get_child_processes(base_task);

    return 0;
}


static void __exit lkm2_exit(void){

    pr_info("LMK2 Completed\n");

}

module_init(lkm2_init);
module_exit(lkm2_exit);