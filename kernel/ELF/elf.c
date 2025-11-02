/*
elf.c
ELF解析器
Copyright W24 Studio 
*/

#include <ELF.h>
#include <string.h>
#include <mm.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))

bool elf32Validate(Elf32_Ehdr *ehdr)
{
    return memcmp(ehdr->e_ident, "\177ELF\1\1\1", 7)==0;
}

static void calc_load_range(Elf32_Ehdr *ehdr, uint32_t *first, uint32_t *last)
{
    Elf32_Phdr *phdr = (Elf32_Phdr *) ((uint32_t) ehdr + ehdr->e_phoff); // 第一个 program header 地址
    *first = 0xffffffff; // UINT32最大值
    *last = 0; // UINT32最小值

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) { // 遍历每一个 program header
        if (phdr[i].p_type != PT_LOAD) continue; // 只关心LOAD段
        *first = min(*first, phdr[i].p_vaddr);
        *last = max(*last, phdr[i].p_vaddr + phdr[i].p_memsz); // 每一个program header首尾取最值
    }
}

static void copy_load_segments(Elf32_Ehdr *ehdr, char *buf)
{
    Elf32_Phdr *phdr = (Elf32_Phdr *) ((uint32_t) ehdr + ehdr->e_phoff); // 第一个 program header 地址
    for (uint16_t i = 0; i < ehdr->e_phnum; i++) { // 遍历每一个 program header
        if (phdr[i].p_type != PT_LOAD) continue; // 只关心LOAD段
        uint32_t segm_in_file = (uint32_t) ehdr + phdr[i].p_offset; // 段在文件中的位置
        memcpy(buf + phdr[i].p_vaddr, (void *) segm_in_file, phdr[i].p_filesz); // 将文件中大小的部分copy过去
        uint32_t remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz; // 两者之差
        memset(buf + (phdr[i].p_vaddr + phdr[i].p_filesz), 0, remain_bytes); // 赋值为0
    }
}


int load_elf(Elf32_Ehdr *ehdr, char **buf, uint32_t *first, uint32_t *last)
{
    if (memcmp(ehdr->e_ident, "\177ELF\1\1\1", 7)) return -1; // 魔数不对，不予执行
    calc_load_range(ehdr, first, last); // 计算加载位移
    *buf = (char *) kmalloc(*last - *first + 5); // 用算得的大小分配内存
    copy_load_segments(ehdr, *buf); // 把 ELF 
    return ehdr->e_entry;
}

sym_info_t get_symbol_info(void *kernel_file_address, Elf32_Addr symbol_address)
{
    sym_info_t sym_info  = {};
    Elf32_Ehdr *ehdr     = kernel_file_address;
    Elf32_Shdr *shdr     = ehdr->e_shoff + kernel_file_address;
    const char *shstrtab = (const char *)(shdr[ehdr->e_shstrndx].sh_offset + kernel_file_address);
    Elf32_Sym *sym       = 0;
    const char *strtab   = 0;
    size_t sym_size      = 0;
    for (size_t i = 0; i < ehdr->e_shnum; ++i) {
        const char *sh_name = &shstrtab[shdr[i].sh_name];
        if (!strcmp(sh_name, ".symtab")) {
            sym      = (Elf32_Sym *)(shdr[i].sh_offset + kernel_file_address);
            sym_size = shdr[i].sh_size / shdr[i].sh_entsize;
        }
        if (!strcmp(sh_name, ".strtab")) strtab = (const char *)(shdr[i].sh_offset + kernel_file_address);
    }
    if (!sym || !strtab) return sym_info;
    for (size_t i = 0; i < sym_size; ++i) {
        if (((sym[i].st_info) & 0xf) != 2) continue;
        if (symbol_address - sym_info.addr <= symbol_address - sym[i].st_value) continue;
        const char *st_name = &strtab[sym[i].st_name];
        sym_info            = (sym_info_t) {st_name, sym[i].st_value};
    }
    if (symbol_address - sym_info.addr >= 0x1000) sym_info.name = 0;
    return sym_info;
}
