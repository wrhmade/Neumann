/*
elf.c
ELF解析器
Copyright W24 Studio 
*/

#include <ELF.h>
#include <string.h>

bool elf32Validate(Elf32_Ehdr *hdr) {
  return hdr->e_ident[EI_MAG0] == ELFMAG0 && hdr->e_ident[EI_MAG1] == ELFMAG1 &&
         hdr->e_ident[EI_MAG2] == ELFMAG2 && hdr->e_ident[EI_MAG3] == ELFMAG3;
}

void load_segment(Elf32_Phdr *phdr, void *elf) {
  //klogf("%08x %08x %d\n", phdr->p_vaddr, phdr->p_offset, phdr->p_filesz);
  memcpy((void *)phdr->p_vaddr, elf + phdr->p_offset, phdr->p_filesz);
  if (phdr->p_memsz > phdr->p_filesz) { // 这个是bss段
    memset((void *)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
  }
}

uint32_t load_elf(Elf32_Ehdr *hdr) {
  Elf32_Phdr *phdr = (Elf32_Phdr *)((uint32_t)hdr + hdr->e_phoff);
  for (int i = 0; i < hdr->e_phnum; i++) {
    load_segment(phdr, (void *)hdr);
    phdr++;
  }
  return hdr->e_entry;
}
