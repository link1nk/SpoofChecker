#include "pch.h"

std::string MAC::FormatMac(BYTE* Addr, ULONG Len)
{
    std::ostringstream StringStream;

    for (ULONG Index = 0; Index < Len; Index++)
    {
        if (Index > 0)
        {
            StringStream << "-";
        }

        StringStream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)Addr[Index];
    }

    return StringStream.str();
}

bool MAC::QueryMac(const char* ServiceName)
{
    char DevicePath[512];
    wsprintfA(DevicePath, "\\\\.\\%s", ServiceName);

    HANDLE DeviceHandle = CreateFileA(DevicePath, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, 0);

    if (DeviceHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    BYTE OutputBuffer[256] = { 0 };
    int Oid = OID_802_3_PERMANENT_ADDRESS;

    MacIOCTL.SetIOCTL(DeviceHandle, IOCTL_NDIS_QUERY_GLOBAL_STATS, "IOCTL_NDIS_QUERY_GLOBAL_STATS");

    if (MacIOCTL.IOCTL(&Oid, sizeof(Oid), OutputBuffer, sizeof(OutputBuffer)))
    {
        MacIOCTL.PrintMAC(OutputBuffer, 6, "Physical MAC", "OID_802_3_PERMANENT_ADDRESS");
    }

    Oid = OID_802_3_CURRENT_ADDRESS;

    if (MacIOCTL.IOCTL(&Oid, sizeof(Oid), OutputBuffer, sizeof(OutputBuffer)))
    {
        MacIOCTL.PrintMAC(OutputBuffer, 6, "Current MAC", "OID_802_3_CURRENT_ADDRESS");
    }

    CloseHandle(DeviceHandle);

    return true;
}

bool MAC::QueryAdaptersInfo()
{
    ULONG Size = 0;
    if (GetAdaptersInfo(nullptr, &Size) != ERROR_BUFFER_OVERFLOW)
    {
        return false;
    }

    PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO) new BYTE[Size];
    if (GetAdaptersInfo(AdapterInfo, &Size) != ERROR_SUCCESS)
    {
        delete[] AdapterInfo;
        return false;
    }

    PIP_ADAPTER_INFO CurrentAdapter = AdapterInfo;
    while (CurrentAdapter)
    {
        std::cout << dye::light_purple("Description: ") << dye::aqua(CurrentAdapter->Description) << std::endl;
        std::cout << dye::light_purple("Adapter: ") << dye::green(CurrentAdapter->AdapterName) << std::endl;

        QueryMac(CurrentAdapter->AdapterName);

        std::cout << std::endl;
        CurrentAdapter = CurrentAdapter->Next;
    }

    delete[] AdapterInfo;
    return true;
}

void MAC::QueryRouter()
{
    ULONG AdapterTableSize = 0x1000;
    PIP_ADAPTER_INFO Adapters = nullptr;
    ULONG Status = 0;

    do
    {
        if (Adapters != nullptr)
        {
            AdapterTableSize += 0x1000;
        }

        Adapters = (PIP_ADAPTER_INFO)realloc(Adapters, AdapterTableSize);
        Status = GetAdaptersInfo(Adapters, &AdapterTableSize);
    } while (Status == ERROR_BUFFER_OVERFLOW);

    PIP_ADAPTER_INFO CurrentAdapter = Adapters;

    while (CurrentAdapter)
    {
        if (strcmp(CurrentAdapter->GatewayList.IpAddress.String, "0.0.0.0") != 0)
        {
            ULONG ArpSize = 0x1000;
            PMIB_IPNETTABLE ArpTable = nullptr;

            do
            {
                ArpTable = (PMIB_IPNETTABLE)realloc(ArpTable, ArpSize);
                Status = GetIpNetTable(ArpTable, &ArpSize, TRUE);

                if (Status == ERROR_INSUFFICIENT_BUFFER)
                {
                    ArpSize += 0x1000;
                }
            } while (Status == ERROR_INSUFFICIENT_BUFFER);

            if (Status == NO_ERROR)
            {
                for (DWORD Index = 0; Index < ArpTable->dwNumEntries; Index++)
                {
                    auto& ArpEntry = ArpTable->table[Index];
                    auto LocalAddr = inet_addr(CurrentAdapter->GatewayList.IpAddress.String);
                    if (ArpEntry.dwAddr == LocalAddr)
                    {
                        std::cout << dye::light_purple("Router MAC: ")
                            << dye::green(FormatMac(ArpEntry.bPhysAddr, ArpEntry.dwPhysAddrLen)) << std::endl;
                    }
                }
            }

            if (ArpTable)
            {
                free(ArpTable);
            }
        }
        CurrentAdapter = CurrentAdapter->Next;
    }

    std::cout << std::endl;

    if (Adapters)
    {
        free(Adapters);
    }
}

void MAC::QueryAllInfo()
{
    UTILS::PrintFixedBox("MAC");

    QueryRouter();
    QueryAdaptersInfo();
}

std::vector<std::string> MAC::GetMacList()
{
    std::vector<std::string> MacAddresses;
    ULONG Size = 0;
    if (GetAdaptersInfo(nullptr, &Size) != ERROR_BUFFER_OVERFLOW)
    {
        return MacAddresses;
    }

    PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO) new BYTE[Size];
    if (GetAdaptersInfo(AdapterInfo, &Size) != ERROR_SUCCESS)
    {
        delete[] AdapterInfo;
        return MacAddresses;
    }

    PIP_ADAPTER_INFO CurrentAdapter = AdapterInfo;
    while (CurrentAdapter)
    {
        MacAddresses.push_back(FormatMac(CurrentAdapter->Address, CurrentAdapter->AddressLength));
        CurrentAdapter = CurrentAdapter->Next;
    }

    delete[] AdapterInfo;
    return MacAddresses;
}

std::string MAC::GetRouterMac()
{
    std::string RouterMacString;

    ULONG AdapterTableSize = 0x1000;
    PIP_ADAPTER_INFO Adapters = nullptr;
    ULONG Status = 0;

    do
    {
        if (Adapters != nullptr)
        {
            AdapterTableSize += 0x1000;
        }

        Adapters = (PIP_ADAPTER_INFO)realloc(Adapters, AdapterTableSize);
        Status = GetAdaptersInfo(Adapters, &AdapterTableSize);
    } while (Status == ERROR_BUFFER_OVERFLOW);

    PIP_ADAPTER_INFO CurrentAdapter = Adapters;

    while (CurrentAdapter)
    {
        if (strcmp(CurrentAdapter->GatewayList.IpAddress.String, "0.0.0.0") != 0)
        {
            ULONG ArpSize = 0x1000;
            PMIB_IPNETTABLE ArpTable = nullptr;

            do
            {
                ArpTable = (PMIB_IPNETTABLE)realloc(ArpTable, ArpSize);
                Status = GetIpNetTable(ArpTable, &ArpSize, TRUE);

                if (Status == ERROR_INSUFFICIENT_BUFFER)
                {
                    ArpSize += 0x1000;
                }
            } while (Status == ERROR_INSUFFICIENT_BUFFER);

            if (Status == NO_ERROR)
            {
                for (DWORD Index = 0; Index < ArpTable->dwNumEntries; Index++)
                {
                    auto& ArpEntry = ArpTable->table[Index];
                    auto LocalAddr = inet_addr(CurrentAdapter->GatewayList.IpAddress.String);

                    if (ArpEntry.dwAddr == LocalAddr)
                    {
                        RouterMacString = FormatMac(ArpEntry.bPhysAddr, ArpEntry.dwPhysAddrLen);
                        break;
                    }
                }
            }

            if (ArpTable)
            {
                free(ArpTable);
            }
        }
        CurrentAdapter = CurrentAdapter->Next;
    }

    if (Adapters)
    {
        free(Adapters);
    }

    return RouterMacString;
}