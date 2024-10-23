# 从elf文件中找到固定的符号
import argparse
from elftools.elf.elffile import ELFFile

def find_symbol_by_virtual_address(elf_file, virtual_address):
    with open(elf_file, 'rb') as f:
        elffile = ELFFile(f)
        symtab = elffile.get_section_by_name('.symtab')
        for symbol in symtab.iter_symbols():
            if symbol['st_value'] <= virtual_address < symbol['st_value'] + symbol['st_size']:
                return symbol.name
    return None

if __name__ == "__main__":
    # 固定的 ELF 文件名
    elf_file = "/home/steven/mine/build/kernel/Kernel.bin"

    parser = argparse.ArgumentParser(description='Find symbol by virtual address in ELF file.')
    parser.add_argument('virtual_address', type=int, help='Virtual address to search for (in decimal)')

    args = parser.parse_args()
    symbol_name = find_symbol_by_virtual_address(elf_file, args.virtual_address)

    if symbol_name:
        print(f'Symbol at address {args.virtual_address} is the [ {symbol_name} ]')
    else:
        print(f'No symbol found at address {args.virtual_address}.')

