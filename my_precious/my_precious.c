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

SYSCALL_DEFINE0(my_precious)
{
        struct task_struct *tsk = current;
        struct page *new_page = alloc_page(GFP_KERNEL);
	struct mm_struct *mm = tsk->mm;

	struct vm_area_struct *vma = mm->mmap;

	while(!vma_is_anonymous(vma))
		vma = vma->vm_next;
	
	if(!vma) {
		printk("vma is Null\n");
		goto out;
	}

	unsigned long address = vma->vm_start;
	page_to_phys(new_page);
        printk("new Page at physical address %ld\n",page_to_phys(new_page));
        tsk->saved_page_paddr = page_to_phys(new_page);
	printk("new Page at physical address %ld\n",tsk->saved_page_paddr);

	down_write(&mm->mmap_lock);
	pgd_t *pgd = pgd_offset_gate(mm, address);
	if(pgd_none(*pgd) || pgd_bad(*pgd)) {
		printk("pgd not valid\n");
		goto out;
	}

	p4d_t *p4d = p4d_offset(pgd, address);
	if(p4d_none(*p4d) || p4d_bad(*p4d)) {
		printk("p4d not valid\n");
                goto out;
	}

	pud_t *pud = pud_offset(p4d, address);
	if(pud_none(*pud) || pud_bad(*pud)) {
		printk("pgd not valid\n");
                goto out;
	}

	pmd_t *pmd = pmd_offset(pud, address);
	if(pmd_none(*pmd) || pmd_bad(*pmd)) {
		printk("pgd not valid\n");
                goto out;
	}

	pte_t *pte = pte_offset_map(pmd, address);
	
	if(!pte)
		goto out;

	unsigned long pfn = page_to_pfn(new_page);
	unsigned long flags = pte_val(*pte) & ~PAGE_MASK;
	pte_t ptep = pfn_pte(pfn, __pgprot(flags));

	set_pte_at(mm, address, pte, ptep);
	flush_tlb_page(vma, address);
	pte_unmap(pte);

out:
	up_write(&mm->mmap_lock);
	mmput(mm);
      
      	return 0;

}

