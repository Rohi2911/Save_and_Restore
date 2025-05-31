#include <asm/current.h>
#include <linux/gfp.h>
#include <linux/mm_types.h>
#include <asm-generic/memory_model.h>
#include <linux/printk.h>
#include <linux/syscalls.h>
#include <asm/io.h>
#include <linux/mm_types.h>
#include <linux/pgtable.h>
#include <asm/pgtable_types.h>
#include <asm/pgtable.h>
#include <linux/rwsem.h>
#include <linux/sched/mm.h>
#include <asm/tlbflush.h>
#include <asm-generic/memory_model.h>
#include <linux/rmap.h>
#include <linux/pagemap.h>
#include <linux/spinlock_types.h>
//#include <asm-generic/atomic.h>
#include <linux/mmzone.h>
#include <linux/page_ref.h>

SYSCALL_DEFINE1(my_precious, int, opr)
{
        struct task_struct *tsk = current;
        struct page *new_page = NULL, *old_page = NULL;
        struct mm_struct *mm = tsk->mm;
	struct vm_area_struct *vma;
	unsigned long address;
	pte_t *pte = NULL;
        spinlock_t *ptl = NULL;
	int err = 0;

	if(opr == 0) {


		down_write(&mm->mmap_lock);

		/*
		 * Find out the first VMA in annonymous
		 * memory region
		 */

		for(vma = mm->mmap; vma; vma = vma->vm_next) {
                	if(vma_is_anonymous(vma)) break;
		}

		if (!vma) {
                	printk(KERN_ERR "No anonymous VMA found\n");
                	err = -EINVAL;
                	goto out_unlock;
		}

		address = vma->vm_start;

		 // Walk page tables
        	pgd_t *pgd = pgd_offset(mm, address);
        	if (pgd_none(*pgd) || pgd_bad(*pgd)) goto out_unlock;

        	p4d_t *p4d = p4d_offset(pgd, address);
        	if (p4d_none(*p4d) || p4d_bad(*p4d)) goto out_unlock;

        	pud_t *pud = pud_offset(p4d, address);
        	if (pud_none(*pud) || pud_bad(*pud)) goto out_unlock;

        	pmd_t *pmd = pmd_offset(pud, address);
        	if (pmd_none(*pmd) || pmd_bad(*pmd)) goto out_unlock;

        	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
        	if (!pte || !pte_present(*pte)) {
                	printk(KERN_ERR "Invalid or absent PTE\n");
                	goto out_unlock_pte;
        	}

		// Extract old page
        	old_page = pfn_to_page(pte_pfn(*pte));
        	if (!old_page) {
                	printk(KERN_ERR "Failed to retrieve old page\n");
                	goto out_unlock_pte;
        	}

		/* Allocate New page*/
		new_page = alloc_pages(GFP_KERNEL | __GFP_ZERO, 0);
        	if (!new_page)
                	return -ENOMEM;
		
		/* Copy data on old page to new page */
		if (page_address(old_page) && page_address(new_page)) {
                	memcpy(page_address(new_page), page_address(old_page),  PAGE_SIZE);
        	} else {
                	printk("One of the page addresses is NULL, skipping memcpy\n");
        	}
		
		tsk->saved_page = new_page;

	out_unlock_pte:
        	if (pte)
                	pte_unmap_unlock(pte, ptl);

	out_unlock:
        	up_write(&mm->mmap_lock);

	}
	
	/*else if (opr == 1) {
		
	}*/
      	return 0;

}

