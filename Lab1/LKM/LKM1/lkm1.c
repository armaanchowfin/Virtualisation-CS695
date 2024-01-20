/*
Write a kernel module to list all processes in a running or runnable state. 
Print their pid. Name the source file as lkm1.c
*/

//#include <linux/sched.h> // task_struct
#include <linux/module.h> // module macros
#include <linux/printk.h> // printk aliases
#include <linux/sched/signal.h> // convenience macros
#include <linux/sched/task.h> // init_task

MODULE_DESCRIPTION("LKM1");
MODULE_AUTHOR("Armaan Chowfin");
MODULE_LICENSE("GPL");

static int __init lkm1_init(void){
    /* 
     * Traverse next pointers of the anchor until init is reached.
     * This assumes the task_structs are stored in a circular task list
     * As per https://elixir.bootlin.com/linux/v6.1/source/include/linux/list.h#L14
     * for_each_process(p), defined in https://elixir.bootlin.com/linux/v6.1/source/include/linux/sched/signal.h#L646
     * Does not print pid 0 i.e. wrapper.
     */

    struct task_struct *p = NULL;
    for_each_process(p)
    {
        if task_is_running(p)
            pr_info("[%d]: %s: %d\n", p->pid, p->comm, p->__state);
    }
    return 0;
    
    #if 0
    /*Get anchor i.e. init process */
    struct task_struct *task = &init_task; // in sched/task.h
    pr_debug("%p\n", task);

    /*
     * Run a raw do-while loop intead of the provided convenience macros.
     * next_task() is defined in https://elixir.bootlin.com/linux/v6.1/source/include/linux/sched/signal.h#L647
     * init_task is a global extern var in https://elixir.bootlin.com/linux/v6.1/source/include/linux/sched/task.h#L53
     * Can include it in current file.
     * 
    */
    do{
        pr_info("[%d]: %s: %d\n", task->pid, task->comm, task->__state);

    } while (next_task(task) != &init_task);

    return 0;
    #endif
     
}

static void __exit lkm1_exit(void){
    pr_info("LKM1 Complete");
}

module_init(lkm1_init);
module_exit(lkm1_exit);