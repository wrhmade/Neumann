/*
cpu.h
CPU相关头文件
Copyright W24 Studio 
*/

#ifndef CPU_H
#define CPU_H
#include <stddef.h>
// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD "AuthenticAMD"
#define CPUID_VENDOR_INTEL "GenuineIntel"

bool cpu_check_cpuid();

typedef struct CPU_VENDOR
{
    uint32_t max_value;
    char info[13];
} __attribute__((packed)) cpu_vendor_t;

void cpu_vendor_id(cpu_vendor_t *item);

typedef struct CPU_VERSION
{
    uint8_t stepping : 4;
    uint8_t model : 4;
    uint8_t family : 4;
    uint8_t type : 2;
    uint8_t RESERVED : 2;
    uint8_t emodel : 4;
    uint8_t efamily0 : 4;
    uint8_t efamily1 : 4;
    uint8_t RESERVED1 : 4;

    uint8_t brand_index;
    uint8_t clflush;
    uint8_t max_num;
    uint8_t apic_id;

    // ECX;
    uint8_t SSE3 : 1;       // 0
    uint8_t PCLMULQDQ : 1;  // 1
    uint8_t DTES64 : 1;     // 2
    uint8_t MONITOR : 1;    // 3
    uint8_t DS_CPL : 1;     // 4
    uint8_t VMX : 1;        // 5
    uint8_t SMX : 1;        // 6
    uint8_t EIST : 1;       // 7
    uint8_t TM2 : 1;        // 8
    uint8_t SSSE3 : 1;      // 9
    uint8_t CNXT_ID : 1;    // 10
    uint8_t SDBG : 1;       // 11
    uint8_t FMA : 1;        // 12
    uint8_t CMPXCHG16B : 1; // 13
    uint8_t xTPR : 1;       // 14
    uint8_t PDCM : 1;       // 15
    uint8_t RESERVED2 : 1;   // 16
    uint8_t PCID : 1;       // 17
    uint8_t DCA : 1;        // 18
    uint8_t SSE4_1 : 1;     // 19
    uint8_t SSE4_2 : 1;     // 20
    uint8_t x2APIC : 1;     // 21
    uint8_t MOVBE : 1;      // 22
    uint8_t POPCNT : 1;     // 23
    uint8_t TSCD : 1;       // 24
    uint8_t AESNI : 1;      // 25
    uint8_t XSAVE : 1;      // 26
    uint8_t OSXSAVE : 1;    // 27
    uint8_t AVX : 1;        // 28
    uint8_t F16C : 1;       // 29
    uint8_t RDRAND : 1;     // 30
    uint8_t RESERVED3 : 1;   // 31

    // EDX
    uint8_t FPU : 1;      // 0 x87 FPU on Chip
    uint8_t VME : 1;      // 1 Virtual-8086 Mode Enhancement
    uint8_t DE : 1;       // 2 Debugging Extensions
    uint8_t PSE : 1;      // 3 Page Size Extensions
    uint8_t TSC : 1;      // 4 Time Stamp Counter
    uint8_t MSR : 1;      // 5 RDMSR and WRMSR Support
    uint8_t PAE : 1;      // 6 Physical Address Extensions
    uint8_t MCE : 1;      // 7 Machine Check Exception
    uint8_t CX8 : 1;      // 8 CMPXCHG8B Inst.
    uint8_t APIC : 1;     // 9 APIC on Chip
    uint8_t RESERVED4 : 1; // 10
    uint8_t SEP : 1;      // 11 SYSENTER and SYSEXIT
    uint8_t MTRR : 1;     // 12 Memory Type Range Registers
    uint8_t PGE : 1;      // 13 PTE Global Bit
    uint8_t MCA : 1;      // 14 Machine Check Architecture
    uint8_t CMOV : 1;     // 15 Conditional Move/Compare Instruction
    uint8_t PAT : 1;      // 16 Page Attribute Table
    uint8_t PSE36 : 1;    // 17 Page Size Extension
    uint8_t PSN : 1;      // 18 Processor Serial Number
    uint8_t CLFSH : 1;    // 19 CLFLUSH instruction
    uint8_t RESERVED5 : 1; // 20
    uint8_t DS : 1;       // 21 Debug Store
    uint8_t ACPI : 1;     // 22 Thermal Monitor and Clock Ctrl
    uint8_t MMX : 1;      // 23 MMX Technology
    uint8_t FXSR : 1;     // 24 FXSAVE/FXRSTOR
    uint8_t SSE : 1;      // 25 SSE Extensions
    uint8_t SSE2 : 1;     // 26 SSE2 Extensions
    uint8_t SS : 1;       // 27 Self Snoop
    uint8_t HTT : 1;      // 28 Multi-threading
    uint8_t TM : 1;       // 29 Therm. Monitor
    uint8_t RESERVED6 : 1; // 30
    uint8_t PBE : 1;      // 31 Pend. Brk. EN.
} __attribute__((packed)) cpu_version_t;

void cpu_version(cpu_version_t *ver);
void cpu_get_model_name(char *model_name);
#endif