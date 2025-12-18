#pragma once

#include "pch.h"

class MAC
{
private:
    inline static IOCTL_Base MacIOCTL;

    static std::string FormatMac(BYTE* addr, ULONG len);

    static bool QueryMac(const char* serviceName);
    static bool QueryAdaptersInfo();
    static void QueryRouter();

public:
    static void QueryAllInfo();
    static std::vector<std::string> GetMacList();
    static std::string GetRouterMac();
};
