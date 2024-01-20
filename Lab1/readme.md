# [Assignment 1 Handout](https://docs.google.com/document/d/e/2PACX-1vQ5u_tBmDgtbHPr4wtyCt-2HWnreknpARu4ycR5gtFjRJqAxqwYBuWXfxldCNQNsFABBqAFT0qBMgZj/pub)
This Readme describes the background information required for the completion of tasks outlined in Assignment 1, CS695: Linux Internals Understanding and eXploration

## 1. LKM: Linux Kernel Modules
This assignment calls for the implementation of custom kernel functionalities as loadable modules.
There are 4 such functionalities, and all of them seem to require manipulation of the structures
that provide the Linux process abstraction. Secondly, all problems require printing data/results
in some form, hence it is also important to understand alternative(s) to the regular, user-space
buffered I/O libraries.
tags: kernel message logging, linux process abstraction.
### 1.1 Message logging from the kernel
-----
Refer: [Logging via `printk`](https://www.kernel.org/doc/html/next/core-api/printk-basics.html):
Implemented via `printk(log_level "format string", *args)`.
Kernel logs are by default printed to the kernel log buffer, which is a ring buffer exported to userspace through `/dev/kmsg`. Linux provides the `dmesg` shell command (implemented as a binary - search via `type -a dmesg`) that returns the entries logged in the kernel log buffer. There are 11 logging levels, specified via log_level `KERN_<logType>`. 

Q1. What differs LKMs from the binaries in usr/bin? A: The "on-demand loading", i.e. no recompilation required for a new functionality.
Q2. Is writing to file `dev/kmsg` specified via an implicit, default file descriptor?
Q3. Is kernel log buffer an intermediate storage location before storing logs to `dev/kmsg`?

The log_level parameter assigns a priority to a particular message. A message is stored in the log on the criterion that its log_level < `console_loglevel`(a kernel variable) -i.e. only higher priority messages are printed.
The current `console_loglevel` is stored in `/proc/sys/kernel/printk` and can be queried via regular shell commands. They are listed as a row of 4 ints:
CURRENT  DEFAULT  MINIMUM  BOOT-TIME-DEFAULT. The logging filter can be specified either by modifying the `/proc/sys/kernel/printk` file directly, or
by using the options available to the `dmesg` command.
`dmesg -n 5` sets the console_loglevel to print KERN_WARNING (4) or more severe messages to console.
`echo 8 > /proc/sys/kernel/printk`. This modifies the first entry i.e CURRENT. Note that this can only be performed if logged in as root via via `su`.

Linux kernel also provides alias functions `pr_*()` for the various `printk` logging-levels. Besides being more concise than the equivalent printk() calls, they can use a common definition for the format string through the `pr_fmt()` macro. This macro can be redefined for individual programs using the `#define` directive (before any `#include`s). Example: `#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__` would prefix every pr_*() message in that file with the module and function name that originated the message. There are 2 special logging macros : `pr_debug()` and `pr_devel()`. These are conditionally compiled out unless `DEBUG` (or also `CONFIG_DYNAMIC_DEBUG` in the case of pr_debug()) is defined.

Q4. How to define/set these options in the linux kernel? A: Refer https://stackoverflow.com/questions/11631968/how-to-enable-dynamic-debugging-in-the-linux-kernel 

### 1.2 Linux Process Abstraction
----
Refer: [Process representation](https://developer.ibm.com/tutorials/l-linux-process-management/)

In Linux, a process is represented by the large struct `task_struct` that contains all necessary data to represent a process, including data for process-related optimiations and relationships to other processes. It is found in `/linux/include/linux/sched.h.`. The reason for its large size (around 1.7kb on a 32bit machine) is required to represent its "multi-functional" nature, i.e. that a process is a common abstraction relevant to most kernel contexts and mechanisms. `task_struct` contains various attributes, the details of which are explored in the ... article [].
One important attribute is the `struct list_head tasks;`. This struct provides the linked-list structure of tasks (i.e. processes) in the kernel. `list_head` contains prev and next pointers that point to the prev and next tasks respectively. Linux processed are managed in two ways:  In a hash-table hashed by `pid` value, and as a circular doubly-linked list. The `tasks` attribute of each `task_struct` provides the framework that maintains the doubly-linked circular list in memory. In most cases, processes are dynamically created and represented by a dynamically allocated `task_struct`. The exception is the `init_task` process, i.e. pid0. `init_task` is statically allocated at boot time, and forms the anchor point of the DLCL. The state of all processes in the kernel can therefore be obtained by traversing this list, starting from the head. Note that the `task_struct` list is not available to user-space programs, therefore this information can only be obtained by implementing a dynamically-loadable LKM to provide this functionality to a kernel.

Q1. Is there truly no alternative to creating a LKM (via syscalls, etc.)? 
A: The ultimate goal is to access specific areas of OS memory from userspace. Explore these ideas.

The underlying mechanism for process creation is common to both user-space and kernel-space tasks - reducing to a call to `do_fork()`. Even Kernel threads are created by a call to `kernel_thread()`, which itself calls `do_fork()`. These functions are implemented in `/linux/arch/i386/kernel/process.c`. For user-space programs, a call to `fork()` is a wrapper to the syscall `sys_fork()`, which, (you guessed it) calls `do_fork()`. Note that there is a pattern to this seperation of functionality afforded by syscalls. In general, linux `sys_*` functions (i.e. system calls) perform the initial activities (error checking, user space activities - elaborate), while the core work involved in process-creation is delegated to another `do_*` function. For processes, `do_fork()` is implemented in `/linux/kernel/fork.c` along with the partner function, `copy_process()`.

Q2. Are tasks added to the list in order-of-creation?
A: Yes. `do_call()` performs a few preparatory fork tasks (such as alloc new pid) and passes the current process context (flags, stack, registers, parent process, and newly allocated PID) to `copy_process()`, thereby initiating the cloning procedure to create a new process. `copy_process()` performs some security checks via the `LSM- Linux Security Module` such as checking if the process is allowed to fork. Eventually, it calls `dup_task_struct` function found in `/linux/kernel/fork.c`, which allocates a new `task_struct` and copies the current process's descriptors into it. Control eventually returns to the `copy_process()` function that initialised the newly created `task_struct`. This is where the new task gets added to the kernel task-list. 

A sequence of copy functions is then invoked that copy individual aspects of the process, from copying open file descriptors (copy_files), copying signal information (copy_sighand and copy_signal), copying process memory (copy_mm), and finally copying the thread (copy_thread). Eventually, the process is allocated a CPU based on `cpus_allowed` inherited from the parent `task_struct`. Next, control returns to `do_call()`, which "runs" the newly created process via `wake_up_new_task()` that places the new process in a run queue for scheduling. Finally, `do_fork()` returns, and passes the newly created pid (i.e. child pid) to the calling process.

Q3a. Are Kernel Threads also stored as `task_struct` objects?
A: No. Kernel Threads are stored as a `thread_struct` attribute in the `task_struct` object, thereby linking threads to a process.
Q3b. Are threads associated with a single process? 
Q3c. What is the thread stack?

## 2. IOCTL: ioctl drivers
## 3. PROC:

