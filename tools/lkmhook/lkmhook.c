#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <asm/pgtable.h>

MODULE_DESCRIPTION("guest lkm hook for thv");
MODULE_AUTHOR("u2i");
MODULE_LICENSE("GPL");

unsigned long (*kallsyms_lookup_name_p)(const char *name);
static void **syscall_table;

asmlinkage long (*orig_sys_init_module)(void __user *umod, unsigned long len, const char __user *uargs);
asmlinkage long (*orig_sys_finit_module)(int fd, const char __user *args, int flags);

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

static void
save_original_syscall(void)
{
    orig_sys_init_module = syscall_table[__NR_init_module];
    orig_sys_finit_module = syscall_table[__NR_finit_module];
}

asmlinkage long
htv_sys_init_module(void __user *umod, unsigned long len, const char __user *uargs)
{
    long orig;
    struct module *mod = NULL;
    pr_info("call htv_sys_init_module\n");

    orig = orig_sys_init_module(umod, len, uargs);

    return orig;
}

asmlinkage long
htv_sys_finit_module(int fd, const char __user *uargs, int flags)
{
    long orig;
    struct module *mod = NULL;
    pr_info("call htv_sys_finit_module\n");

    orig = orig_sys_finit_module(fd, uargs, flags);

    return orig;
}

static int
lkmhook_init(void)
{
    register_kprobe(&kp);
    pr_info("kp.addr: %px\n", kp.addr);
    kallsyms_lookup_name_p = (void *) kp.addr;
    pr_info("kallsyms_lookup_name address is 0x%px\n", kallsyms_lookup_name_p);
    unregister_kprobe(&kp);

    syscall_table = (void *)kallsyms_lookup_name_p("sys_call_table");
    pr_info("sys_call_table address is 0x%px\n", syscall_table);
    save_original_syscall();
    pr_info("original sys_init_module's address is %px\n", orig_sys_init_module);
    pr_info("original sys_finit_module's address is %px\n", orig_sys_finit_module);

    return 0;
}

static void
lkmhook_cleanup(void)
{
    pr_info("cleanup");
}

module_init(lkmhook_init);
module_exit(lkmhook_cleanup);
