#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x8a35b432, "sme_me_mask" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x72d79d83, "pgdir_shift" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0xef692a70, "cdev_add" },
	{ 0x1d19f77b, "physical_mask" },
	{ 0x6d0419bd, "current_task" },
	{ 0xdad13544, "ptrs_per_p4d" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xefd100cf, "pv_ops" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xd1d6e0b7, "boot_cpu_data" },
	{ 0x8b375d72, "cdev_init" },
	{ 0x37b5f2b0, "cdev_del" },
	{ 0x517f61ad, "module_layout" },
};

MODULE_INFO(depends, "");

