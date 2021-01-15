#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

MODULE_DESCRIPTION("guest lkm hook");
MODULE_AUTHOR("u2i");
MODULE_LICENSE("GPL");

static void **syscall_table;
static char *target = "blahblah";
module_param(target, charp, S_IRUGO);

asmlinkage long (*orig_sys_init_module)(void __user *umod, unsigned long len, const char __user *uargs);
asmlinkage long (*orig_sys_finit_module)(int fd, const char __user *uargs, int flags);

void
call_vmm_call_function(call_vmm_function_t *function,
                       call_vmm_arg_t *arg, call_vmm_ret_t *ret)
{
    struct call_vmm_call_function_sub_data data;
    data.function = function;
    data.arg = arg;
    data.ret = ret;
    pr_info("vmcall %d(arg=0x%lx)\n", data.function->vmmcall_number, data.arg->rbx);
    switch (data.function->vmmcall_type) {
    case VMMCALL_TYPE_VMCALL:
        asm volatile ("vmcall"
                       : "=a" (data.ret->rax), "=b" (data.ret->rbx),
                         "=c" (data.ret->rcx), "=d" (data.ret->rdx),
                         "=S" (data.ret->rsi), "=D" (data.ret->rdi)
                       : "a" (data.function->vmcall_number),
                         "b" (data.arg->rbx),
                         "c" (data.arg->rcx), "d" (data.arg->rdx),
                         "S" (data.arg->rsi), "D" (data.arg->rdi)
                       : "memory");
        break;
    case VMMCALL_TYPE_VMMCALL:
        asm volatile ("vmmcall"
                       : "=a" (data.ret->rax), "=b" (data.ret->rbx),
                         "=c" (data.ret->rcx), "=d" (data.ret->rdx),
                         "=S" (data.ret->rsi), "=D" (data.ret->rdi)
                       : "a" (data.function->vmmcall_number),
                         "b" (data.arg->rbx),
                         "c" (data.arg->rcx), "d" (data.arg->rdx),
                         "S" (data.arg->rsi), "D" (data.arg->rdi)
                       : "memory");
        break;
    }
}

/* Get a entry number of specified vmmvall */
void
vmmcall_get_function(const char *vmmcall, call_vmm_function_t *res)
{
    call_vmm_function_t gf;
    call_vmm_arg_t gf_a;
    call_vmm_ret_t gf_r;

    gf.vmmcall_number = GET_VMMCALL_NUMBER;
    gf.vmmcall_type = VMMCALL_TYPE_VMCALL;
    gf_a.rbx = (intptr_t)vmmcall;
    pr_info("vmmcall_string:%p, 0x%lx, %s\n", vmmcall, gf_a.rbx, vmmcall);
    call_vmm_call_function(&gf, &gf_a, &gf_r);

    /* RAX = vmmcall number */
    res->vmmcall_number = (int)gf_r.rax;
    res->vmmcall_type = VMMCALL_TYPE_VMCALL;    // XXX: should get this value too.
}

static void
vmcall_lkmhook(void *mod)
{
    call_vmm_function_t lkmf;
    call_vmm_arg_t lkm_a;
    call_vmm_ret_t lkm_r;
    char lkmhook[] = "lkmhook";

    vmmcall_get_function(lkmhook, &lkmf);
    pr_info("lkmhook number=%d\n", lkmf.vmmcall_number);

    lkm_a.rbx = (intptr_t)mod;
    call_vmm_call_function(&lkmf, &lkm_a, &lkm_r);
}

static struct module*
get_module(const char *moduname)
{
    struct module *mod = NULL;
    if (mutex_lock_interruptible(&module_mutex) != 0) {
        goto mutex_fail;
    }

    mod = find_module(modname);

    mutex_unlock(&module_mutex);

mutex_fail:
    pr_info("get_module(%s) = %p\n", modname, mod);
    return mod;
}

asmlinakge long
thv_sys_finit_module(int fd, const char __user *uargs, int flags)
{
    long orig;
    struct module *mod = NULL;
    pr_info("call thv_sys_finit_module\n");

    orig = orig_sys_finit_module(fd, uargs, flags);

    mod = get_module(taget);

    if (mod)
        vmcall_lkmhook(mod->module_core);

    return orig;
}

static struct kprobe kp = {
    .symbol_name = "sys_call_table"
};

static int
lkmhook_init(void)
{
    register_kprobe(&kp);
    syscall_table = (void *) kp.addr;
    unregister_kprobe(&kp);

    pr_info("sys_call_table address is 0x%p\n", syscall_table);
    save_original_syscall();
    pr_info("original sys_init_module's address is %p\n", orig_sys_init_module);
    pr_info("original sys_finit_module's address is %p\n", orig_sys_finit_module);

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
    if (mod_area_phys)
        kfree(mod_area_phys);
}

module_init(lkmhook_init);
module_exit(lkmhook_cleanup);
