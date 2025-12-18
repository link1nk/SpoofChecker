#pragma once

#include "pch.h"

class SMBIOS
{
    friend class Registry;
    enum class INITIALIZATION_TYPE { FirmwareTable, Registry };

private:
    inline static RawSMBIOSData* m_RawData = nullptr;

    static char* GetString(const SMBIOSStruct* Header, UINT8 StringIndex);
    static RawSMBIOSData* GetRawData();
    static const SMBIOSStruct* GetNextStruct(const SMBIOSStruct* curStruct);

    static void QueryBiosInfo(const SMBIOSStructType0* Table);
    static void QuerySystemInfo(const SMBIOSStructType1* Table);
    static void QueryBaseboardInfo(const SMBIOSStructType2* Table);
    static void QueryEnclosureInfo(const SMBIOSStructType3* Table);
    static void QueryProcessorInfo(const SMBIOSStructType4* Table);
    static void QueryMemoryDeviceInfo(const SMBIOSStructType17* Table);

    static bool Initialize();

public:
    static void QueryAllInfo();
};
