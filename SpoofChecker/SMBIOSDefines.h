#pragma once

#include "pch.h"

typedef UINT8   SMBIOS_STRING;

/** The entire SMBIOS table with metadata */
struct RawSMBIOSData {
    BYTE    Used20CallingMethod;
    BYTE    SMBIOSMajorVersion;
    BYTE    SMBIOSMinorVersion;
    BYTE    DmiRevision;
    DWORD   Length;
    BYTE    SMBIOSTableData[];
};

/** Generic structure */
struct SMBIOSStruct {
    BYTE    Type;
    BYTE    Length;
    WORD    Handle;
};

/** Non-necessary structure */
struct SMBIOSStructNonRequired : SMBIOSStruct {

};

typedef struct _SMBIOSStructType0 : SMBIOSStruct
{
    SMBIOS_STRING   Vendor;
    SMBIOS_STRING   BiosVersion;
    UINT8           BiosSegment[2];
    SMBIOS_STRING   BiosReleaseDate;
    UINT8           BiosSize;
    UINT8           BiosCharacteristics[8];
} SMBIOSStructType0, * PSMBIOSStructType0;

typedef struct _SMBIOSStructType1 : SMBIOSStruct
{
    SMBIOS_STRING   Manufacturer;
    SMBIOS_STRING   ProductName;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;

    //
    // always byte copy this data to prevent alignment faults!
    //
    GUID			Uuid; // EFI_GUID == GUID?

    UINT8           WakeUpType;
} SMBIOSStructType1, * PSMBIOSStructType1;

typedef struct _SMBIOSStructType2 : SMBIOSStruct
{
    SMBIOS_STRING   Manufacturer;
    SMBIOS_STRING   ProductName;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;
    SMBIOS_STRING   AssetTag;
} SMBIOSStructType2, * PSMBIOSStructType2;

typedef struct _SMBIOSStructType3 : SMBIOSStruct
{
    SMBIOS_STRING   Manufacturer;
    UINT8           Type;
    SMBIOS_STRING   Version;
    SMBIOS_STRING   SerialNumber;
    SMBIOS_STRING   AssetTag;
    UINT8           BootupState;
    UINT8           PowerSupplyState;
    UINT8           ThermalState;
    UINT8           SecurityStatus;
    UINT8           OemDefined[4];
} SMBIOSStructType3, * PSMBIOSStructType3;

typedef struct _SMBIOSStructType4 : SMBIOSStruct
{
    UINT8           Socket;
    UINT8           ProcessorType;
    UINT8           ProcessorFamily;
    SMBIOS_STRING   ProcessorManufacture;
    UINT8           ProcessorId[8];
    SMBIOS_STRING   ProcessorVersion;
    UINT8           Voltage;
    UINT8           ExternalClock[2];
    UINT8           MaxSpeed[2];
    UINT8           CurrentSpeed[2];
    UINT8           Status;
    UINT8           ProcessorUpgrade;
    UINT8           L1CacheHandle[2];
    UINT8           L2CacheHandle[2];
    UINT8           L3CacheHandle[2];
    SMBIOS_STRING   SerialNumber;
    SMBIOS_STRING   AssetTag;
} SMBIOSStructType4, * PSMBIOSStructType4;

/** Structure of type 5 */
struct SMBIOSStructType5 : SMBIOSStruct {
    BYTE    ErrorDetectingMethod;
    BYTE    ErrorCorrectingCapability;
    BYTE    SupportedInterleave;
    BYTE    CurrentInterleave;
    BYTE    MaximumMemoryModuleSize;
    WORD    SupportedSpeeds;
    WORD    SupportedMemoryTypes;
    BYTE    MemoryModuleVoltage;
    BYTE    NumberOfAssociatedMemorySlots;
};

/** Structure of type 6 */
struct SMBIOSStructType6 : SMBIOSStruct {
    BYTE    SocketDesignation;
    BYTE    BankConnections;
    BYTE    CurrentSpeed;
    WORD    CurrentMemoryType;
    BYTE    InstalledSize;
    BYTE    EnabledSize;
    BYTE    ErrorStatus;
};

/** Necessary structure of type 7 */
struct SMBIOSStructType7 : SMBIOSStruct {
    BYTE    SocketDesignation;
    WORD    CacheConfiguration;
    WORD    MaximumCacheSize;
    WORD    InstalledSize;
    WORD    SupportedSRAMType;
    WORD    CurrentSRAMType;
    BYTE    CacheSpeed;
    BYTE    ErrorCorrectionType;
    BYTE    SystemCachetype;
    BYTE    Associativity;
};

/** Structure of type 8 */
struct SMBIOSStructType8 : SMBIOSStruct {
    BYTE    InternalReferenceDesignator;
    BYTE    InternalConnectorType;
    BYTE    ExternalReferenceDesignator;
    BYTE    ExternalConnectorType;
    BYTE    PortType;
};

/** Necessary structure of type 9 */
struct SMBIOSStructType9 : SMBIOSStruct {
    BYTE    SlotDesignation;
    BYTE    SlotType;
    BYTE    SlotDataBusWidth;
    BYTE    CurrentUsage;
    BYTE    SlotLength;
    WORD    SlotID;
    BYTE    SlotCharacteristics1;
    BYTE    SlotCharacteristics2;
    WORD    SegmentGroupNumber;
    BYTE    BusNumber;
    BYTE    DeviceNumber;
};

/** Structure of type 10 */
struct SMBIOSStructType10 : SMBIOSStruct {
    BYTE    DeviceType;
    BYTE    DescriptionString;
};

/** Structure of type 11 */
struct SMBIOSStructType11 : SMBIOSStruct {
    BYTE    Count;
};

/** Structure of type 12 */
struct SMBIOSStructType12 : SMBIOSStruct {
    BYTE    Count;
};

/** Structure of type 13 */
struct SMBIOSStructType13 : SMBIOSStruct {
    BYTE    InstalledLanguages;
    BYTE    Flags;
    BYTE    Reserved[15];
    BYTE    CurrentLanguage;
};

/** Structure of type 14 */
struct SMBIOSStructType14 : SMBIOSStruct {
    BYTE    GroupName;
    BYTE    ItemType;
    WORD    ItemHandle;
};

/** Structure of type 15 */
struct SMBIOSStructType15 : SMBIOSStruct {
    WORD    LogAreaLength;
    WORD    LogHeaderStartOffset;
    WORD    LogDataStartOffset;
    BYTE    AccessMethod;
    BYTE    LogStatus;
    WORD    LogChangeToken[2];
    WORD    AccessMethodAddress[2];
    BYTE    LogHeaderFormat;
    BYTE    NumberOfSupportedLogTypeDescriptors;
    BYTE    LengthOfEachLogTypeDescriptor;
};

/** Necessary structure of type 16 */
struct SMBIOSStructType16 : SMBIOSStruct {
    BYTE    Location;
    BYTE    Use;
    BYTE    MemoryErrorCorrection;
    BYTE    MaximumCapacity[4];
    WORD    MemoryErrorInformationHandle;
    WORD    NumberOfMemoryDevices;
    BYTE    ExtendedMaximumCapacity[8];
};

/** Necessary structure of type 17 */
typedef struct _SMBIOSStructType17 : SMBIOSStruct
{
    UINT16 PhysicalArrayHandle;
    UINT16 ErrorInformationHandle;
    UINT16 TotalWidth;
    UINT16 DataWidth;
    UINT16 Size;
    UINT8 FormFactor;
    UINT8 DeviceSet;
    SMBIOS_STRING DeviceLocator;
    SMBIOS_STRING BankLocator;
    UINT8 MemoryType;
    UINT16 TypeDetail;
    // 2.3+
    UINT16 Speed;
    SMBIOS_STRING Manufacturer;
    SMBIOS_STRING SerialNumber;
    SMBIOS_STRING AssetTagNumber;
    SMBIOS_STRING PartNumber;
    // 2.6+
    UINT8 Attributes;
    // 2.7+
    UINT32 ExtendedSize;
    UINT16 ConfiguredClockSpeed;
    // 2.8+
    UINT16 MinimumVoltage;
    UINT16 MaximumVoltage;
    UINT16 ConfiguredVoltage;
} SMBIOSStructType17, * PSMBIOSStructType17;

/** Necessary structure of type 19 */
struct SMBIOSStructType19 : SMBIOSStruct {
    BYTE    StartingAddress[4];
    BYTE    EndingAddress[4];
    WORD    MemoryArrayHandle;
    BYTE    PartitionWidth;
    BYTE    ExtendedStartingAddress[8];
    BYTE    ExtendedEndingAddress[8];
};

/** Necessary structure of type 32 */
struct SMBIOSStructType32 : SMBIOSStruct {
    BYTE    Reserved[6];
    BYTE    BootStatus[10];
};