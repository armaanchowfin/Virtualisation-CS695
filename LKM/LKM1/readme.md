This readme contains implementation details and considerations for lkm1.c. 
Solutions only.
Task: 
```
Write a kernel module to list all processes in a running or runnable state. 
Print their pid. Name the source file as lkm1.c
```
The task requires our module to access the process representations i.e `task_struct`s for each process in the kernel task-list and list the pids of each process whose `state` attribute evaluates to `TASK_RUNNING`.

## Plan: Traverse the kernel task-list.
    Linux `task_struct`s are stored in a doubly-linked circular list as per (https://developer.ibm.com/tutorials/l-linux-process-management/), 
    with the `init_task` as the first process, statically created, forming the anchor point.
    - `struct task_struct init_task` is defined in /init/init_task.c. To include this variable in a kernel module,simply include the header file containining its declaration.
> The clean, reliable way to declare and define global variables is to use a header file to contain an extern declaration of the variable.
The header is included by the one source file that defines the variable and by all the source files that reference the variable

- `extern struct task_struct init_task` is declared in `/include/linux/sched/task.h`. The extern keyword in `task.h` allows the current header (and subsequent headers that #include it) to use the init_task variable. 



### 1: Find out what attributes of `task_struct` correspond to our goal, and how to access them.
Q1. What does it mean to #include a path?
- It means that on compilation, the preprocessor recursively unpacks the included file. Syntax differs based on whether the file is a system header or 
see https://gcc.gnu.org/onlinedocs/cpp/Invocation.html

// is_global_init(task) : checks if task is the init task i.e. pid 1.
- This is a mistake. The anchor task is pid0 i.e. wrapper/sched task.

Approach 1: Traverse task_structs using list_head->next i.e. tasks->next
Approach 2: Traverse processes using kernel-defined macros such as for_each_process or next_process().
- Why is next_task() not just p->tasks->next?? why so many wrappers?

Build Errors:
![errors](image.png)
--> #include <linux/sched.h> does not recursively add linux/sched/task.h. Whats going on??
-> sched.h DOES not include task.h !!! Thus it must be added independently.

2. Kernel hangs if I run a raw do-while loop with next_task instead of using the for_each_process macro.

Q2. Why arent macros included?
-> 

Turns out, you have to use convenience macros, otherwise you get strange errors like missing attributes

`task_struct` in detail
Q3. 

