#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct symbol_entry
{
    unsigned long address;
    char type;
    char* symbol;
    int symbol_length;
}symbol_entry_T;

symbol_entry_T *table;
int size, count;
unsigned long _text, _etext;


void write_src(void);
void read_map(FILE* filp);
int read_symbol(FILE* filp, struct symbol_entry * sym_entry);

/**
 * @brief 
 * 
 * @param argc  接受 nm -n kernel.bin 的执行结果，作为该程序的运行参数 
 * @param argv 
 * @return int 
 */
int main(int argc, char** argv)
{
    read_map(stdin);
    write_src();
    return 0;
}

/**
 * @brief 从标准输入中解析每一条 nm -n 命令的执行结果，保存 函数名、函数地址，函数类型
 * 
 * @param filp 
 * @param sym_entry 
 * @return int 
 */
int read_symbol(FILE* filp, struct symbol_entry * sym_entry)
{
    char string[100];
    int rc;

    rc = fscanf(filp, "%lx %c %499s\n", &sym_entry->address, &sym_entry->type, string);
    
    if(rc != 3) {  // 没有读到正确的三个数据，则跳过该行数据
        if(rc != EOF)
            fgets(string, 100, filp);
        return -1;
    }
    
    sym_entry->symbol = strdup(string); // 复制函数名
    sym_entry->symbol_length = strlen(string) + 1; // 函数名+1， 因为 .asciz 生成的文件名称会带一个'\0'作为结尾
    return 0;
}

/**
 * @brief read_symbol的封装函数
 * 
 * @param filp 
 */
void read_map(FILE* filp)
{
    int i;
    while(!feof(filp))// 读到文件结尾符就停止
    {
        if( count >= size ) { 
            size += 100; 
            table = realloc(table, sizeof(*table) * size); // 存储 nm 命令结果的内存，每次以100为步进单位，得到申请存储空气
        }
        if(read_symbol(filp, &table[count]) == 0) // 去解析 nm 命令结果
            count++;    // 每解析一条，计数加一
    }

    // 得到代码段的首位地址
    for(i = 0; i < count; i++)
    {
        if(strcmp(table[i].symbol, "_text") == 0)
            _text = table[i].address;
        if(strcmp(table[i].symbol, "_etext") == 0)
            _etext = table[i].address;
    }
}


// 检测符号地址是否在代码段之中
static int symbol_valid(struct symbol_entry* sym_entry)
{
    if(sym_entry->address < _text || sym_entry->address > _etext)
        return 0;
    
    return 1;
}

/**
 * @brief 生成符号文件。文件中保存了符号名，符号长度，符号地址索引，符号对应的虚拟地址
 * 
 */
void write_src(void)
{
    unsigned long last_addr;
    int i, valid = 0;
    long position = 0;

    // 生成标识符起始地址
    printf(".section .rodata\n\n");
    printf(".global kallsyms_addresses\n"); // 标识符起始地址
    printf(".align 8\n\n");
    printf("kallsyms_addresses:\n");

    for(i = 0, last_addr = 0; i < count; i++)
    {
        if(!symbol_valid(&table[i]))
            continue;
        if(table[i].address == last_addr)
            continue;
        printf("\t.quad\t%#lx\n", table[i].address);
        valid++;
        last_addr = table[i].address;
    }
    putchar('\n');

    // 生成标识符数量
    printf(".globl kallsyms_syms_num\n");
    printf(".align 8\n\n");
    printf("kallsyms_syms_num:\n"); // 数据列表项数 
    printf("\t.quad\t%d\n", valid);
    putchar('\n');

    // 生成标识符起始索引
    printf(".globl kallsyms_index\n"); // 函数名起始索引
    printf(".align 8\n\n");
    printf("kallsyms_index:\n");
    for(i = 0, last_addr = 0; i < count; i++)
    {
        if(!symbol_valid(&table[i]))
            continue;
        if(table[i].address == last_addr)
            continue;
        printf("\t.quad\t%ld\n", position);
        position += table[i].symbol_length;
        last_addr = table[i].address;
    }
    putchar('\n');

    // 生成标识符名称列表
    printf(".globl kallsyms_names\n"); // 函数名缓冲区
    printf(".align 8\n\n");
    printf("kallsyms_names:\n");
    for(i = 0, last_addr = 0; i < count; i++)
    {
        if(!symbol_valid(&table[i]))
            continue;
        if(table[i].address == last_addr)
            continue;
        printf("\t.asciz\t\"%s\"\n",table[i].symbol);
        last_addr = table[i].address;
    }
    putchar('\n');

}
