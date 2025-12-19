#pragma once

#include "pch.h"

class HID
{
private:
    static void PrintHidString(HANDLE DeviceHandle, DWORD IoctlCode, const std::string& Label);
    static void PrintCollectionInfo(HANDLE DeviceHandle);
    static void GetDriverFromHidDevicePath(const std::wstring& DevicePath);
    static void ProcessDevice(const std::wstring& DevicePath, const std::wstring& DeviceDescription);

public:
    static void QueryAllInfo();
};