Usage: `sudo insmod target_pid=<target pid> target_virtual_address=<target va>`

Q1. What kinds of errors can the passed in arguments raise?

Learnings
1. Linux Memory Mapping API: mm_types.h, page.h

Maple Trees
https://lwn.net/Articles/845507/
An alternative to the current Linux Virtual Memory Area (VMA) implementation with Radix-tree -> RB-Trees.
Q. All Virtual Memory implementations have to deal with lock contention. What is RCU?

Q. What does it mean to store an address-range in a tree? What's the most primitive way to store an address-range? (probably tuple)

Process:
Read through the lwn docs on maple trees, realised i needed an understanding of the linux suubsystems first. Hence https://linux-kernel-labs.github.io/refs/heads/master/lectures/intro.html
"Spin-locks" : To implement Symmetric Multi-processing (on multicore systems, kernel can run on any core instead of being fixed to a single core)
Q. What does it mean to "run" a Kernel? 

Random: 
David Miller: Networking subsystem maintainer linux. http://vger.kernel.org/~davem/skb.html : Socket buffer.

Devices and drivers: Access to devices is requested by user-space. /dev/devices contains the entry tag (file node) that is seen by user-space, but in isolation has no meaning besides being a string. The driver that we implement associates this file node with the linux filesystem and implements the "meat" of the device. 
The `struct cdev` maps (interfaces) device to kernel (device to file-system via driver.)
Requests from user-space are then handled by specific driver functions.
While working with files, a driver must implement all the functions defined in `struct file_operations` in `linux/fs.h`. The struct contains function pointers to the respective operation implementation.
- Here, "device" is of type file.
Q. Where are these function pointers set?