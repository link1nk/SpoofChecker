#include "pch.h"

bool SMBIOS::Initialize()
{
    m_RawData = GetRawData();

    return m_RawData != nullptr;
}

char* SMBIOS::GetString(const SMBIOSStruct* Header, UINT8 StringIndex)
{
    const char* StringAreaStart = reinterpret_cast<const char*>(Header) + Header->Length;

    if (StringIndex == 0 || *StringAreaStart == 0)
    {
        return const_cast<char*>("");
    }

    for (UINT8 Index = 1; Index < StringIndex; ++Index)
    {
        StringAreaStart += strlen(StringAreaStart) + 1;
        if (*StringAreaStart == 0)
        {
            return const_cast<char*>("");
        }
    }

    return const_cast<char*>(StringAreaStart);
}

RawSMBIOSData* SMBIOS::GetRawData()
{
    DWORD SmBiosDataSize = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (SmBiosDataSize == 0)
    {
        return nullptr;
    }

    RawSMBIOSData* SmBiosData = (RawSMBIOSData*)HeapAlloc(GetProcessHeap(), 0, SmBiosDataSize);
    if (!SmBiosData)
    {
        return nullptr;
    }

    DWORD BytesWritten = GetSystemFirmwareTable('RSMB', 0, SmBiosData, SmBiosDataSize);
    if (BytesWritten != SmBiosDataSize)
    {
        HeapFree(GetProcessHeap(), 0, SmBiosData);
        return nullptr;
    }

    return SmBiosData;
}

const SMBIOSStruct* SMBIOS::GetNextStruct(const SMBIOSStruct* CurrentStruct)
{
    const BYTE* Ptr = reinterpret_cast<const BYTE*>(CurrentStruct) + CurrentStruct->Length;

    while (Ptr[0] != 0 || Ptr[1] != 0)
    {
        Ptr++;
    }

    Ptr += 2;
    return reinterpret_cast<const SMBIOSStruct*>(Ptr);
}

void SMBIOS::QueryBiosInfo(const SMBIOSStructType0* Table)
{
    std::cout << "[" << dye::purple("Type 0") << " -> " << dye::aqua("BIOS Information") << "]\n";
    std::cout << dye::light_purple("  Vendor: ") << dye::green(GetString(Table, Table->Vendor)) << std::endl;
    std::cout << std::endl;
}

void SMBIOS::QuerySystemInfo(const SMBIOSStructType1* Table)
{
    std::cout << "[" << dye::purple("Type 1") << " -> " << dye::aqua("System Information") << "]\n";
    std::cout << dye::light_purple("  Manufacturer: ") << dye::green(GetString(Table, Table->Manufacturer)) << std::endl;
    std::cout << dye::light_purple("  Product Name: ") << dye::green(GetString(Table, Table->ProductName)) << std::endl;
    std::cout << dye::light_purple("  Serial Number: ") << dye::green(GetString(Table, Table->SerialNumber)) << std::endl;
    std::cout << std::endl;
}

void SMBIOS::QueryBaseboardInfo(const SMBIOSStructType2* Table)
{
    std::cout << "[" << dye::purple("Type 2") << " -> " << dye::aqua("Baseboard Information") << "]\n";
    std::cout << dye::light_purple("  Manufacturer: ") << dye::green(GetString(Table, Table->Manufacturer)) << std::endl;
    std::cout << dye::light_purple("  Product Name: ") << dye::green(GetString(Table, Table->ProductName)) << std::endl;
    std::cout << dye::light_purple("  Serial Number: ") << dye::green(GetString(Table, Table->SerialNumber)) << std::endl;
    std::cout << dye::light_purple("  Asset Tag: ") << dye::green(GetString(Table, Table->AssetTag)) << std::endl;
    std::cout << std::endl;
}

void SMBIOS::QueryEnclosureInfo(const SMBIOSStructType3* Table)
{
    std::cout << "[" << dye::purple("Type 3") << " -> " << dye::aqua("Enclosure Information") << "]\n";
    std::cout << dye::light_purple("  Manufacturer: ") << dye::green(GetString(Table, Table->Manufacturer)) << std::endl;
    std::cout << dye::light_purple("  Serial Number: ") << dye::green(GetString(Table, Table->SerialNumber)) << std::endl;
    std::cout << std::endl;
}

void SMBIOS::QueryProcessorInfo(const SMBIOSStructType4* Table)
{
    std::cout << "[" << dye::purple("Type 4") << " -> " << dye::aqua("Processor Information") << "]\n";
    std::cout << dye::light_purple("  Processor ID: ");
    for (int Index = 0; Index < sizeof(Table->ProcessorId); Index++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << dye::green((int)Table->ProcessorId[Index]);
    }
    std::cout << std::dec << std::endl;

    std::cout << dye::light_purple("  Serial Number: ") << dye::green(GetString(Table, Table->SerialNumber)) << std::endl;
    std::cout << dye::light_purple("  Asset Tag: ") << dye::green(GetString(Table, Table->AssetTag)) << std::endl;
    std::cout << std::endl;
}

void SMBIOS::QueryMemoryDeviceInfo(const SMBIOSStructType17* Table)
{
    std::cout << "[" << dye::purple("Type 17") << " -> " << dye::aqua("Memory Device") << "]\n";
    std::cout << dye::light_purple("  Bank Locator: ") << dye::green(GetString(Table, Table->BankLocator)) << std::endl;
    std::cout << dye::light_purple("  Device Locator: ") << dye::green(GetString(Table, Table->DeviceLocator)) << std::endl;
    std::cout << dye::light_purple("  Part Number: ") << dye::green(GetString(Table, Table->PartNumber)) << std::endl;
    std::cout << dye::light_purple("  Manufacturer: ") << dye::green(GetString(Table, Table->Manufacturer)) << std::endl;
    std::cout << dye::light_purple("  Serial Number: ") << dye::green(GetString(Table, Table->SerialNumber)) << std::endl;
    std::cout << dye::light_purple("  Asset Tag: ") << dye::green(GetString(Table, Table->AssetTagNumber)) << std::endl;
    std::cout << std::endl;
}

void SMBIOS::QueryAllInfo()
{
    UTILS::PrintFixedBox("SMBIOS");

    if (!Initialize())
    {
        return;
    }

    const BYTE* DataPtr = m_RawData->SMBIOSTableData;
    const BYTE* EndPtr = reinterpret_cast<const BYTE*>(m_RawData) + m_RawData->Length;

    while (DataPtr < EndPtr)
    {
        const SMBIOSStruct* Header = reinterpret_cast<const SMBIOSStruct*>(DataPtr);
        if (Header->Length < 4)
        {
            break;
        }

        switch (Header->Type)
        {
        case 0:
            QueryBiosInfo(reinterpret_cast<const SMBIOSStructType0*>(Header));
            break;
        case 1:
            QuerySystemInfo(reinterpret_cast<const SMBIOSStructType1*>(Header));
            break;
        case 2:
            QueryBaseboardInfo(reinterpret_cast<const SMBIOSStructType2*>(Header));
            break;
        case 3:
            QueryEnclosureInfo(reinterpret_cast<const SMBIOSStructType3*>(Header));
            break;
        case 4:
            QueryProcessorInfo(reinterpret_cast<const SMBIOSStructType4*>(Header));
            break;
        case 17:
            QueryMemoryDeviceInfo(reinterpret_cast<const SMBIOSStructType17*>(Header));
            break;
        }

        if (Header->Type == 127)
        {
            break;
        }

        DataPtr = reinterpret_cast<const BYTE*>(GetNextStruct(Header));
    }
}