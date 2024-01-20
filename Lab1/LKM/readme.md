# LKM: Implementation of linux kernel modules of varying complexity.
[LKMPG](https://sysprog21.github.io/lkmpg/)

Ideas involved in LKMs
## 1. What is a Module
- Description in isolation, 
-->LKM: A code segment capable of being dynamically "loaded" and "unloaded" into the kernel to provide an additional functionality without recompiling the entire kernel.
Monolithic kernel implementations are usually tightly coupled (verify this). The introduction of the LKM toolchain/functionality allows a developer to independently develop a particular kernel functionality without having to make more fundamental changes to the kernel itself.
LKMs functionality is requested by the kernel during 
Linux 2.0 onwards introduced this idea of kernel modularity via 

- What problem does on-demand module-loading solve?
cd
- Who is allowed to use a module? (how does kernel implement this restriction?)

- What services can a module provide to its user




## 2. Creating a Module 
- What are the physical requirements to create a LKM 
--> `module.c`, `Makefile`, `kbuild` files are the inputs to the `kbuild` toolchain provided by the `kmod` package.
[module.c structure](null)
The module.c file requires `__init` and `__exit` functions.
Q1. Are there restrictions on the signatures of these functions?

### Modules and Command-Line arguments. 

[Makefile structure](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/makefiles.rst)

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




## 3. Loading/Unloading a Module and Usage 
- Where must a kernel module be stored?

- Who can trigger LKM load/unload/run, How is it triggered

- Does a module "run" like a process does, what is the procedure to load/use/unload a LKM? (is it load -> return -> unload?)
--> Each action associated with kernel modules (creation, loading, usage) is enabled by `kmod`.
The `kmod` package is a set of tools that handle common tasks associaed with Linux kernel modules like insert, remove, list, check properties, ?resolving dependencies and aliases?
--> kmod was initially implemented as a separate, standalone kernel process that handled module loading requests, but it has long since been simplified by not requiring the separate process context. To use kmod, you must include <linux/kmod.h> in your driver source.