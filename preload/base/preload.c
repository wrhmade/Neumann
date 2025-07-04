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
    boxfill(binfo->vram,binfo->scrnx,0,0,binfo->scrnx-1,binfo->scrny-1,0x555555);
    putstr_ascii(binfo->vram,binfo->scrnx,0,0,0xAAAAAA,"Neumann Preload");
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165-1,binfo->scrny/2-50-1,binfo->scrnx/2+165+1,binfo->scrny/2+50+1,0x000000);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2-165,binfo->scrny/2+50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2-165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xAAAAAA);

    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx/2-92,binfo->scrny/2-8,0x555555,"Preparing the system...");

    init_mem();

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165-1,binfo->scrny/2-50-1,binfo->scrnx/2+165+1,binfo->scrny/2+50+1,0x000000);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2-165,binfo->scrny/2+50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2-165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xAAAAAA);

    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx/2-68,binfo->scrny/2-8,0x555555,"Loading kernel...");

    fileinfo_t finfo;
    if(fat16_open_file(&finfo,"kernel.bin")!=0)
    {
        boxfill(binfo->vram,binfo->scrnx,0,0,binfo->scrnx-1,binfo->scrny-1,0x000000);
        putstr_ascii(binfo->vram,binfo->scrnx,0,0,0xFFFFFF,"kernel.bin not found.");
        for(;;);
    }
    char *buf=(char *)malloc(sizeof(char)*(finfo.size+5));
    fat16_read_file(&finfo,buf);

  

    
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165-1,binfo->scrny/2-50-1,binfo->scrnx/2+165+1,binfo->scrny/2+50+1,0x000000);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2-165,binfo->scrny/2+50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2-165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xAAAAAA);

    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx/2-40,binfo->scrny/2-8,0x555555,"Parsing...");

    uint32_t entry = load_elf((Elf32_Ehdr *)buf);

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165-1,binfo->scrny/2-50-1,binfo->scrnx/2+165+1,binfo->scrny/2+50+1,0x000000);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2-165,binfo->scrny/2+50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2-165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xAAAAAA);

    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx/2-64,binfo->scrny/2-8,0x555555,"Kernel is ready");
    free(buf);

	  boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165-1,binfo->scrny/2-50-1,binfo->scrnx/2+165+1,binfo->scrny/2+50+1,0x000000);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2-165,binfo->scrny/2+50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xFFFFFF);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2+165,binfo->scrny/2+50,0x555555);

    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-165,binfo->scrny/2+50,binfo->scrnx/2-165,binfo->scrny/2+50,0xAAAAAA);
    boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2+165,binfo->scrny/2-50,binfo->scrnx/2+165,binfo->scrny/2-50,0xAAAAAA);

    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx/2-40,binfo->scrny/2-8,0x555555,"Jumping...");

    char s[200];
    sprintf(s,"Entry:0x%08x",entry);
    char *p=(char *)entry;
    putstr_ascii(binfo->vram,binfo->scrnx,0,48,0xFFFFFF,s);
    sprintf(s,"The first 10 bytes:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
    p[0]&0xff,p[1]&0xff,p[2]&0xff,p[3]&0xff,p[4]&0xff,p[5]&0xff,p[6]&0xff,p[7]&0xff,p[8]&0xff,p[9]&0xff);
    putstr_ascii(binfo->vram,binfo->scrnx,0,64,0xFFFFFF,s);


    _IN(1*8,entry);

    for(;;);
}
