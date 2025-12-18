#pragma once

#include "pch.h"

class DISK
{
    enum class DISK_DEVICE_TYPE { PHYSICAL, SCSI };

    struct Vpd83Id
    {
        uint8_t codeSet;
        uint8_t idType;
        std::string decoded;
    };

private:
    inline static ULONG m_Number = -1;
    inline static HANDLE m_Handle = INVALID_HANDLE_VALUE;

    inline static IOCTL_Base m_DiskIOCTL;

    static PCHAR ConvertToString(DWORD* DiskData, ULONG FirstIndex, ULONG LastIndex, PCHAR Buffer);
    static void ConvertToString(WORD* data, int firstIndex, int lastIndex, char* out);
    static void SwapEndianness(PVOID Ptr, SIZE_T Size);

    static bool IsValid() { return m_Handle != INVALID_HANDLE_VALUE; }
    static int Number() { return m_Number; }

    static const char* IdTypeName(uint8_t idType);
    static std::string ParseVpd80(const std::vector<uint8_t>& vpd);
    static std::vector<Vpd83Id> ParseVpd83All(const std::vector<uint8_t>& vpd);
    static void PrintVpd83(const std::vector<uint8_t>& vpd);

    static HANDLE GetDevice(ULONG Number, DISK_DEVICE_TYPE Type);

    static void QuerySmart();
    static void QueryStorage();
    static void QueryAtaPassThrough();
    static void QueryScsiMiniport();
    static void QueryNvmeIdentify();
    static void QueryScsiPassThrough(UCHAR pageCode);
    static void QueryScsiPassThroughDirect(UCHAR pageCode);
    static void QueryDiskId();
    static void QueryPartitionInfo();
    static void QueryPowershellCommandInfo();

public:
    static void QueryAllInfo();
};