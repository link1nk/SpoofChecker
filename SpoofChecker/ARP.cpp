#include "pch.h"

std::string ARP::GetArpTypeString(unsigned int Type)
{
    switch (Type)
    {
    case MIB_IPNET_TYPE_OTHER:   return "other";
    case MIB_IPNET_TYPE_INVALID: return "invalid";
    case MIB_IPNET_TYPE_DYNAMIC: return "dynamic";
    case MIB_IPNET_TYPE_STATIC:  return "static";
    default:                     return "unknown";
    }
}

std::string ARP::FormatMac(const BYTE* Addr, DWORD Len)
{
    if (Len == 0)
    {
        return "N/A";
    }

    std::ostringstream StringStream;

    for (DWORD Index = 0; Index < Len; Index++)
    {
        if (Index > 0)
        {
            StringStream << "-";
        }
        StringStream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(Addr[Index]);
    }

    return StringStream.str();
}

void ARP::QueryAdapters(std::map<DWORD, std::string>& AdapterIpMap)
{
    ULONG Size = 0;
    GetAdaptersInfo(nullptr, &Size);

    std::vector<BYTE> Buffer(Size);
    PIP_ADAPTER_INFO AdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(Buffer.data());

    if (GetAdaptersInfo(AdapterInfo, &Size) == ERROR_SUCCESS)
    {
        PIP_ADAPTER_INFO Current = AdapterInfo;

        while (Current)
        {
            AdapterIpMap[Current->Index] = Current->IpAddressList.IpAddress.String;
            Current = Current->Next;
        }
    }
}

void ARP::QueryTable(const std::map<DWORD, std::string>& AdapterIpMap)
{
    ULONG Size = 0;

    if (GetIpNetTable(nullptr, &Size, TRUE) != ERROR_INSUFFICIENT_BUFFER)
    {
        std::cerr << "GetIpNetTable falhou (tamanho)." << std::endl;
        return;
    }

    std::vector<BYTE> Buffer(Size);
    PMIB_IPNETTABLE Table = reinterpret_cast<PMIB_IPNETTABLE>(Buffer.data());

    if (GetIpNetTable(Table, &Size, TRUE) != NO_ERROR)
    {
        std::cerr << "GetIpNetTable falhou (dados)." << std::endl;
        return;
    }

    DWORD LastIfIndex = (DWORD)-1;

    for (DWORD Index = 0; Index < Table->dwNumEntries; ++Index)
    {
        auto& Row = Table->table[Index];

        if (Row.dwIndex != LastIfIndex)
        {
            LastIfIndex = Row.dwIndex;
            auto Iterator = AdapterIpMap.find(Row.dwIndex);
            std::string InterfaceIp = (Iterator != AdapterIpMap.end()) ? Iterator->second : "unknown";

            std::ostringstream StringStream;

            StringStream << "\nInterface: " << InterfaceIp
                << " --- 0x" << std::hex << Row.dwIndex << std::dec << "\n";

            std::cout << dye::aqua(StringStream.str());

            StringStream.str("");
            StringStream << "Internet Address      Physical Address       Type\n";

            std::cout << dye::yellow(StringStream.str());
        }

        in_addr IpAddress;
        IpAddress.S_un.S_addr = Row.dwAddr;
        std::string IpString = inet_ntoa(IpAddress);
        std::string MacString = FormatMac(Row.bPhysAddr, Row.dwPhysAddrLen);

        std::cout
            << std::left
            << std::setfill(' ')
            << std::setw(22) << dye::light_purple(IpString)
            << std::setw(23) << dye::green(MacString)
            << dye::grey(GetArpTypeString(Row.dwType))
            << "\n";
    }

    std::cout << std::endl;
}

void ARP::QueryAllInfo()
{
    UTILS::PrintFixedBox("ARP");

    WSADATA WsaData;

    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
    {
        std::cerr << "WSAStartup falhou." << std::endl;
        return;
    }

    std::map<DWORD, std::string> AdapterIpMap;

    QueryAdapters(AdapterIpMap);
    QueryTable(AdapterIpMap);

    WSACleanup();
}

std::map<std::string, std::string> ARP::GetArpMap()
{
    std::map<std::string, std::string> ArpMap;

    ULONG Size = 0;
    GetIpNetTable(nullptr, &Size, TRUE);
    std::vector<BYTE> Buffer(Size);
    PMIB_IPNETTABLE Table = reinterpret_cast<PMIB_IPNETTABLE>(Buffer.data());

    if (GetIpNetTable(Table, &Size, TRUE) != NO_ERROR)
    {
        return ArpMap;
    }

    for (DWORD Index = 0; Index < Table->dwNumEntries; ++Index)
    {
        auto& Row = Table->table[Index];

        in_addr IpAddress;
        IpAddress.S_un.S_addr = Row.dwAddr;
        std::string IpString = inet_ntoa(IpAddress);

        ArpMap[IpString] = FormatMac(Row.bPhysAddr, Row.dwPhysAddrLen);
    }

    return ArpMap;
}