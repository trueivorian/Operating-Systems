#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x13fa4f04, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x7b679b16, __VMLINUX_SYMBOL_STR(nf_unregister_hook) },
	{ 0x56857cc0, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0x4469dd25, __VMLINUX_SYMBOL_STR(__init_rwsem) },
	{ 0x5ead1ca5, __VMLINUX_SYMBOL_STR(nf_register_hook) },
	{ 0x9ad8ee1, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x71f745e7, __VMLINUX_SYMBOL_STR(tcp_done) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0x13e5d2fe, __VMLINUX_SYMBOL_STR(mmput) },
	{ 0x5536a4b0, __VMLINUX_SYMBOL_STR(get_task_mm) },
	{ 0x4629334c, __VMLINUX_SYMBOL_STR(__preempt_count) },
	{ 0xcc66a3d2, __VMLINUX_SYMBOL_STR(path_put) },
	{ 0x346e7182, __VMLINUX_SYMBOL_STR(d_path) },
	{ 0x4718f2fb, __VMLINUX_SYMBOL_STR(kern_path) },
	{ 0xb81960ca, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x6cacead7, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x49ed4ea9, __VMLINUX_SYMBOL_STR(skb_copy_bits) },
	{ 0xb4a1fdac, __VMLINUX_SYMBOL_STR(up_write) },
	{ 0x2cef066, __VMLINUX_SYMBOL_STR(down_write) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x353c50a8, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xb0ebad1f, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x9e9276e7, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0xc7599b88, __VMLINUX_SYMBOL_STR(module_put) },
	{ 0x81401a80, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x666b16dc, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x60bdacce, __VMLINUX_SYMBOL_STR(up_read) },
	{ 0x699d5bf1, __VMLINUX_SYMBOL_STR(down_read) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "9739DD9DA10B854E30F46C3");
