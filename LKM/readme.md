# LKM: Implementation of linux kernel modules of varying complexity.
[LKMPG](https://sysprog21.github.io/lkmpg/)

Ideas involved in LKMs
Refer: [Introduction to Loadable Kernel Modules, TLDP](https://tldp.org/HOWTO/Module-HOWTO/x73.html)
## 1. What is a Module
- Description in isolation,
A LKM is a code segment capable of being dynamically "loaded" and "unloaded" into the kernel to provide an additional functionality without recompiling the entire kernel. Monolithic kernel implementations are usually tightly coupled - That is, it is one, single, large program where all the functional components of the kernel have access to all of its internal data structures and routines. 
The introduction of the LKM package allows a developer to independently develop a particular kernel functionality without having to make more fundamental changes to the kernel image itself. The LKM package contains a set of programs for loading, inserting, and removing kernel modules for Linux such as 
`modprobe`,
`depmod`,
`insmod`,
`rmmod`,
`lsmod`
etc., installed via `sudo apt-get install build-essential kmod` on Ubuntu/Debian/Linux

- What problem does on-demand module-loading solve?
cd
- Who is allowed to use a module? (how does kernel implement this restriction?)

- What services can a module provide to its user




## 2. Creating a Module 
- What are the physical requirements to create a LKM 
A LKM requires at the minimum a module.c file that contains module logic, and a Makefile.
--> `module.c`, `Makefile`, `kbuild` files are the inputs to the `kbuild` toolchain provided by the `kmod` package.
[module.c structure](null)
The module.c file requires `__init` and `__exit` functions.
Q1. Are there restrictions on the signatures of these functions?

### Modules and Command-Line arguments. 
https://girishjoshi.io/post/linux-kernel-module-parameter-permissions-access-rights/
The `module_param()` macro, found in `linux/moduleparam.h` provides a mechanism for module source to take CLI at runtime.
The final argument is the permissions bits, for exposing parameters in `sysfs`(if non-zero) at a later stage

[Makefile structure](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/makefiles.rst)
Recursive use of make means using make as a command in a makefile. This technique is useful when you want separate makefiles for various subsystems that compose a larger system. For example, suppose you have a sub-directory subdir which has its own makefile, and you would like the containing directory’s makefile to run make on the sub-directory. You can do it by writing this:
```
subsystem:
    $(MAKE) -C subdir
```

[Compiling a Module](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/modules.rst)
In newer versions of the kernel, kbuild will first look for a file named "Kbuild," and only if that is not found, will it then look for a makefile.

The LKM toolchain provides a uniform method to create "valid" kernel modules. Run `sudo apt-get install build-essential kmod` on ubuntu/debian/GNU Linux to install.

- What does a valid LKM constitute
A valid LKM is one that adheres to the syntax and semantics of the `kmod` package. This defition results from the below claim:
Claim: "Following the recommended build process will always lead to a valid LKM"
[What is kmod](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch11.html)
--> what guarantees does the `kmod` build process provide to an LKM developer? (outcomes)
--> how do these guarantees imply module validity.
----> What are the more fundamental considerations taken by the `kmod` package that enable the creation of a "valid" LKM: (This requires an understanding of the `kmod` development process)


`lsmod`, `rmmod` and `insmod`:
These are three bash commands associated with working with modules.
## 3. Loading/Unloading a Module and Usage 
Modules cannot directly print to the screen like printf() can, but they can log information and warnings that are eventually displayed on the
screen, specifically within a console.  A module always begins with either the init_module function or a function designated by the module_init call. All modules conclude by invoking either cleanup_module or a function specified through the module_exitcall. This serves as the module’s exit function,
reversing the actions of the entry function by unregistering the previously registered functionalities.

>modules are object files whose symbols get resolved upon running insmod or modprobe.
Q. How does this relate to regular object files?
Q1. Where must a kernel module be stored?

Q2. Who can trigger LKM load/unload/run, How is it triggered
Two mechanisms: Manual loading via `insmod` and demand-loading via the `kerneld` daemon.
`kerneld` is a user process, which runs the kerneld program from the modutils package. kerneld sets up an IPC message channel with the kernel. When the kernel needs an LKM, it sends a message on that channel to kerneld and kerneld runs modprobe to load the LKM, then sends a message back to the kernel to say that it is done.
Automatic loading is usually not worth it. Therefore, bash scripts load all custom kernels that would be required by the subsystem.

q3. Does a module "run" like a process does, what is the procedure to load/use/unload a LKM? (is it load -> return -> unload?)
--> Each action associated with kernel modules (creation, loading, usage) is enabled by `kmod`.
The `kmod` package is a set of tools that handle common tasks associaed with Linux kernel modules like insert, remove, list, check properties, ?resolving dependencies and aliases?
--> kmod was initially implemented as a separate, standalone kernel process that handled module loading requests, but it has long since been simplified by not requiring the separate process context. To use kmod, you must include <linux/kmod.h> in your driver source.

Q4. Are loaded modules scheduled like processes? `insmod`

Q5. What is a module config file?
`'kernel/module/KConfig`