/*
acpi.h
高级配置和电源管理接口驱动头文件
Copyright W24 Studio 
*/

#ifndef ACPI_H
#define ACPI_H
#include <stdint.h>

#define DISABLE_X2APIC


#define RSDP_STORE_ADDRESS_MIN 0x000E0000
#define RSDP_STORE_ADDRESS_MAX 0x000FFFFF
#define RSDP_MAGIC "RSD PTR "
#define FADT_MAGIC "FACP"
#define DSDT_MAGIC "DSDT"
#define APIC_MAGIC "APIC"
#define HPET_MAGIC "HPET"

/* start:APIC */
#define MADT_APIC_CPU 0x00
#define MADT_APIC_IO  0x01
#define MADT_APIC_INT 0x02
#define MADT_APIC_NMI 0x03

#define LAPIC_REG_ID            32
#define LAPIC_REG_TIMER_CURCNT  0x390
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER         0x320
#define LAPIC_REG_SPURIOUS      0xf0
#define LAPIC_REG_TIMER_DIV     0x3e0

#define APIC_ICR_LOW  0x300
#define APIC_ICR_HIGH 0x310
/* end:APIC */

typedef struct ACPI_RSDP
{
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
}acpi_rsdp_t;

typedef struct ACPI_SDT_Header {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
}acpi_sdt_header_t;

typedef struct ACPI_RSDT
{
    acpi_sdt_header_t header;
    int entry;
}acpi_rsdt_t;

typedef struct ACPI_GenericAddressStructure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
}acpi_gas_t;

typedef struct ACPI_FADT
{
    acpi_sdt_header_t h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    acpi_gas_t ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
  
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    acpi_gas_t X_PM1aEventBlock;
    acpi_gas_t X_PM1bEventBlock;
    acpi_gas_t X_PM1aControlBlock;
    acpi_gas_t X_PM1bControlBlock;
    acpi_gas_t X_PM2ControlBlock;
    acpi_gas_t X_PMTimerBlock;
    acpi_gas_t X_GPE0Block;
    acpi_gas_t X_GPE1Block;
}acpi_fadt_t;

/* start:APIC */
typedef struct {
    acpi_sdt_header_t h;
    uint32_t local_apic_address;
    uint32_t flags;
    void *entries;
} __attribute__((packed)) MADT;

struct madt_hander {
    uint8_t entry_type;
    uint8_t length;
} __attribute__((packed));

struct madt_io_apic {
    struct madt_hander h;
    uint8_t apic_id;
    uint8_t reserved;
    uint32_t address;
    uint32_t gsib;
} __attribute__((packed));

struct madt_local_apic {
    struct madt_hander h;
    uint8_t ACPI_Processor_UID;
    uint8_t local_apic_id;
    uint32_t flags;
};

typedef struct {
    uint8_t vector;
    uint32_t irq;
} ioapic_routing;

typedef struct madt_hander MadtHeader;
typedef struct madt_io_apic MadtIOApic;
typedef struct madt_local_apic MadtLocalApic;
/* end:APIC */

/* start:HPET */
struct generic_address {
        uint8_t address_space;
        uint8_t bit_width;
        uint8_t bit_offset;
        uint8_t access_size;
        uint32_t address;
} __attribute__((packed));

struct hpet {
        acpi_sdt_header_t h;
        uint32_t event_block_id;
        struct generic_address base_address;
        uint16_t clock_tick_unit;
        uint8_t page_oem_flags;
} __attribute__((packed));

typedef struct {
        uint64_t configurationAndCapability;
        uint64_t comparatorValue;
        uint64_t fsbInterruptRoute;
        uint64_t unused;
} __attribute__((packed)) HpetTimer;

typedef struct {
        uint64_t generalCapabilities;
        uint64_t reserved0;
        uint64_t generalConfiguration;
        uint64_t reserved1;
        uint64_t generalIntrruptStatus;
        uint8_t reserved3[0xc8];
        uint64_t mainCounterValue;
        uint64_t reserved4;
        HpetTimer timers[];
} __attribute__((packed)) volatile HpetInfo;

typedef struct hpet Hpet;
/* end:HPET */

void acpi_init();
int acpi_poweroff();
int acpi_reboot();

/* start:APIC */
void apic_init(MADT *madt);
void disable_pic(void);
void ioapic_write(uint32_t reg, uint32_t value);
uint32_t ioapic_read(uint32_t reg);
void ioapic_add(ioapic_routing *routing);
void lapic_write(uint32_t reg, uint32_t value);
uint32_t lapic_read(uint32_t reg);
uint64_t lapic_id(void);
void local_apic_init(void);
void io_apic_init(void);
void send_eoi(void);
void lapic_timer_stop(void);
void send_ipi(uint32_t apic_id, uint32_t command);
void apic_init(MADT *madt);
/* end:APIC */

/* start:HPET */
uint64_t nano_time(void);
void hpet_init(Hpet *hpet);
/* end:HPET */

//检测是否支持x2APIC
int get_apic_mode();
#endif
