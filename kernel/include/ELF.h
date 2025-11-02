/*
elf.h
ELF解析器头文件
Copyright W24 Studio 
*/
#ifndef ELF_H
#define ELF_H
#include <stddef.h>
#include <stdint.h>
#define PT_LOAD 1

#define EI_NIDENT 16

typedef uint32_t Elf32_Word, Elf32_Off, Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef struct {
    Elf32_Word p_type;   // 当前header描述的段类型
    Elf32_Off  p_offset; // 段的第一个字节在文件中的偏移
    Elf32_Addr p_vaddr;  // 段在内存中的虚拟地址
    Elf32_Addr p_paddr;  // 段在内存中的物理地址，为兼容不进入保护模式的OS
    Elf32_Word p_filesz; // 段在文件中的长度
    Elf32_Word p_memsz;  // 段在内存中的长度
    Elf32_Word p_flags;  // 与段相关的标志
    Elf32_Word p_align;  // 确定段在文件和内存中如何对齐
} Elf32_Phdr;

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT]; // ELF特征标

    Elf32_Half e_type;      // 文件类型
    Elf32_Half e_machine;   // 运行至少需要的体系结构
    Elf32_Word e_version;   // 文件版本
    Elf32_Addr e_entry;     // 程序的入口点
    Elf32_Off  e_phoff;     // Program Header 表的偏移
    Elf32_Off  e_shoff;     // Section Header 表的偏移
    Elf32_Word e_flags;     // 对于32位系统为0
    Elf32_Half e_ehsize;    // ELF Header 的大小，单位字节
    Elf32_Half e_phentsize; // Program Header 的大小
    Elf32_Half e_phnum;     // Program Header 的数量
    Elf32_Half e_shentsize; // Section Header 的大小
    Elf32_Half e_shnum;     // Section Header 的数量
    Elf32_Half e_shstrndx;  // 包含 Section 名称的字符串表位于哪一项
} Elf32_Ehdr;

typedef struct {
    uint32_t sh_name;      // 节区名称的索引，指向节区字符串表
    uint32_t sh_type;      // 节区的类型，指定该节区的含义
    uint32_t sh_flags;     // 节区的标志，描述该节区的特性（如可读、可写、可执行等）
    uint32_t sh_addr;      // 节区在内存中的地址（如果被加载）
    uint32_t sh_offset;    // 节区在文件中的偏移
    uint32_t sh_size;      // 节区的大小
    uint32_t sh_link;      // 链接到的节区索引（例如，符号表的节区链接到字符串表的节区）
    uint32_t sh_info;      // 附加信息，通常与节区类型相关
    uint32_t sh_addralign; // 节区的地址对齐方式
    uint32_t sh_entsize;   // 如果该节区包含表格元素，则此字段指定每个元素的大小
} Elf32_Shdr;

typedef struct {
    uint32_t st_name;  // 符号名的索引，指向符号字符串表
    uint32_t st_value; // 符号的值（地址或其他标识）
    uint32_t st_size;  // 符号的大小
    uint8_t  st_info;  // 符号的类型和绑定信息（高4位为绑定信息，低4位为类型）
    uint8_t  st_other; // 保留，通常为0
    uint16_t st_shndx; // 符号所在的节区的索引
} Elf32_Sym;

typedef struct {
        const char *name;
        Elf32_Addr addr;
} sym_info_t;

#define ELF32_ST_BIND(val)		(((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)		((val) & 0xf)

#define STB_LOCAL	0		/* Local symbol */
#define STB_GLOBAL	1		/* Global symbol */
#define STB_WEAK	2		/* Weak symbol */
#define	STB_NUM		3		/* Number of defined types.  */
#define STB_LOOS	10		/* Start of OS-specific */
#define STB_GNU_UNIQUE	10		/* Unique symbol.  */
#define STB_HIOS	12		/* End of OS-specific */
#define STB_LOPROC	13		/* Start of processor-specific */
#define STB_HIPROC	15		/* End of processor-specific */


#define STT_NOTYPE	0		/* Symbol type is unspecified */
#define STT_OBJECT	1		/* Symbol is a data object */
#define STT_FUNC	2		/* Symbol is a code object */
#define STT_SECTION	3		/* Symbol associated with a section */
#define STT_FILE	4		/* Symbol's name is file name */
#define STT_COMMON	5		/* Symbol is a common data object */
#define STT_TLS		6		/* Symbol is thread-local data object*/
#define	STT_NUM		7		/* Number of defined types.  */
#define STT_LOOS	10		/* Start of OS-specific */
#define STT_GNU_IFUNC	10		/* Symbol is indirect code object */
#define STT_HIOS	12		/* End of OS-specific */
#define STT_LOPROC	13		/* Start of processor-specific */
#define STT_HIPROC	15		/* End of processor-specific */

#define SHN_UNDEF	0		/* Undefined section */
#define SHN_LORESERVE	0xff00		/* Start of reserved indices */
#define SHN_LOPROC	0xff00		/* Start of processor-specific */
#define SHN_BEFORE	0xff00		/* Order section before all others
					   (Solaris).  */
#define SHN_AFTER	0xff01		/* Order section after all others
					   (Solaris).  */
#define SHN_HIPROC	0xff1f		/* End of processor-specific */
#define SHN_LOOS	0xff20		/* Start of OS-specific */
#define SHN_HIOS	0xff3f		/* End of OS-specific */
#define SHN_ABS		0xfff1		/* Associated symbol is absolute */
#define SHN_COMMON	0xfff2		/* Associated symbol is common */
#define SHN_XINDEX	0xffff		/* Index is in extra table.  */
#define SHN_HIRESERVE	0xffff		/* End of reserved indices */

bool elf32Validate(Elf32_Ehdr *ehdr);
int load_elf(Elf32_Ehdr *ehdr, char **buf, uint32_t *first, uint32_t *last);
#endif