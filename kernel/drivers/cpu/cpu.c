/*
cpu.c
CPU相关
Copyright W24 Studio 
*/

#include <cpu.h>
#include <stddef.h>
#include <acpi.h>

#pragma GCC optimize("00") //硬件处理不开优化

// 检测是否支持 cpuid 指令
bool cpu_check_cpuid()
{
    bool ret;
    asm volatile(
        "pushfl \n" // 保存 eflags

        "pushfl \n"                   // 得到 eflags
        "xorl $0x00200000, (%%esp)\n" // 反转 ID 位
        "popfl\n"                     // 写入 eflags

        "pushfl\n"                  // 得到 eflags
        "popl %%eax\n"              // 写入 eax
        "xorl (%%esp), %%eax\n"     // 将写入的值与原值比较
        "andl $0x00200000, %%eax\n" // 得到 ID 位
        "shrl $21, %%eax\n"         // 右移 21 位，得到是否支持

        "popfl\n" // 恢复 eflags
        : "=a"(ret));
    return ret;
}

// 得到供应商 ID 字符串
void cpu_vendor_id(cpu_vendor_t *item)
{
    asm volatile(
        "cpuid \n"
        : "=a"(*((uint32_t *)item + 0)),
          "=b"(*((uint32_t *)item + 1)),
          "=d"(*((uint32_t *)item + 2)),
          "=c"(*((uint32_t *)item + 3))
        : "a"(0));
    item->info[12] = 0;
}

void cpu_version(cpu_version_t *item)
{
    asm volatile(
        "cpuid \n"
        : "=a"(*((uint32_t *)item + 0)),
          "=b"(*((uint32_t *)item + 1)),
          "=c"(*((uint32_t *)item + 2)),
          "=d"(*((uint32_t *)item + 3))
        : "a"(1));
}

void cpuid(unsigned int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	*eax = op;
	*ecx = 0;
	asm volatile("cpuid"
				: "=a" (*eax),				//杈撳嚭鍙傛暟
				"=b" (*ebx),
				"=c" (*ecx),
				"=d" (*edx)
				: "0" (*eax), "2" (*ecx)	//杈撳叆鍙傛暟
				: "memory");
}

void cpu_get_model_name(char *model_name)
{
	unsigned int *v = (unsigned int *) model_name;
	cpuid(0x80000002, &v[0], &v[1], &v[2], &v[3]);
	cpuid(0x80000003, &v[4], &v[5], &v[6], &v[7]);
	cpuid(0x80000004, &v[8], &v[9], &v[10], &v[11]);
	model_name[48] = 0;
}

int cpu_x2apic(void)
{
    #ifndef DISABLE_X2APIC
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x00000001, &eax, &ebx, &ecx, &edx);
    return (ecx & (1 << 21));
    #else
    return 0;
    #endif
}
