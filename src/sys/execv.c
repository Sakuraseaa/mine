#include "toolkit.h"
#include "fskit.h"
#include "mmkit.h"
#include "kernelkit.h"

typedef u32_t Elf64_Word;
typedef u64_t Elf64_Addr;
typedef u64_t Elf64_Off;
typedef u16_t Elf64_Half;
typedef u64_t Elf64_Xword;


#define EI_NIDENT 16

typedef struct
{
    u8_t	e_ident[EI_NIDENT];	/* Magic number and other info */    // 魔术， elf文件类型（标识是32位还是64位），指定大端还是小端，ELF头的版本信息，e_ident7 ~ 15位不使用
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

#define PF_X            (1 << 0)        /* Segment is executable */
#define PF_W            (1 << 1)        /* Segment is writable */
#define PF_R            (1 << 2)        /* Segment is readable */
static task_t* cur = nullptr;
/**
 * @brief open_exec_file(char*)用于搜索文件系统的目标文件，本函数与sys_open函数的指向流程基本相似
 *本函数最重要的作用是为目标文件描述符指派操作方法（filp.f_ops = dentry.dir_inode.f_ops）
 */
file_t *open_exec_file(str_t path)
{
    dir_entry_t *dentry = nullptr;
	file_t *filp = nullptr;

	dentry = path_walk(path, 0, 0);
	if (dentry == nullptr)
		return (void *)-ENOENT;
	if (dentry->dir_inode->attribute == FS_ATTR_DIR) {
		color_printk(RED, BLACK, "bash: %s: Is a directory\n", path);
		return (void *)-ENOTDIR;
	}
	filp = (file_t *)knew(sizeof(file_t), 0);
	if (filp == nullptr)
		return (void *)-ENOMEM;
	filp->position = 0;
	filp->mode = 0;
	filp->dentry = dentry;
	filp->mode = O_RDONLY;
	filp->f_ops = dentry->dir_inode->f_ops;

	return filp;
}

/**
 * @brief 进行虚拟地址的映射
 * 自信的说，这个映射代码是过时的，outdated。现在的exec已经不需要该子函数😏了
 * @param user_addr 
 */
#if 0
static void virtual_map(u64_t user_addr){
	
	u64_t *tmp;
	u64_t *virtual = nullptr;
	
	// 为其分配独立的应用层地址空间,PML(page map level 4, 4级页表)中的页表项指针
	tmp = Phy_To_Virt((u64_t *)((u64_t)cur->mm->pgd & (~0xfffUL)) + ((user_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		virtual = knew(PAGE_4K_SIZE, 0); // 申请PDPT内存，填充PML4页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}
	
	// 获取该虚拟地址对应的PDPT(page directory point table)中的页表项指针
	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		virtual = knew(PAGE_4K_SIZE, 0); // 申请PDT内存，填充PDPT页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}
	
	// 获取该虚拟地址对应的PDT(page directory table)中的页表项指针
	// 申请用户占用的内存,填充页表, 填充PDT内存
	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_2M_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		virtual = knew(PAGE_4K_SIZE, 0); // 申请page_table 内存，填充page_dirctory页表项
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}

	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((user_addr >> PAGE_4K_SHIFT) & 0x1ff));
	if (*tmp == 0)
	{
		virtual = knew(PAGE_4K_SIZE, 0); // 申请页表内存，填充页表项
		memset(virtual, 0, PAGE_4K_SIZE);

		set_pdt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Page_4K));
	}

}
#endif

static void set_vmatofile_flag(vtfflags_t* flags, u32_t power)
{
	flags->entry = 0;
	flags->flags.read = (power & PF_R) == PF_R;
	flags->flags.write = (power & PF_W) == PF_W;
	flags->flags.execute = (power & PF_X) == PF_X;
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
static bool segment_load(file_t* filp, u64_t offset, u64_t filesz, u64_t vaddr, u32_t power)
{
    // 计算段将要加载到的虚拟页
    u64_t vaddr_first_page = vaddr & TASK_SIZE;

	vma_to_file_t* vtft = knew(sizeof(vma_to_file_t), 0);
	vtft->vtf_file = filp;
	vtft->vtf_position = offset;
	vtft->vtf_size = filesz;
	vtft->vtf_alread_load_size = 0;
	set_vmatofile_flag(&vtft->vtf_flag, power);
	vma_new_vadrs(cur->mm, vaddr_first_page, filesz, vtft, 0, 0, 0);
	
    return true;
}

static u64_t code_start_addr = 0;

/**
 * @brief analysis The Section Table
 * 
 * @param filp 
 * @param elf_header 
 * @return u64_t 
 */
static u64_t section_analysis(file_t *filp, Elf64_Ehdr* elf_header) {
    
	Elf64_Half	s_size = elf_header->e_shentsize;		/* Section header table entry size */  
    Elf64_Half	s_num = elf_header->e_shnum;		    /* Section header table entry count */
	Elf64_Off	s_off = elf_header->e_shoff; 			/* Section header table file offset */
	
    u32_t sect_idx = 0;
	Elf64_Shdr *section_header = (Elf64_Shdr*)knew(s_num * s_size, 0);
	Elf64_Shdr *shstr_entry = nullptr;
	str_t s_name_table = nullptr;

	// read Section Header

	memset(section_header, 0, s_num * s_size);
	filp->f_ops->lseek(filp, s_off, SEEK_SET);
	filp->f_ops->read(filp, (buf_t)section_header, s_num * s_size, &filp->position);


	// read secontion Header string table
	shstr_entry = &section_header[elf_header->e_shstrndx];
	s_name_table = knew(shstr_entry->sh_size, 0);
	memset(s_name_table, 0, shstr_entry->sh_size);
	filp->f_ops->lseek(filp, shstr_entry->sh_offset, SEEK_SET);
	filp->f_ops->read(filp, s_name_table, shstr_entry->sh_size, &filp->position);
	
	//  为 mm_struct 文件赋值
	mmdsc_t * mm = cur->mm;
	while(sect_idx < s_num) {
	
		str_t s_name = s_name_table + (section_header[sect_idx].sh_name);
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
static u64_t load(str_t pathname)
{
	file_t *filp = nullptr;
	u64_t end_bss = 0;
	s64_t ret = -1;

	Elf64_Ehdr elf_header;
	Elf64_Phdr prog_header;
	memset(&elf_header, 0, sizeof(Elf64_Ehdr));
	memset(&prog_header, 0, sizeof(Elf64_Phdr));
	
	filp = open_exec_file(pathname);
	if((u64_t)filp > -0x1000UL) // 这是什么意思？
		return (u64_t)filp;
	

	// 读取程序头
	filp->f_ops->read(filp, (void *)&elf_header, sizeof(Elf64_Ehdr), &filp->position);
	
	// 校验elf头, check elf header
    if (
        // octal 177 = 0x7f, 0x7f + ELF is elf magic number.
        // \1\1\1 means 64-bit elf file, LSB(Least Significant Bit), Current version separately
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
    u32_t prog_idx = 0;
	
	while (prog_idx < elf_header.e_phnum)
	{
		memset(&prog_header, 0, prog_header_size);
		// 读取程序头表项
		filp->f_ops->lseek(filp, prog_header_offset + (prog_idx * prog_header_size), SEEK_SET);	
		if (filp->f_ops->read(filp, (void *)&prog_header, sizeof(Elf64_Phdr), &filp->position) != prog_header_size) {
			color_printk(RED, BLACK,"EXECVE -> read file ERROR!\n");
			return ret;
		}
		
		// 加载可加载段，到内存
		if(PT_LOAD == prog_header.p_type) {
			segment_load(filp, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr, prog_header.p_flags);
			end_bss = prog_header.p_vaddr + prog_header.p_filesz; // 计算end_bss
		}
		prog_idx++;
	}
	
	return end_bss;
}


// 被init调用,加载用户进程体，到用户空间800000
u64_t do_execve(pt_regs_t *regs, str_t name, str_t argv[], str_t envp[])
{
	u64_t stack_start_addr = USER_VIRTUAL_ADDRESS_END & PAGE_4K_MASK;
	u64_t retval = 0;
	s64_t pos = 0;
	cur = current;
	if (cur->flags & PF_VFORK)
	{
		// 若当前进程使用PF_VFORK标志，说明它正与父进程共享地址空间
		// 而新程序必须拥有独立的地址空间才能正常运行
		cur->mm = (mmdsc_t *)knew(sizeof(mmdsc_t), 0);
		
		// 进程相关内存初始化三大步
		mmadrsdsc_t_init(cur->mm);
		kvma_inituserspace_virmemadrs(&cur->mm->msd_virmemadrs);
		hal_mmu_init(&cur->mm->msd_mmu);

		DEBUGK("load_binary_file malloc new pgd:%#018lx", cur->mm->msd_mmu.mud_cr3);

		cur->flags &= ~PF_VFORK;
		hal_mmu_load(&initmm.msd_mmu);
	}

	if((retval = load(name)) == -1) {
		// ERROR!!!!!
	}

	if (!(cur->flags & PF_KTHREAD))
		cur->addr_limit = TASK_SIZE;


	// 设置用户堆 起始地址
	cur->mm->start_brk = cur->mm->end_brk = PAGE_4K_ALIGN(cur->mm->end_bss); 

	// 设置用户栈 起始地址
	cur->mm->start_stack = stack_start_addr;
	cur->mm->stack_length = PAGE_4K_SIZE;

	exit_files(cur);
	// 这感觉有点不对
	// 在用户空间，复制进程运行参数, rewriter:: this is argc locked 10. all right.
	if( argv != nullptr ) {
		s32_t len = 0, i = 0;
		char_t** dargv = (char_t**)(stack_start_addr - 10 * sizeof(char_t*));
		pos = (u64_t)dargv;

		for(i = 0; i < 10 && argv[i] != nullptr; i++)
		{
			len = strnlen_user(argv[i], 1024) + 1;
			strcpy((char_t*)(pos - len), argv[i]);
			dargv[i] = (char_t*)(pos - len);
			pos -= len;
		}

		stack_start_addr = pos - 10;
		regs->rdi = i; // argc
		regs->rsi = (u64_t)dargv; // argv
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

	// go to ret_system_call
	return retval;
}
