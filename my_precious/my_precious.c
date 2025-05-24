#include <asm/current.h>
#include <linux/gfp.h>
#include<linux/mm_types.h>
#include <asm-generic/memory_model.h>
#include <linux/printk.h>
#include <linux/syscalls.h>
#include <asm/io.h>

SYSCALL_DEFINE0(my_precious)
{
        struct task_struct *tsk = current;
        struct page *new_page = alloc_page(GFP_KERNEL);
        page_to_phys(new_page);
        printk("new Page at physical address %ld\n",page_to_phys(new_page));
        tsk->saved_page_paddr = page_to_phys(new_page);
	printk("new Page at physical address %ld\n",tsk->saved_page_paddr);
        return 0;

}

