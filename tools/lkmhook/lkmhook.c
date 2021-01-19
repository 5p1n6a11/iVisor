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

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

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

    return 0;
}

static void
lkmhook_cleanup(void)
{
    pr_info("cleanup");
}

module_init(lkmhook_init);
module_exit(lkmhook_cleanup);
