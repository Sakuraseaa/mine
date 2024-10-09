#include "VFS.h"
#include "basetype.h"
#include "errno.h"
#include "memory.h"
#include "task.h"
#include "fcntl.h"
#include "printk.h"
#include "lib.h"
#include "debug.h"

typedef unsigned int Elf64_Word;
typedef unsigned long Elf64_Addr;
typedef unsigned long Elf64_Off;
typedef unsigned short Elf64_Half;
typedef unsigned long Elf64_Xword;


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
    Elf64_Off	p_offset;		/* Segment file offset */       // segment 在文件中的偏移
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
	if (dentry->dir_inode->attribute == FS_ATTR_DIR) {
		color_printk(RED, BLACK, "bash: %s: Is a directory\n", path);
		return (void *)-ENOTDIR;
	}
	filp = (struct file *)knew(sizeof(struct file), 0);
	if (filp == NULL)
		return (void *)-ENOMEM;
	filp->position = 0;
	filp->mode = 0;
	filp->dentry = dentry;
	filp->mode = O_RDONLY;
	filp->f_ops = dentry->dir_inode->f_ops;


	current->i_exec = filp->dentry->dir_inode;

	return filp;
}

/**
 * @brief 进行虚拟地址的映射
 * 
 * @param user_addr 
 */
static void virtual_map(unsigned long user_addr){
	
	unsigned long *tmp;
	unsigned long *virtual = NULL;
	struct Page *p = NULL;
	
	// 为其分配独立的应用层地址空间,PML(page map level 4, 4级页表)中的页表项指针
	tmp = Phy_To_Virt((unsigned long *)((unsigned long)current->mm->pgd & (~0xfffUL)) + ((user_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		virtual = knew(PAGE_4K_SIZE, 0); // 申请PDPT内存，填充PML4页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}
	
	// 获取该虚拟地址对应的PDPT(page directory point table)中的页表项指针
	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		virtual = knew(PAGE_4K_SIZE, 0); // 申请PDT内存，填充PDPT页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}
	
	// 获取该虚拟地址对应的PDT(page directory table)中的页表项指针
	// 申请用户占用的内存,填充页表, 填充PDT内存
	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_2M_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		virtual = knew(PAGE_4K_SIZE, 0); // 申请page_table 内存，填充page_dirctory页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}

	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_4K_SHIFT) & 0x1ff));
	if (*tmp == 0)
	{
		virtual = knew(PAGE_4K_SIZE, 0); // 申请页表内存，填充页表项
		memset(virtual, 0, PAGE_4K_SIZE);

		set_pdt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Page_4K));
	}

}

/**
 * @brief segment_load用于将filp指向的文件中偏移为offset, 大小为filesz的段加载到虚拟地址为
 *        vaddr所对应的物理内存处. 该函数会自动为分配用户空间内存并完成与vaddr表示的虚拟页的映射
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
    long size_in_first_page = PAGE_4K_SIZE - (vaddr & (PAGE_4K_SIZE - 1));

    // 如果虚拟页内装不下, 则计算额外需要的页数
    unsigned long occupy_pages = 1; // 计算本次加载要占用的物理页数
    if (filesz > size_in_first_page) {
        unsigned long left_size = filesz - size_in_first_page;
        occupy_pages = (PAGE_4K_ALIGN(left_size) / PAGE_4K_SIZE) + 1;
    }

    unsigned long page_idx = 0;
    unsigned long vaddr_page = vaddr_first_page;
	do
    { 	
		if( !(*pml4e_ptr(vaddr_page) & 0x01) || !(*pdpe_ptr(vaddr_page) & 0x01) 
			|| !(*pde_ptr(vaddr_page) & 0x01) || !(*pte_ptr(vaddr_page) & 0x01)) 
		{
			virtual_map(vaddr_page); // 映射地址，若虚拟地址对应的实际地址为空，则分配物理面
		}
        vaddr_page += PAGE_4K_SIZE;
        page_idx++;
    } while (page_idx < occupy_pages);
	
	filp->f_ops->lseek(filp, offset, SEEK_SET);
	filp->f_ops->read(filp, (void *)vaddr_first_page, filesz, &filp->position);

    return true;
}

static unsigned long code_start_addr = 0;

/**
 * @brief analysis The Section Table
 * 
 * @param filp 
 * @param elf_header 
 * @return unsigned long 
 */
static unsigned long section_analysis(struct file *filp, Elf64_Ehdr* elf_header) {
    
	Elf64_Half	s_size = elf_header->e_shentsize;		/* Section header table entry size */  
    Elf64_Half	s_num = elf_header->e_shnum;		    /* Section header table entry count */
	Elf64_Off	s_off = elf_header->e_shoff; 			/* Section header table file offset */
	
    unsigned int sect_idx = 0;
	Elf64_Shdr *section_header = (Elf64_Shdr*)knew(s_num * s_size, 0);
	Elf64_Shdr *shstr_entry = NULL;
	char* s_name_table = NULL;

	// read Section Header

	memset(section_header, 0, s_num * s_size);
	filp->f_ops->lseek(filp, s_off, SEEK_SET);
	filp->f_ops->read(filp, (char*)section_header, s_num * s_size, &filp->position);


	// read secontion Header string table
	shstr_entry = &section_header[elf_header->e_shstrndx];
	s_name_table = knew(shstr_entry->sh_size, 0);
	memset(s_name_table, 0, shstr_entry->sh_size);
	filp->f_ops->lseek(filp, shstr_entry->sh_offset, SEEK_SET);
	filp->f_ops->read(filp, s_name_table, shstr_entry->sh_size, &filp->position);
	
	//  为 mm_struct 文件赋值
	struct mm_struct* mm = current->mm;
	while(sect_idx < s_num) {
	
		char* s_name = s_name_table + (section_header[sect_idx].sh_name);
		if(!strcmp(s_name, ".text")) {
		
			mm->start_code = section_header[sect_idx].sh_addr;
			mm->end_code = mm->start_code + section_header[sect_idx].sh_size;
		
		} else if (!strcmp(s_name, ".rodata")) {
		
			mm->start_rodata = section_header[sect_idx].sh_addr;
			mm->end_rodata = mm->start_rodata + section_header[sect_idx].sh_size;

		} else if(!strcmp(s_name,".data")) {
		
			mm->start_data = section_header[sect_idx].sh_addr;
			mm->end_data = mm->start_data + section_header[sect_idx].sh_size;
		
		} else if(!strcmp(s_name, ".bss")) {
			
			mm->start_bss = section_header[sect_idx].sh_addr;
			mm->end_bss = mm->start_bss + section_header[sect_idx].sh_size;
		
		}
		
		sect_idx++;
	}

	return 0;
}

/**
 * @brief load用于将filename指向的程序文件加载到内存中
 *
 * @param pathname 需要加载的程序文件的名称
 * @return unsiged long 若加载成功, 则返回程序的 bss 段的结束地址; 若加载失败, 则返回-1
 */
static unsigned long load(char *pathname)
{
	struct file *filp = NULL;
	unsigned long end_bss = 0;
	long ret = -1;

	Elf64_Ehdr elf_header;
	Elf64_Phdr prog_header;
	memset(&elf_header, 0, sizeof(Elf64_Ehdr));
	memset(&prog_header, 0, sizeof(Elf64_Phdr));
	
	filp = open_exec_file(pathname);
	if((unsigned long)filp > -0x1000UL) // 这是什么意思？
		return (unsigned long)filp;
	

	// 读取程序头
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

	code_start_addr = elf_header.e_entry; // 程序的入口地址
	
	section_analysis(filp,&elf_header);
	
	Elf64_Half prog_header_size = elf_header.e_phentsize;
    Elf64_Off prog_header_offset = elf_header.e_phoff;
    // 遍历所有程序头表
    unsigned int prog_idx = 0;
	
	while (prog_idx < elf_header.e_phnum)
	{
		memset(&prog_header, 0, prog_header_size);
		// 读取程序头表项
		filp->f_ops->lseek(filp, prog_header_offset + (prog_idx * prog_header_size), SEEK_SET);	
		if(filp->f_ops->read(filp, (void *)&prog_header, sizeof(Elf64_Phdr), &filp->position) != prog_header_size) {
			color_printk(RED, BLACK,"EXECVE -> read file ERROR!\n");
			return ret;
		}
		
		// 加载可加载段，到内存
		if(PT_LOAD == prog_header.p_type) {
			
			segment_load(filp, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr);
			end_bss = prog_header.p_vaddr + prog_header.p_filesz; // 计算end_bss
		}
		prog_idx++;
	}
	
	return end_bss;
}


// 被init调用,加载用户进程体，到用户空间800000
unsigned long do_execve(pt_regs_t *regs, char *name, char* argv[], char *envp[])
{
  	// color_printk(RED, BLACK, "do_execve task is running\n");
	unsigned long stack_start_addr = TASK_SIZE + 1;
	unsigned long retval = 0;
	long pos = 0;
	
	if (current->flags & PF_VFORK)
	{
		// 若当前进程使用PF_VFORK标志，说明它正与父进程共享地址空间
		// 而新程序必须拥有独立的地址空间才能正常运行
		current->mm = (struct mm_struct *)knew(sizeof(struct mm_struct), 0);
		memset(current->mm, 0, sizeof(struct mm_struct));
		current->mm->pgd = (pml4t_t *)Virt_To_Phy(knew(PAGE_4K_SIZE, 0));
		DEBUGK("load_binary_file malloc new pgd:%#018lx\n", current->mm->pgd);
		memset(Phy_To_Virt(current->mm->pgd), 0, PAGE_4K_SIZE / 2);
		// copy kernel space
		memcpy(Phy_To_Virt(init_task[0]->mm->pgd) + 256, Phy_To_Virt(current->mm->pgd) + 256, PAGE_4K_SIZE / 2);

		current->flags &= ~PF_VFORK;
		__asm__ __volatile__("movq %0, %%cr3 \n\t" ::"r"(current->mm->pgd): "memory");  
	}

	if((retval = load(name)) == -1) {
		// ERROR!!!!!
	}

	if (!(current->flags & PF_KTHREAD))
		current->addr_limit = TASK_SIZE;

	current->mm->end_bss = retval;

	// 设置用户堆 起始地址
	current->mm->start_brk = current->mm->end_brk = PAGE_2M_ALIGN(current->mm->end_bss); 

	// 映射栈地址, 目前刚启动为栈分配2MB （有点多？）
	// issue:: 栈空气不够了，如何扩充？ （缺页中断！）
	virtual_map(stack_start_addr - PAGE_4K_SIZE);

	// 设置用户栈 起始地址
	current->mm->start_stack = stack_start_addr - PAGE_4K_SIZE;
	current->mm->stack_length = PAGE_4K_SIZE;

	exit_files(current);

	// 在用户空间，复制进程运行参数, rewriter:: this is argc locked 10. all right.
	if( argv != NULL ) {
		int len = 0, i = 0;
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

    __asm__ __volatile__("movq %0,%%gs; movq %0, %%fs;"::"r"(0UL));

	// 为进程创建初始进程环境(上下文)
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
	
	flush_tlb();

	// go to ret_system_call
	return retval;
}



