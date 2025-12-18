#pragma once

#include "pch.h"

class ARP
{
private:
    static std::string GetArpTypeString(unsigned int dwType);
    static std::string FormatMac(const BYTE* addr, DWORD len);

    static void QueryAdapters(std::map<DWORD, std::string>& adapterIpMap);
    static void QueryTable(const std::map<DWORD, std::string>& adapterIpMap);

public:
    static void QueryAllInfo();
    static std::map<std::string, std::string> GetArpMap();
};
