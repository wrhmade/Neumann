/*
preload.c
准备加载内核
Copyright W24 Studio 
*/

#include <macro.h>
#include <binfo.h>
#include <graphic.h>
#include <mm.h>
#include <fat16.h>
#include <ELF.h>
#include <stddef.h>
#include <string.h>
#include <krnlcons.h>
#include <stdio.h>

void _IN(int cs, int eip);

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



void PRELDRMAIN()
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    binfo->boot_device=DEVICE_HD;
    krnlcons_change_backcolor(0x000000);
    krnlcons_change_forecolor(0xFFFFFF);
    krnlcons_display();

    krnlcons_putstr("Preparing the system...\n");

    init_mem();

    krnlcons_putstr("Loading kernel...\n");

    fileinfo_t finfo;
    if(fat16_open_file(&finfo,"kernel.bin")!=0)
    {
        krnlcons_putstr("[Error] kernel.bin not found.\n");
        for(;;);
    }
    char *buf=(char *)malloc(sizeof(char)*(finfo.size+5));
    fat16_read_file(&finfo,buf);
    
    krnlcons_putstr("Parsing...\n");

    uint32_t entry = load_elf((Elf32_Ehdr *)buf);

    krnlcons_putstr("Kernel is ready\n");
    binfo->kernel_elf_base = (uint32_t *)buf;

    krnlcons_putstr("Jumping...\n");

    char s[200];
    sprintf(s,"Entry:0x%08x\n",entry);
    char *p=(char *)entry;
    krnlcons_putstr(s);
    sprintf(s,"The first 10 bytes:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
    p[0]&0xff,p[1]&0xff,p[2]&0xff,p[3]&0xff,p[4]&0xff,p[5]&0xff,p[6]&0xff,p[7]&0xff,p[8]&0xff,p[9]&0xff);
    krnlcons_putstr(s);


    _IN(1*8,entry);

    for(;;);
}
