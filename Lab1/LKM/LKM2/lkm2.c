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

static int p_id = 0;

module_param(p_id, int, 0); 
MODULE_PARM_DESC(p_id, "pid of desired process");


/*
 * /linux/list.h provides the Doubly-Linked Circular list API.
 * list_entry: gets surrounding struct from its list_head
*/
static void get_child_processes(struct task_struct *base_task){
    list_for_each_entry(base_task, &base_task->children, sibling){
        pr_info("[%d]:[%s]\n", child_task->pid, child_task->comm);
        get_child_processes(child_task);
    }

    // for(struct list_head *child_list_ent = child_head->next;
    //     !list_is_head(child_head, child_list_ent);     
    //     child_list_ent = child_list_ent->next           
    //     ){
    //         /**
    //           * list_entry - get the struct for this entry
    //           * list_entry(ptr, type, member)
    //           * @ptr:	the &struct list_head pointer.
    //           * @type:	the type of the struct this is embedded in.
    //           * @member:	the name of the list_head within the struct.
    //         */
    //         struct task_struct *child_task = &list_entry(&child_list_ent, struct task_struct, sibling)
    //         pr_info("[%d]:[%s]\n", child_task->pid, child_task->comm);
            
    //     }

}

static int __init lkm2_init(void){
    //struct pid from pid integer.
    struct pid *base_pid = find_get_pid(p_id); 

    // Gives a reference to the task_struct of the process at pid p_id
    struct task_struct *base_task = pid_task(base_pid, PIDTYPE_PID); 

    get_child_processes(base_task);
    
    return 0;

}

static void __exit lkm2_exit(void){

    pr_info("LMK2 Completed");

}

module_init(lkm2_init);
module_exit(lkm2_exit);