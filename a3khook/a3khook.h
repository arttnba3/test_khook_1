/**
 * This is the header file for hooking a function using my own technique:
 *      - dynamic hooking and patching
 * 
 * The technique keeps patching the code segment dynamicly,which doesn't require
 * to identify the instructions on the location of target function.
 * 
 * Copyright (c) 2023 arttnba3
 * Author: arttnba3 <arttnba@gmail.com>
 */

#ifndef _A3KHOOK_H_
#define _A3KHOOK_H_

#include <linux/kernel.h>
#include <linux/mutex.h>

#define HOOK_BUF_SZ 0x30

struct hook_info {
    struct mutex lock;
    char new_insn[HOOK_BUF_SZ];
    char orig_insn[HOOK_BUF_SZ];
    void (*hook_before) (size_t *args);
    size_t (*orig_fn) (size_t, size_t, size_t, size_t, size_t, size_t);
    size_t (*hook_after) (size_t orig_ret, size_t *args);
    int call_orig;
};

extern void a3khook_add_new_hook(struct hook_info *info, void *new_dst);
extern size_t a3khook_executor(struct hook_info *info, size_t *args);

/**
 * For hooking a function, you need to:
 * 
 *      - construct and initialize your own `hook_info` with 4 data:
 *              - `orig_fn` is the function to be hooked
 *              - `hook_before` will be called before the original function
 *              - `hook_after` will be called after the original function
 *              - `call_orig` decides whether the original fnc will be called
 * 
 *      - define your hook_func(size_t, size_t, size_t, size_t, size_t, size_t)
 *          as the `new_dst` for hooking. In this function you should collect
 *          all the  original arguments that may be passed, and call the
 *          `a3khook_executor()` to do the execution.
 * 
 *      - call `a3khook_add_new_hook(your_hook_info, target_addr)` to hook.
 * 
 * 
 * This is a template for you:
 * 
 * // what you need to provide for hooking
 * 
 * void my_hook_before_fn(size_t *args)
 * {
 *     // do what you want
 * }
 * size_t my_hook_after_fn(size_t orig_ret, size_t *args)
 * {
 *     // do what you want
 *     return orig_ret;
 * }
 * 
 * struct hook_info my_hook_info = {
 *     .hook_before = my_hook_before_fn,
 *     .hook_after = my_hook_after_fn,
 *     .orig_fn = (void*) commit_creds, // just an example there :)
 *     .call_orig = 1,
 * };
 * 
 * size_t template_hook_dst(size_t arg0, size_t arg1, size_t arg2, size_t arg3, 
 *                          size_t arg4, size_t arg5)
 * {
 *     size_t args[6];
 * 
 *     args[0] = arg0;
 *     args[1] = arg1;
 *     args[2] = arg2;
 *     args[3] = arg3;
 *     args[4] = arg4;
 *     args[5] = arg5;
 * 
 *     return a3khook_executor(&my_hook_info, args);
 * }
 * 
 * // what you need to do while loading the module
 * 
 * static int __init module_init(void)
 * {
 *     //...
 * 
 *     a3khook_add_new_hook(&my_hook_info, template_hook_dst);
 * 
 *     //...
 * }
 * 
 */

#endif