#pragma once

#include "pch.h"

class Volumes
{
private:
    inline static IOCTL_Base VolumesIOCTL;

    static void PrintVolumeSerial_BootSector(std::wstring volumePath);
    static void PrintVolumeSerial(const std::wstring& volumePath);
    static void PrintVolumeUniqueId(const std::wstring& volumePath);
    static void EnumerateVolumes();
    static void PrintVolumeSerial_FSCTL(const std::wstring& volumePath);
    static void PrintVolumesUniqueId_UsingMountMgr();

public:
    static void QueryAllInfo();
};