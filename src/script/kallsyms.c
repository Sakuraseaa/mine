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

int main(int argc, char** argv)
{
    read_map(stdin);
    write_src();
    return 0;
}

int read_symbol(FILE* filp, struct symbol_entry * sym_entry)
{
    char string[100];
    int rc;
    rc = fscanf(filp, "%llx %c %499s\n", &sym_entry->address, &sym_entry->type, string);
    if(rc != 3) {
        if(rc != EOF)
            fgets(string, 100, filp);
        return -1;
    }
    sym_entry->symbol = strdup(string);
    sym_entry->symbol_length = strlen(string) + 1;
    return 0;
}

void read_map(FILE* filp)
{
    int i;
    while(!feof(filp))
    {
        if( count >= size ) {
            size += 100;
            table = realloc(table, sizeof(*table) * size);
        }
        if(read_symbol(filp, &table[count]) == 0)
            count++;
    }
    for(i = 0; i < count; i++)
    {
        if(strcmp(table[i].symbol, "_text") == 0)
            _text = table[i].address;
        if(strcmp(table[i].symbol, "_etext") == 0)
            _etext = table[i].address;
    }
}

static int symbol_valid(struct symbol_entry* sym_entry)
{
    if(sym_entry->address < _text || sym_entry->address > _etext)
        return 0;
    
    return 1;
}

void write_src(void)
{
    unsigned long last_addr;
    int i, valid = 0;
    long position = 0;

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
        printf("\t.quad\t%#llx\n", table[i].address);
        valid++;
        last_addr = table[i].address;
    }
    putchar('\n');

    printf(".globl kallsyms_syms_num\n");
    printf(".align 8\n\n");
    printf("kallsyms_syms_num:\n"); // 数据列表项数 
    printf("\t.quad\t%d\n", valid);
    putchar('\n');

    printf(".globl kallsyms_index\n"); // 函数名起始索引
    printf(".align 8\n\n");
    printf("kallsyms_index:\n");
    for(i = 0, last_addr = 0; i < count; i++)
    {
        if(!symbol_valid(&table[i]))
            continue;
        if(table[i].address == last_addr)
            continue;
        printf("\t.quad\t%d\n", position);
        position += table[i].symbol_length;
        last_addr = table[i].address;
    }
    putchar('\n');

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
