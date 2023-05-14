#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>

#include "a3khook/a3khook.h"

void my_hook_before_fn(size_t *args)
{
    printk(KERN_ERR "[a3khook:] my_hook_before_fn() is called!");
}

size_t my_hook_after_fn(size_t orig_ret, size_t *args)
{
    printk(KERN_ERR "[a3khook:] my_hook_after_fn() is called!");
    return orig_ret;
}

struct hook_info my_hook_info = {
    .hook_before = my_hook_before_fn,
    .hook_after = my_hook_after_fn,
    .orig_fn = (void*) commit_creds, // just an example there :)
    .call_orig = 1,
};

size_t template_hook_dst(size_t arg0, size_t arg1, size_t arg2, size_t arg3, 
                         size_t arg4, size_t arg5)
{
    size_t args[6];

    args[0] = arg0;
    args[1] = arg1;
    args[2] = arg2;
    args[3] = arg3;
    args[4] = arg4;
    args[5] = arg5;

    return a3khook_executor(&my_hook_info, args);
}

static int __init kernel_module_init(void)
{
    printk(KERN_INFO "Hello the Linux kernel!");

    /* this is just a test */
    a3khook_add_new_hook(&my_hook_info, template_hook_dst);
    return 0;
}

static void __exit kernel_module_exit(void)
{
    printk(KERN_INFO "Good bye the Linux kernel!");
}

module_init(kernel_module_init);
module_exit(kernel_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("arttnba3");
