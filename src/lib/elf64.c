typedef unsigned int Elf64_Word;
typedef unsigned long Elf64_Addr, Elf64_Off;
typedef unsigned short Elf64_Half;

#define EI_NIDENT 16
// 32位elf头
typedef struct
{
    unsigned char e_ident[EI_NIDENT]; // 魔术， elf文件类型（标识是32位还是64位），指定大端还是小端，ELF头的版本信息，e_ident7 ~ 15位不使用
    Elf64_Half e_type;         // 文件类型 Relocatable file(.o), Executable(.out), Shared Object File(.so)
    Elf64_Half e_machine;      // CPU 平台类型
    Elf64_Word e_version;      // ELF 版本号
    Elf64_Addr e_entry;        // 执行该程序的入口地址
    Elf64_Off e_phoff;         // 程序头表在文件中的偏移
    Elf64_Off e_shoff;         // 段表在文件中的偏移
    Elf64_Word e_flags;         // ELF标志位，标识一些ELF文件平台相关的属性
    Elf64_Half e_ensize;       // ElF文件头本身的大小
    Elf64_Half e_phentsize;    // 程序头表的表项的大小(segment tbale)
    Elf64_Half e_phnum;        // 程序头表的表项的数量
    Elf64_Half e_shentsize;    // 段表的表项大小（section table）
    Elf64_Half e_shnum;        // 段表的表项数量
    Elf64_Half e_shstrndx;     // 段表字符串表所在的段在段表中的下标
} Elf64_Ehdr;

// 程序头表描述符 segment类型
typedef struct
{
    Elf64_Word p_type;   // segment 的类型
    Elf64_Word p_flags;  // segment 的权限，R， W， X
    Elf64_Off p_offset;  // segment 在文件中的偏移
    Elf64_Addr p_vaddr;  // segment 在第一个字节在进程虚拟地址空间的起始位置
    Elf64_Addr p_paddr;  // segment 的物理装载地址
    Elf64_Addr p_filesz; // segment 在ELF文件中所占空间的长度
    Elf64_Addr p_memsz;  // segment 在进程虚拟地址空间所占用的长度
    Elf64_Addr p_align;  // 对齐属性，实际对齐字节等于2的p_align次
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