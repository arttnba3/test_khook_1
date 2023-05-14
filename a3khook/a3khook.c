#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <asm/io.h>

#include "a3khook.h"

/**
 * @brief ioremap a specific vaddr's page to a new vaddr and copy data to it.
 * We can use this to overwrite some read-only memory.
 * 
 * @param dst read-only mem to be overwrite
 * @param src new data to overwrite
 * @param len len of the new data
 * @return int 0 for success, 1 for failed
 */
static int a3khook_write_romem_by_ioremap(void *dst, void *src, size_t len)
{
    size_t dst_phys_page_addr, dst_offset;
    size_t dst_ioremap_page_addr;

    /* get the physical addr of target addr */
    dst_phys_page_addr = page_to_pfn(virt_to_page(dst)) * PAGE_SIZE;
    dst_offset = (size_t) dst & 0xfff;

    /* make sure the len may not be too long :) */
    if (len > 0x1000) {
        return 1;
    }
    
    /* map the physical pages to another virt addr so that we can write on it */
    dst_ioremap_page_addr = (size_t) ioremap(dst_phys_page_addr, len + 0x1000);
    memcpy((void*) (dst_ioremap_page_addr + dst_offset), src, len);
    iounmap((void*) dst_ioremap_page_addr);

    return 0;
}

size_t a3khook_executor(struct hook_info *info, size_t *args)
{
    size_t ret;

    /**
     * Note that the function may be called within multi-thread, which may get
     * bad instructions before we repacth the code segment.
     * So that we set a lock here, which may slow down but keep the safety.
     */
    mutex_lock(&info->lock);

    /* patch the original function and call the original and hook functions */
    if (info->hook_before) {
        info->hook_before(args);
    }

    a3khook_write_romem_by_ioremap(info->orig_fn,info->orig_insn,HOOK_BUF_SZ);
    if (info->call_orig) {
        ret = info->orig_fn(args[0],args[1],args[2],args[3],args[4],args[5]);
    }

    if (info->hook_after) {
        ret = info->hook_after(ret, args);
    }

    /* re-patch the hook point again */
    a3khook_write_romem_by_ioremap(info->orig_fn, &info->new_insn, HOOK_BUF_SZ);
    
    mutex_unlock(&info->lock);

    return ret;
}

void a3khook_add_new_hook(struct hook_info *info, void *new_dst)
{
    size_t jmp_offset;

    /* init for lock */
    mutex_init(&info->lock);

    /* save the original hook info */
    memcpy(&info->orig_insn, info->orig_fn, HOOK_BUF_SZ);

    /* let it jmp to hook func */
    jmp_offset = (size_t) new_dst - (size_t) info->orig_fn - 5;/*len of insn*/
    info->new_insn[0] = 0xE9;
    *(uint32_t*) (&info->new_insn[1]) = (uint32_t) jmp_offset;

    a3khook_write_romem_by_ioremap(info->orig_fn, &info->new_insn, HOOK_BUF_SZ);
}
