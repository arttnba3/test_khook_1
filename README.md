# a3khook

Another lightweight dynamic-hooking engine for Linux kernel.

### 0x00. Becore we start

The core of the hooking engine is `a3khook` directory, and the `main.c` is just an example LKM using it : )

This is the preview of hooking `commit_creds()` as an example:

![preview.png](https://s2.loli.net/2023/05/15/tF21qeGI3oEkSrH.png)

### 0x01. How to use?

Firstly you need to include the `a3khook/a3khook.h` in your source file and add the `a3khook/a3khook.o` into your `Makefile` or `Kbuild`.

Then You should define your own `struct hook_info` in your module with these fileds initialized:

- `hook_info.orig_fn`: The function to be hooked.
- `hook_info.call_orig`: Whether the original function will be called (1 for true).
- `hook_info.hook_before`: The function to be called before the original function.
- `hook_info.hook_after`: The function to be called after the original function.

This is an example:

```c
#include "a3khook/a3khook.h"

void my_hook_before_fn(size_t *args)
{
    printk(KERN_ERR "[a3khook:] my_hook_before_fn() is called!");
    // do what you want
}

size_t my_hook_after_fn(size_t orig_ret, size_t *args)
{
    printk(KERN_ERR "[a3khook:] my_hook_after_fn() is called!");
    // do what you want
    return orig_ret;
}

struct hook_info my_hook_info = {
    .hook_before = my_hook_before_fn,
    .hook_after = my_hook_after_fn,
    .orig_fn = (void*) commit_creds, // just an example there :)
    .call_orig = 1,
};
```

Then you need to define a `hook_dst()` function like this template, which is actually called after the target function had been hooked:

```c
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
```

Now, you can start to hook the function simply like this in you module's `init` function:

```c
static int __init kernel_module_init(void)
{
    printk(KERN_INFO "Hello the Linux kernel!");

    /* this is just a test */
    a3khook_add_new_hook(&my_hook_info, template_hook_dst);
    return 0;
}
```

Note that you should prepare a `struct hook_info` and `hook_dst()` for each function to be hooked.

### 0x02. How it works?

Unlike other existed hook engines, we don't identify what the instructions really are at the location of target function. Instead we just store the data that will be ovwerwrited by our hooking and patch the target directly.

When the target function is called, it'll jump directly to our `hook_dst()`, which will just collect all the arguments that may be passed in and call the `a3khook_executor()`.

In `a3khook_executor()` , we'll act as below:

- Call the `hook_info.hook_before()` if it's set.
- Use the data we stored before to re-patch the target function back.
- Call the `hook_info.call_orig()`, which is the target function to be hooked.
- Call the `hook_info.hook_after()` if it's set.
- Patch the target function to our `hook_dst()` again.

With such action flow, we don't need to identify the instructions at the location of target function to be hook (because CISC sucks). Just patch the target function again and again is okay : )

### 0x03. Author

[arttnba3](mailto:arttnba@gmail.com)

### 0x04. License

GPL
