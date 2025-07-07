/*
acpi.c
高级配置和电源管理接口驱动
Copyright W24 Studio 
*/

#include <acpi.h>
#include <krnlcons.h>
#include <string.h>
#include <stdio.h>
#include <io.h>

acpi_rsdp_t *rsdp;
acpi_rsdt_t *rsdt;
acpi_fadt_t *fadt;
acpi_sdt_header *dsdt;

static int acpi_ok=0;

int rsdp_check()
{
    int length=rsdp->Length;
    uint8_t sum;
    uint8_t *addr=(uint8_t *)rsdp;
    for(int i=0;i<length;i++)
    {
        sum+=addr[i];
    }
    return sum==0;
}

int rsdt_check()
{
    int length=rsdt->header.Length;
    uint8_t sum;
    uint8_t *addr=(uint8_t *)rsdt;
    for(int i=0;i<length;i++)
    {
        sum+=addr[i];
    }
    return sum==0;
}

int fadt_check()
{
    int length=fadt->h.Length;
    uint8_t sum;
    uint8_t *addr=(uint8_t *)fadt;
    for(int i=0;i<length;i++)
    {
        sum+=addr[i];
    }
    return sum==0;
}

int find_rsdp()
{
    for(int i=RSDP_STORE_ADDRESS_MIN;i<=RSDP_STORE_ADDRESS_MAX;i++)
    {
        if(!memcmp((char *)i,RSDP_MAGIC,8))
        {
            if(rsdp_check())
            {
                rsdp=(acpi_rsdp_t *)i;
                return 0;
            }
        }
    }
    return -1;
}

acpi_sdt_header *acpi_find_table(char *magic,int len)
{
    int entries=(rsdt->header.Length - sizeof(rsdt->header))/4;
    for (int i = 0; i < entries; i++)
    {
        acpi_sdt_header *h = (acpi_sdt_header *)((uint32_t)rsdt->entry+i*4);
        if (!memcmp(h->Signature,magic,len))
        {
            return h;
        }
    }
    return NULL;
}

int find_fadt()
{
    fadt=(acpi_fadt_t *)acpi_find_table(FADT_MAGIC,4);
    return (fadt==NULL)?-1:0;
}

int find_dsdt()
{
    dsdt=acpi_find_table(DSDT_MAGIC,4);
    return (dsdt==NULL)?-1:0;
}

void acpi_init()
{
    char s[300];
    if(find_rsdp()==-1)
    {
        krnlcons_putstr_color("Cannot find RSDP.\n",0xFFFFFF,0x000000);
        return;
    }
    sprintf(s,"INFO:RSDP ADDR:0x%08p\n",rsdp);
    krnlcons_putstr(s);

    rsdt=(acpi_rsdt_t *)rsdp->RsdtAddress;
    if(!rsdt_check())
    {
        krnlcons_putstr_color("RSDT Error.\n",0xFFFFFF,0x000000);
        return;
    }
    if(find_fadt()==-1)
    {
        krnlcons_putstr_color("Cannot find FADT.\n",0xFFFFFF,0x000000);
        return;
    }
    sprintf(s,"INFO:FADT ADDR:0x%08p\n",fadt);
    krnlcons_putstr(s);
    if(!fadt_check())
    {
        krnlcons_putstr_color("FADT Error.\n",0xFFFFFF,0x000000);
        return;
    }
    if(!(fadt->SMI_CommandPort==0 && fadt->AcpiDisable==0 && fadt->AcpiEnable==0 && io_in16(fadt->PM1aControlBlock)&1==0))
    {
        krnlcons_putstr("INFO:Setting up...\n");
        io_out8(fadt->SMI_CommandPort,fadt->AcpiEnable);
        while(io_in16(fadt->PM1aControlBlock)&1==0);
        if(fadt->PM1bControlBlock)
        {
            while(io_in16(fadt->PM1bControlBlock)&1==0);
        }
    }
    acpi_ok=1;
    krnlcons_putstr("INFO:OK.\n");
}

int acpi_poweroff()
{
    if(!acpi_ok)return -1;
    find_dsdt();
    int i;
    char *S5Addr = (char *)dsdt;
	int dsdtLength = (dsdt->Length - sizeof(acpi_sdt_header))/4;
    unsigned short SLP_TYPa, SLP_TYPb;
    for(i=0;i<dsdtLength;i++)
    {
        if (!memcmp(S5Addr,"_S5_",4)) break;
		S5Addr++;
    }
    if (i < dsdtLength)
	{
		if ( ( *(S5Addr-1) == 0x08 || ( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') ) && *(S5Addr+4) == 0x12 )
		{
			S5Addr+=5;
			S5Addr+=((*S5Addr&0xc0)>>6)+2;
			
			if (*S5Addr == 0x0a) S5Addr++;
			SLP_TYPa = *(S5Addr)<<10;
			S5Addr++;
			
			if (*S5Addr == 0x0a) S5Addr++;
			SLP_TYPb = *(S5Addr)<<10;
			S5Addr++;
		}
		io_out16(fadt->PM1aControlBlock, SLP_TYPa | 1<<13);
		if (fadt->PM1bControlBlock != 0)
		{
			io_out16(fadt->PM1bControlBlock, SLP_TYPb | 1<<13);
		}
	}
    return 1;
}

int acpi_reboot()
{
    io_out8(0x64,0xfe);
}