/*
acpi.h
高级配置和电源管理接口驱动头文件
Copyright W24 Studio 
*/

#ifndef ACPI_H
#define ACPI_H
#include <stdint.h>

#define RSDP_STORE_ADDRESS_MIN 0x000E0000
#define RSDP_STORE_ADDRESS_MAX 0x000FFFFF
#define RSDP_MAGIC "RSD PTR "
#define FADT_MAGIC "FACP"
#define DSDT_MAGIC "DSDT"

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
}acpi_sdt_header;

typedef struct ACPI_RSDT
{
    acpi_sdt_header header;
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
    acpi_sdt_header h;
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

void acpi_init();
int acpi_poweroff();
#endif 