#include "VFS.h"
#include "errno.h"
#include "memory.h"
#include "task.h"
#include "fcntl.h"
#include "stdio.h"
#include "printk.h"
typedef unsigned int Elf64_Word;
typedef unsigned long Elf64_Addr, Elf64_Off;
typedef unsigned short Elf64_Half;
typedef unsigned long Elf64_Xword;
typedef char bool
typedef 0 FALSE
typedef 1 TRUE

#define EI_NIDENT 16

typedef struct
{
    unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */    // 魔术， elf文件类型（标识是32位还是64位），指定大端还是小端，ELF头的版本信息，e_ident7 ~ 15位不使用
    Elf64_Half	e_type;			    /* Object file type */                   // 文件类型 Relocatable file(.o), Executable(.out), Shared Object File(.so)
    Elf64_Half	e_machine;		    /* Architecture */                       // CPU 平台类型
    Elf64_Word	e_version;		    /* Object file version */                // ELF 版本号
    Elf64_Addr	e_entry;		    /* Entry point virtual address */        // 执行该程序的入口地址
    Elf64_Off	e_phoff;			/* Program header table file offset */   // 程序头表在文件中的偏移
    Elf64_Off	e_shoff;			/* Section header table file offset */   // 段表在文件中的偏移
    Elf64_Word	e_flags;		    /* Processor-specific flags */           // ELF标志位，标识一些ELF文件平台相关的属性
    Elf64_Half	e_ehsize;		    /* ELF header size in bytes */           // ElF文件头本身的大小
    Elf64_Half	e_phentsize;		/* Program header table entry size */    // 程序头表的表项的大小(segment tbale)
    Elf64_Half	e_phnum;		    /* Program header table entry count */   // 程序头表的表项的数量
    Elf64_Half	e_shentsize;		/* Section header table entry size */    // 段表的表项大小（section table）
    Elf64_Half	e_shnum;		    /* Section header table entry count */   // 段表的表项数量
    Elf64_Half	e_shstrndx;		    /* Section header string table index */  // 段表字符串表所在的段在段表中的下标
} Elf64_Ehdr;

/* section header. i'll leave it, maybe l will need it later*/
/* Section header.  */
typedef struct
{
    Elf64_Word	sh_name;		/* Section name (string tbl index) */
    Elf64_Word	sh_type;		/* Section type */
    Elf64_Xword	sh_flags;		/* Section flags */
    Elf64_Addr	sh_addr;		/* Section virtual addr at execution */
    Elf64_Off	    sh_offset;		/* Section file offset */
    Elf64_Xword	sh_size;		/* Section size in bytes */
    Elf64_Word	sh_link;		/* Link to another section */
    Elf64_Word	sh_info;		/* Additional section information */
    Elf64_Xword	sh_addralign;		/* Section alignment */
    Elf64_Xword	sh_entsize;		/* Entry size if section holds table */
} Elf64_Shdr;

typedef struct
{
    Elf64_Word	p_type;			/* Segment type */              // segment 的类型
    Elf64_Word	p_flags;		/* Segment flags */             // segment 的权限，R， W， X
    Elf64_Off	    p_offset;		/* Segment file offset */       // segment 在文件中的偏移
    Elf64_Addr	p_vaddr;		/* Segment virtual address */   // segment 在第一个字节在进程虚拟地址空间的起始位置
    Elf64_Addr	p_paddr;		/* Segment physical address */  // segment 的物理装载地址
    Elf64_Xword	p_filesz;		/* Segment size in file */      // segment 在ELF文件中所占空间的长度
    Elf64_Xword	p_memsz;		/* Segment size in memory */    // segment 在进程虚拟地址空间所占用的长度
    Elf64_Xword	p_align;		/* Segment alignment */         // 对齐属性，实际对齐字节等于2的p_align次
} Elf64_Phdr;

/* 段类型 */
enum segment_type
{
    PT_NULL,    // 忽略
    PT_LOAD,    // 可加载程序段
    PT_DYNAMIC, // 动态加载信息
    PT_INTERP,  // 动态加载器名称
    PT_NOTE,    // 一些辅助信息
    PT_SHLIB,   // 保留
    PT_PHDR     // 程序头表
};

/**
 * @brief open_exec_file(char*)用于搜索文件系统的目标文件，本函数与sys_open函数的指向流程基本相似
 *本函数最重要的作用是为目标文件描述符指派操作方法（filp.f_ops = dentry.dir_inode.f_ops）
 */
struct file *open_exec_file(char *path)
{
    struct dir_entry *dentry = NULL;
	struct file *filp = NULL;

	dentry = path_walk(path, 0, 0);
	if (dentry == NULL)
		return (void *)-ENOENT;
	if (dentry->dir_inode->attribute == FS_ATTR_DIR)
		return (void *)-ENOTDIR;

	filp = (struct file *)kmalloc(sizeof(struct file), 0);
	if (filp == NULL)
		return (void *)-ENOMEM;
	filp->position = 0;
	filp->mode = 0;
	filp->dentry = dentry;
	filp->mode = O_RDONLY;
	filp->f_ops = dentry->dir_inode->f_ops;

	return filp;
}
/**
 * @brief pte_addr用于获得虚拟地址vaddr对应的页表项指针，pte中有vaddr保存的物理页地址
 *
 * @param vaddr 需要获得pte地址的虚拟地址
 * @return uint32_t* 可访问pte的虚拟地址
 */
unsigned long *pmle_ptr(unsigned long vaddr)
{
    unsigned long *pte =  Phy_To_Virt((unsigned long *)((unsigned long)current->mm->pgd & (~0xfffUL)) +
					  ((vaddr >> PAGE_GDT_SHIFT) & 0x1ff));
    return pte;
}

/**
 * @brief pde_addr用于获得指向给定虚拟地址所在的页表的地址，即pde中的物理地址
 *
 * @param vaddr 需要获得pde地址的虚拟地址
 * @return uint32_t* 虚拟地址的pde地址
 */
unsigned long *pdpe_ptr(unsigned long vaddr)
{
    // 要想访问页目录表，那么就得让cpu以为访问到物理页地址的时候，其实访问的是页目录表地址
    // 然后通过 pde偏移 * 4, 访问目标页目录表项
	unsigned long *pde = Phy_To_Virt((unsigned long *)(*pmle_ptr(vaddr) & (~0xfffUL)) + ((vaddr >> PAGE_1G_SHIFT) & 0x1ff));
    return pde;
}
static long virtual_map(unsigned long user_addr){
	
	unsigned long *tmp;
	unsigned long *virtual = NULL;
	struct Page *p = NULL;
	
	if (current->flags & PF_VFORK)
	{
		// 若当前进程使用PF_VFORK标志，说明它正与父进程共享地址空间
		// 而新程序必须拥有独立的地址空间才能正常运行
		current->mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
		memset(current->mm, 0, sizeof(struct mm_struct));
		current->mm->pgd = (pml4t_t *)Virt_To_Phy(kmalloc(PAGE_4K_SIZE, 0));
		color_printk(RED, BLACK, "load_binary_file malloc new pgd:%#018lx\n", current->mm->pgd);
		memset(Phy_To_Virt(current->mm->pgd), 0, PAGE_4K_SIZE / 2);
		// copy kernel space
		memcpy(Phy_To_Virt(init_task[0]->mm->pgd) + 256, Phy_To_Virt(current->mm->pgd) + 256, PAGE_4K_SIZE / 2);
		current->flags &= ~PF_VFORK;
	}

	// 为其分配独立的应用层地址空间,PML(page map level 4, 4级页表)中的页表项指针
	tmp = Phy_To_Virt((unsigned long *)((unsigned long)current->mm->pgd & (~0xfffUL)) +
					  ((user_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if (*tmp == NULL)
	{
		virtual = kmalloc(PAGE_4K_SIZE, 0); // 申请PDPT内存，填充PML4页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_GDT));
	}
// 获取该虚拟地址对应的PDPT(page directory point table)中的页表项指针
	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if (*tmp == NULL)
	{
		virtual = kmalloc(PAGE_4K_SIZE, 0); // 申请PDT内存，填充PDPT页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}
// 获取该虚拟地址对应的PDT(page directory table)中的页表项指针
	// 申请用户占用的内存,填充页表, 填充PDT内存
	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_2M_SHIFT) & 0x1ff));
	if (*tmp == NULL)
	{
		p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
		set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));
	}
	
}

/**
 * @brief segment_load用于将fd指向的文件中偏移为offset, 大小为filesz的段加载到虚拟地址为
 *        vaddr所对应的物理内存处. 该函数会自动为分配一个物理页, 并完成与vaddr表示的虚拟页的映射
 *
 * @param fd 需要加载的程序文件的文件描述符
 * @param offset 段在文件内的偏移地址
 * @param filesz 段大小
 * @param vaddr 加载到内存中的虚拟地址
 * @return true 加载成功
 * @return false 加载失败
 */
static bool segment_load(struct file* filp, unsigned long offset, unsigned long filesz, unsigned long vaddr) {
    // 计算段将要加载到的虚拟页
    unsigned long vaddr_first_page = vaddr & TASK_SIZE;
    // 表示文件在第一个页框中占用的字节大小
    unsigned long size_in_first_page = PAGE_2M_SIZE - (vaddr & PAGE_2M_MASK);

    // 如果虚拟页内装不下, 则计算额外需要的页数
    unsigned long occupy_pages = 0;
    if (filesz > size_in_first_page) {
        unsigned long left_size = filesz - size_in_first_page;
        occupy_pages = PAGE_2M_ALIGN(left_size / PAGE_2M_SIZE) + 1;
    }
    else {
        occupy_pages = 1;
    }

    unsigned long page_idx = 0;
    unsigned long vaddr_page = vaddr_first_page;
    while (page_idx < occupy_pages)
    {
		



        vaddr_page += PAGE_2M_SIZE;
        page_idx++;
    }
	
	filp->f_ops->lseek(filp, offset, SEEK_SET);
	filp->f_ops->read(filp, vaddr_page, filesz, &filp->position);

    return true;
}

/**
 * @brief load用于将filename指向的程序文件加载到内存中
 *
 * @param pathname 需要加载的程序文件的名称
 * @return int32_t 若加载成功, 则返回程序的起始地址(虚拟地址); 若加载失败, 则返回-1
 */
static long load(char *pathname)
{
	struct file *filp = NULL;
	unsigned long retval = 0;
	long pos = 0, ret = -1;

	Elf64_Ehdr elf_header;
	Elf64_Phdr prog_header;
	memset(&elf_header, 0, sizeof(Elf64_Ehdr));
	memset(&prog_header, 0, sizeof(Elf64_Phdr));

	
	filp = open_exec_file(pathname);
	if((unsigned long)filp > -0x1000UL) // 这是什么意思？
		return (unsigned long)filp;
	
	filp->f_ops->read(filp, (void *)&elf_header, sizeof(Elf64_Ehdr), &filp->position);
	
	// 校验elf头, check elf header
    if (
        // octal 177 = 0x7f, 0x7f + ELF is elf magic number.
        // \1\1\1 means 64-bit elf file, LSB(Least Significant Bit), current version separately
        memcmp(elf_header.e_ident, "\177ELF\2\1\1", 7)
        // e_type == 2, executable file
        || elf_header.e_type != 2
        // e_machine == 3,  Advanced Micro Devices X86-64
        || elf_header.e_machine != 62
        // e_version == 1, fix to 1
        || elf_header.e_version != 1
        // maximum support 1024
        || elf_header.e_phnum > 1024
        // correspond to Elf32_Phdr
        || elf_header.e_phentsize != sizeof(Elf64_Phdr))
    {
        color_printk(RED, BLACK,"Elf Header check failed!\n");
        return ret;
    }

  	Elf64_Half prog_header_size = elf_header.e_phentsize;
    Elf64_Off prog_header_offset = elf_header.e_phoff;
    // 遍历所有程序头表
    unsigned int prog_idx = 0;
	filp->f_ops->lseek(filp, prog_header_offset, SEEK_SET);	
	while (prog_idx < elf_header.e_phnum)
	{
		memset(&prog_header, 0, prog_header_size);

		if(filp->f_ops->read(filp, (void *)&prog_header, sizeof(Elf64_Phdr), &filp->position) != prog_header_size) {
			color_printk(RED, BLACK,"EXECVE -> read file ERROR!\n");
			return ret;
		}
		
		if(PT_LOAD == prog_header.p_type) {
			segment_load(filp, prog_header.p_offset, prog_header.p_memsz, prog_header.p_vaddr);
		}
		prog_idx++;
	}
	

done:
	
}


// 被init调用,加载用户进程体，到用户空间800000
unsigned long do_execve(struct pt_regs *regs, char *name, char* argv[], char *envp[])
{
  	// color_printk(RED, BLACK, "do_execve task is running\n");
	unsigned long code_start_addr = 0x800000;
	unsigned long stack_start_addr = 0xa00000;
	unsigned long brk_start_addr = 0xc00000;
	struct file *filp = NULL;
	unsigned long retval = 0;
	long pos = 0;

	load(name);

	__asm__ __volatile__("movq %0, %%cr3 \n\t" ::"r"(current->mm->pgd): "memory");
    

	if (!(current->flags & PF_KTHREAD))
		current->addr_limit = TASK_SIZE;

	current->mm->start_code = code_start_addr;
	current->mm->end_code = 0;
	current->mm->start_data = 0;
	current->mm->end_data = 0;
	current->mm->start_rodata = 0;
	current->mm->end_rodata = 0;
	current->mm->start_bss = code_start_addr + filp->dentry->dir_inode->file_size;
	current->mm->end_bss = stack_start_addr;
	current->mm->start_brk = brk_start_addr;
	current->mm->end_brk = brk_start_addr;
	current->mm->start_stack = stack_start_addr;

	exit_files(current);

	if( argv != NULL ) {
		int argc = 0, len = 0, i = 0;
		char** dargv = (char**)(stack_start_addr - 10 * sizeof(char*));
		pos = (unsigned long)dargv;

		for(i = 0; i < 10 && argv[i] != NULL; i++)
		{
			len = strnlen_user(argv[i], 1024) + 1;
			strcpy((char*)(pos - len), argv[i]);
			dargv[i] = (char*)(pos - len);
			pos -= len;
		}

		stack_start_addr = pos - 10;
		regs->rdi = i; // argc
		regs->rsi = (unsigned long)dargv; // argv
	}


	// 清理地址空间脏数据
	memset((void *)code_start_addr, 0, stack_start_addr - code_start_addr);
	pos = 0;
	retval = filp->f_ops->read(filp, (void *)code_start_addr, filp->dentry->dir_inode->file_size, &pos);
	
    __asm__ __volatile__("movq %0,%%gs; movq %0, %%fs;"::"r"(0UL));


	// 这里没有初始化gs,fs段选择子
	regs->ds = USER_DS;
	regs->es = USER_DS;
	regs->ss = USER_DS;
	regs->cs = USER_CS;
	// regs->rip = new_rip;
	// regs->rsp = new_rsp;
	//  在中断栈中填入地址
	regs->r10 = code_start_addr; // RIP
	regs->r11 = stack_start_addr; // RSP
	regs->rax = 1;
	// go to ret_system_call
	return retval;
}

