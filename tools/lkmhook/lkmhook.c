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
thv_sys_init_module(void __user *umod, unsigned long len, const char __user *uargs)
{
    long orig;
    struct module *mod = NULL;
    pr_info("call htv_sys_init_module\n");

    orig = orig_sys_init_module(umod, len, uargs);

    return orig;
}

asmlinkage long
thv_sys_finit_module(int fd, const char __user *uargs, int flags)
{
    long orig;
    struct module *mod = NULL;
    pr_info("call htv_sys_finit_module\n");

    orig = orig_sys_finit_module(fd, uargs, flags);

    return orig;
}

static void
change_page_attr_to_rw(pte_t *pte)
{
    set_pte_atomic(pte, pte_mkwrite(*pte));
}

static void
change_page_attr_to_ro(pte_t *pte)
{
    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

static void
replace_system_call(void *new_init, void *new_finit)
{
    unsigned int level = 0;
    pte_t *pte;

    pte = lookup_address((unsigned long) syscall_table, &level);
    /* Need to set r/w to a page which syscall_table is in. */
    change_page_attr_to_rw(pte);
    syscall_table[__NR_init_module] = new_init;
    syscall_table[__NR_finit_module] = new_finit;
    /* set back to read only */
    change_page_attr_to_ro(pte);
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

    replace_system_call(thv_sys_init_module, thv_sys_finit_module);
    pr_info("system call replaced\n");

    return 0;
}

static void
lkmhook_cleanup(void)
{
    pr_info("cleanup");
    if (orig_sys_init_module && orig_sys_finit_module)
        replace_system_call(orig_sys_init_module, orig_sys_finit_module);
}

module_init(lkmhook_init);
module_exit(lkmhook_cleanup);
