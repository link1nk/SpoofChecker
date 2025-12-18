#include "pch.h"

std::string Registry::ReadRegistryBinary(HKEY KeyRoot, const std::wstring& SubKey, const std::wstring& ValueName)
{
    HKEY KeyHandle;
    std::string Result = "(not found)";
    if (RegOpenKeyExW(KeyRoot, SubKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &KeyHandle) == ERROR_SUCCESS)
    {
        DWORD Type = 0, DataSize = 0;
        if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, nullptr, &DataSize) == ERROR_SUCCESS)
        {
            if (Type == REG_BINARY && DataSize > 0)
            {
                std::vector<BYTE> Buffer(DataSize);
                if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, nullptr,
                    Buffer.data(), &DataSize) == ERROR_SUCCESS)
                {
                    std::ostringstream StringStream;
                    for (size_t Index = 0; Index < Buffer.size(); Index++)
                    {
                        StringStream << std::hex << std::setw(2) << std::setfill('0') << (int)Buffer[Index];
                        if (Index < Buffer.size() - 1) StringStream << " ";
                    }
                    Result = StringStream.str();
                }
            }
        }
        RegCloseKey(KeyHandle);
    }
    return Result.empty() ? "(not found)" : Result;
}

std::string Registry::ReadRegistryStringA(HKEY KeyRoot, const std::wstring& SubKey, const std::wstring& ValueName)
{
    HKEY KeyHandle;
    std::string Result = "(not found)";
    if (RegOpenKeyExW(KeyRoot, SubKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &KeyHandle) == ERROR_SUCCESS)
    {
        DWORD Type = 0, DataSize = 0;
        if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, nullptr, &DataSize) == ERROR_SUCCESS)
        {
            if ((Type == REG_SZ || Type == REG_EXPAND_SZ) && DataSize > 2)
            {
                std::wstring Buffer(DataSize / sizeof(wchar_t), L'\0');
                if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, nullptr,
                    reinterpret_cast<LPBYTE>(&Buffer[0]), &DataSize) == ERROR_SUCCESS)
                {
                    Buffer.resize((DataSize / sizeof(wchar_t)) - 1);
                    Result.assign(Buffer.begin(), Buffer.end());
                }
            }
        }
        RegCloseKey(KeyHandle);
    }
    return Result.empty() ? "(not found)" : Result;
}

DWORD Registry::ReadRegistryDword(HKEY KeyRoot, const std::wstring& SubKey, const std::wstring& ValueName)
{
    HKEY KeyHandle;
    if (RegOpenKeyExW(KeyRoot, SubKey.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
    {
        return 0;
    }

    DWORD Type = 0;
    DWORD Data = 0;
    DWORD DataSize = sizeof(Data);
    if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, (LPBYTE)&Data, &DataSize) != ERROR_SUCCESS || Type != REG_DWORD)
    {
        RegCloseKey(KeyHandle);
        return 0;
    }

    RegCloseKey(KeyHandle);
    return Data;
}

DWORD64 Registry::ReadRegistryQword(HKEY KeyRoot, const std::wstring& SubKey, const std::wstring& ValueName)
{
    HKEY KeyHandle;
    DWORD64 Value = 0;
    DWORD DataSize = sizeof(Value);
    LONG Result = RegOpenKeyExW(KeyRoot, SubKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &KeyHandle);
    if (Result == ERROR_SUCCESS)
    {
        if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&Value), &DataSize) != ERROR_SUCCESS)
        {
            Value = 0;
        }
        RegCloseKey(KeyHandle);
    }
    return Value;
}

std::vector<std::wstring> Registry::ReadRegistryMultiSzW(HKEY KeyRoot, const std::wstring& SubKey, const std::wstring& ValueName)
{
    std::vector<std::wstring> Result;
    HKEY KeyHandle;

    if (RegOpenKeyExW(KeyRoot, SubKey.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
    {
        return Result;
    }

    DWORD Type = 0;
    DWORD DataSize = 0;

    if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, nullptr, &DataSize) != ERROR_SUCCESS || Type != REG_MULTI_SZ)
    {
        RegCloseKey(KeyHandle);
        return Result;
    }

    std::vector<wchar_t> Buffer(DataSize / sizeof(wchar_t));

    if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(Buffer.data()), &DataSize) == ERROR_SUCCESS)
    {
        const wchar_t* Ptr = Buffer.data();
        while (*Ptr)
        {
            std::wstring Entry(Ptr);
            Result.push_back(Entry);
            Ptr += Entry.size() + 1;
        }
    }

    RegCloseKey(KeyHandle);
    return Result;
}

std::vector<std::string> Registry::ReadRegistryMultiSzA(HKEY KeyRoot, const std::wstring& SubKey, const std::wstring& ValueName)
{
    std::vector<std::string> Result;
    HKEY KeyHandle;

    if (RegOpenKeyExW(KeyRoot, SubKey.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
    {
        return Result;
    }

    DWORD Type = 0;
    DWORD DataSize = 0;

    if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, nullptr, &DataSize) == ERROR_SUCCESS && Type == REG_MULTI_SZ)
    {
        std::vector<wchar_t> Buffer(DataSize / sizeof(wchar_t));

        if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(Buffer.data()), &DataSize) == ERROR_SUCCESS)
        {
            const wchar_t* Ptr = Buffer.data();
            while (*Ptr)
            {
                std::wstring WStr(Ptr);
                Result.push_back(UTILS::WStringToString(WStr));
                Ptr += WStr.size() + 1;
            }
        }
    }

    RegCloseKey(KeyHandle);
    return Result;
}

std::string Registry::UnixTimestampToDate(DWORD Timestamp)
{
    time_t RawTime = Timestamp;
    struct tm TimeInfo;
    char Buffer[64];

    localtime_s(&TimeInfo, &RawTime);
    strftime(Buffer, sizeof(Buffer), "%Y-%m-%d %H:%M:%S", &TimeInfo);

    return std::string(Buffer);
}

void Registry::EnumerateScsiDevices(const std::wstring& BaseKey)
{
    HKEY KeyHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &KeyHandle) != ERROR_SUCCESS)
    {
        return;
    }

    DWORD Index = 0;
    WCHAR SubKeyName[256];
    DWORD SubKeyLength;

    while (true)
    {
        SubKeyLength = 256;
        FILETIME FileTime;
        if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, &FileTime) != ERROR_SUCCESS)
        {
            break;
        }

        std::wstring NewPath = BaseKey + L"\\" + SubKeyName;

        if (wcsncmp(SubKeyName, L"Logical Unit Id", 15) == 0)
        {
            std::string Identifier = ReadRegistryStringA(HKEY_LOCAL_MACHINE, NewPath, L"Identifier");
            std::string Serial = ReadRegistryStringA(HKEY_LOCAL_MACHINE, NewPath, L"SerialNumber");

            if (!Identifier.empty() || !Serial.empty())
            {
                std::string NewPathAnsi = UTILS::WStringToString(NewPath);

                std::cout << "[" << dye::purple("Registry Key") << "] -> "
                    << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + NewPathAnsi) << std::endl;

                std::cout << dye::light_purple("  Identifier: ") << dye::green(Identifier) << std::endl;
                std::cout << dye::light_purple("  SerialNumber: ") << dye::green(Serial) << std::endl;
            }
        }

        EnumerateScsiDevices(NewPath);
    }

    RegCloseKey(KeyHandle);
}

void Registry::PrintHexString(const std::string& Name, const std::string& HexString, size_t PerLine)
{
    if (HexString.empty() || HexString == "(not found)")
    {
        std::cout << dye::light_purple(Name) << ": " << dye::green("(not found)") << std::endl;
        return;
    }

    std::cout << dye::light_purple(Name) << ":" << std::endl;

    std::istringstream StringStream(HexString);
    std::string Byte;
    size_t Count = 0;

    std::cout << "    ";

    while (StringStream >> Byte)
    {
        std::cout << dye::green(Byte) << " ";
        Count++;

        if (Count % PerLine == 0)
        {
            std::cout << std::endl << "    ";
        }
    }

    if (Count % PerLine != 0)
    {
        std::cout << std::endl;
    }
}

void Registry::EnumPnPTwoLevel(const std::wstring& BaseKey, const wchar_t* Title)
{
    HKEY BaseHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &BaseHandle) != ERROR_SUCCESS)
    {
        return;
    }

    std::string TitleAnsi = UTILS::WStringToString(Title);
    std::cout << dye::aqua("=== ") << dye::aqua(TitleAnsi) << dye::aqua(" ===") << std::endl;

    DWORD DeviceIndex = 0;
    WCHAR DeviceName[256];
    DWORD DeviceNameLength;

    while (true)
    {
        DeviceNameLength = 256;
        if (RegEnumKeyExW(BaseHandle, DeviceIndex++, DeviceName, &DeviceNameLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
        {
            break;
        }

        std::wstring DevicePath = BaseKey + L"\\" + DeviceName;
        HKEY DeviceHandle;

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, DevicePath.c_str(), 0, KEY_READ, &DeviceHandle) != ERROR_SUCCESS)
        {
            continue;
        }

        DWORD InstanceIndex = 0;
        WCHAR InstanceName[256];
        DWORD InstanceNameLength;

        while (true)
        {
            InstanceNameLength = 256;
            if (RegEnumKeyExW(DeviceHandle, InstanceIndex++, InstanceName, &InstanceNameLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            {
                break;
            }

            std::wstring InstancePath = DevicePath + L"\\" + InstanceName;

            std::string FriendlyName = ReadRegistryStringA(HKEY_LOCAL_MACHINE, InstancePath, L"FriendlyName");
            std::string ParentIdPrefix = ReadRegistryStringA(HKEY_LOCAL_MACHINE, InstancePath, L"ParentIdPrefix");

            std::vector<std::wstring> HardwareIdsList;
            DWORD Type = 0;
            DWORD Size = 0;

            if (RegGetValueW(HKEY_LOCAL_MACHINE, InstancePath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, &Type, nullptr, &Size) == ERROR_SUCCESS)
            {
                std::vector<wchar_t> Buffer(Size / sizeof(wchar_t));
                if (RegGetValueW(HKEY_LOCAL_MACHINE, InstancePath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, nullptr, Buffer.data(), &Size) == ERROR_SUCCESS)
                {
                    const wchar_t* Ptr = Buffer.data();
                    while (*Ptr)
                    {
                        HardwareIdsList.push_back(Ptr);
                        Ptr += wcslen(Ptr) + 1;
                    }
                }
            }

            bool AnyDataFound = (FriendlyName != "(not found)") || (!HardwareIdsList.empty()) || (ParentIdPrefix != "(not found)");

            if (AnyDataFound)
            {
                std::cout << "[" << dye::purple("Registry Key") << "] -> "
                    << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(InstancePath)) << std::endl;

                if (FriendlyName != "(not found)")
                {
                    std::cout << dye::light_purple("  FriendlyName: ") << dye::green(FriendlyName) << std::endl;
                }

                if (!HardwareIdsList.empty())
                {
                    for (const auto& HwId : HardwareIdsList)
                    {
                        std::cout << dye::light_purple("  HardwareID: ")
                            << dye::green(UTILS::WStringToString(HwId)) << std::endl;
                    }
                }

                if (ParentIdPrefix != "(not found)")
                {
                    std::cout << dye::light_purple("  ParentIdPrefix: ") << dye::green(ParentIdPrefix) << std::endl;
                }

                std::cout << std::endl;
            }
        }
        RegCloseKey(DeviceHandle);
    }
    RegCloseKey(BaseHandle);
}

void Registry::PrintMaybeName(HKEY RootKey, const std::wstring& KeyPath)
{
    std::string NameString = ReadRegistryStringA(RootKey, KeyPath, L"Name");
    if (NameString != "(not found)")
    {
        std::cout << dye::light_purple("  Name: ") << dye::green(NameString) << std::endl;
    }
    else
    {
        std::string NameBinary = ReadRegistryBinary(RootKey, KeyPath, L"Name");
        if (NameBinary != "(not found)")
        {
            std::cout << dye::light_purple("  Name (BIN):") << std::endl;
            std::istringstream StringStream(NameBinary);
            std::string ByteString;
            int Count = 0;
            std::cout << "    ";
            while (StringStream >> ByteString)
            {
                std::cout << dye::green(ByteString) << " ";
                if (++Count % 32 == 0)
                {
                    std::cout << std::endl << "    ";
                }
            }
            std::cout << std::endl;
        }
    }
}

std::string Registry::BytesToHex(const BYTE* Data, DWORD Size, size_t MaxLength)
{
    std::ostringstream StringStream;
    size_t Limit = Size;

    if (MaxLength > 0 && MaxLength < Size)
    {
        Limit = MaxLength;
    }

    for (size_t Index = 0; Index < Limit; Index++)
    {
        StringStream << std::hex << std::setw(2) << std::setfill('0') << (int)Data[Index] << " ";
        if ((Index + 1) % 16 == 0)
        {
            StringStream << "\n";
        }
    }
    return StringStream.str();
}

RawSMBIOSData* Registry::GetSMBIOS()
{
    HKEY KeyHandle = nullptr;
    LONG Status = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data",
        0,
        KEY_READ,
        &KeyHandle);

    if (Status != ERROR_SUCCESS)
    {
        return nullptr;
    }

    DWORD DataSize = 0;
    Status = RegQueryValueExW(
        KeyHandle,
        L"SMBiosData",
        nullptr,
        nullptr,
        nullptr,
        &DataSize);

    if (Status != ERROR_SUCCESS || DataSize == 0)
    {
        RegCloseKey(KeyHandle);
        return nullptr;
    }

    RawSMBIOSData* SmBiosData = (RawSMBIOSData*)HeapAlloc(GetProcessHeap(), 0, DataSize);
    if (!SmBiosData)
    {
        RegCloseKey(KeyHandle);
        return nullptr;
    }

    Status = RegQueryValueExW(
        KeyHandle,
        L"SMBiosData",
        nullptr,
        nullptr,
        reinterpret_cast<LPBYTE>(SmBiosData),
        &DataSize);

    RegCloseKey(KeyHandle);

    if (Status != ERROR_SUCCESS)
    {
        HeapFree(GetProcessHeap(), 0, SmBiosData);
        return nullptr;
    }

    return SmBiosData;
}

void Registry::EnumerateMonitors()
{
    std::cout << dye::aqua("=== Monitors ===") << std::endl;
    HKEY RootKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY", 0, KEY_READ, &RootKey) != ERROR_SUCCESS)
    {
        std::cout << "(not found)" << std::endl;
        return;
    }

    DWORD Index = 0;
    WCHAR SubKeyName[256];
    DWORD SubKeyLength;
    bool IsFound = false;

    while (true)
    {
        SubKeyLength = 256;
        if (RegEnumKeyExW(RootKey, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
        {
            break;
        }

        std::wstring DisplayPath = L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\" + std::wstring(SubKeyName);

        HKEY DisplayKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, DisplayPath.c_str(), 0, KEY_READ, &DisplayKey) == ERROR_SUCCESS)
        {
            DWORD SubIndex = 0;
            WCHAR DeviceName[256];
            DWORD DeviceNameLength;

            while (true)
            {
                DeviceNameLength = 256;
                if (RegEnumKeyExW(DisplayKey, SubIndex++, DeviceName, &DeviceNameLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                {
                    break;
                }

                std::wstring MonitorPath = DisplayPath + L"\\" + DeviceName;
                std::wstring DeviceParametersPath = MonitorPath + L"\\Device Parameters";

                std::string EdidData = ReadRegistryBinary(HKEY_LOCAL_MACHINE, DeviceParametersPath, L"EDID");

                std::vector<std::wstring> HardwareIds;
                DWORD Type = 0;
                DWORD Size = 0;

                if (RegGetValueW(HKEY_LOCAL_MACHINE, MonitorPath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, &Type, nullptr, &Size) == ERROR_SUCCESS && Size > 2)
                {
                    std::wstring Buffer(Size / sizeof(wchar_t), L'\0');
                    if (RegGetValueW(HKEY_LOCAL_MACHINE, MonitorPath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, nullptr, &Buffer[0], &Size) == ERROR_SUCCESS)
                    {
                        const wchar_t* Ptr = Buffer.c_str();
                        while (*Ptr)
                        {
                            HardwareIds.push_back(Ptr);
                            Ptr += wcslen(Ptr) + 1;
                        }
                    }
                }

                if (EdidData != "(not found)" || !HardwareIds.empty())
                {
                    IsFound = true;

                    std::string SubKeyNameAnsi = UTILS::WStringToString(SubKeyName);
                    std::string DeviceNameAnsi = UTILS::WStringToString(DeviceName);

                    std::cout << "[" << dye::purple("Registry Key") << "] -> "
                        << dye::light_yellow(UTILS::WStringToString(MonitorPath)) << std::endl;

                    std::cout << dye::light_purple("Monitor: ")
                        << dye::green(SubKeyNameAnsi) << "\\" << dye::green(DeviceNameAnsi) << std::endl;

                    if (!HardwareIds.empty())
                    {
                        std::cout << dye::light_purple("  HardwareID: ");
                        for (const auto& Id : HardwareIds)
                        {
                            std::cout << dye::yellow(UTILS::WStringToString(Id)) << std::endl;
                        }
                    }

                    if (EdidData != "(not found)")
                    {
                        std::cout << dye::light_purple("  EDID:") << std::endl;

                        std::istringstream StringStream(EdidData);
                        std::string ByteString;
                        std::vector<std::string> Bytes;
                        while (StringStream >> ByteString)
                        {
                            Bytes.push_back(ByteString);
                        }

                        // Correção: Se o buffer for 256 bytes mas a segunda metade for vazia, corta.
                        if (Bytes.size() == 256)
                        {
                            bool IsPadding = true;
                            for (size_t K = 128; K < 256; K++)
                            {
                                if (Bytes[K] != "00")
                                {
                                    IsPadding = false;
                                    break;
                                }
                            }
                            if (IsPadding)
                            {
                                Bytes.resize(128);
                            }
                        }

                        int Count = 0;
                        for (const auto& ByteVal : Bytes)
                        {
                            if (Count % 32 == 0)
                            {
                                std::cout << "    ";
                            }
                            std::cout << dye::green(ByteVal) << " ";
                            Count++;
                            if (Count % 32 == 0)
                            {
                                std::cout << std::endl;
                            }
                        }
                        if (Count % 32 != 0)
                        {
                            std::cout << std::endl;
                        }
                    }
                }
            }
            RegCloseKey(DisplayKey);
        }
    }

    if (!IsFound)
    {
        std::cout << "(not found)" << std::endl;
    }
    std::cout << std::endl;
    RegCloseKey(RootKey);
}

void Registry::EnumerateGraphicsDrivers()
{
    std::cout << dye::aqua("=== GraphicsDrivers Configuration & Connectivity ===") << std::endl;

    std::vector<std::wstring> BaseKeys = {
        L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\\Configuration",
        L"SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\\Connectivity"
    };

    for (const auto& BaseKey : BaseKeys)
    {
        HKEY KeyHandle;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
        {
            std::cout << dye::red("(not found): ") << UTILS::WStringToString(BaseKey) << std::endl;
            continue;
        }

        std::cout << dye::purple("[Registry Path] -> ") << dye::light_yellow(UTILS::WStringToString(BaseKey)) << std::endl;

        DWORD Index = 0;
        WCHAR SubKeyName[256];
        DWORD SubKeyLength;

        while (true)
        {
            SubKeyLength = 256;
            if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            {
                break;
            }

            std::wstring SubKeyFullPath = BaseKey + L"\\" + SubKeyName;
            std::cout << "  " << dye::light_purple("SubKey: ") << dye::green(UTILS::WStringToString(SubKeyName)) << std::endl;

            if (BaseKey.find(L"Configuration") != std::wstring::npos)
            {
                ULONGLONG Timestamp = 0;
                DWORD Type = 0;
                DWORD Size = sizeof(Timestamp);
                if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyFullPath.c_str(), L"Timestamp", RRF_RT_REG_QWORD, &Type, &Timestamp, &Size) == ERROR_SUCCESS)
                {
                    std::cout << "    " << dye::light_purple("Timestamp: ") << std::dec
                        << dye::green(Timestamp) << " -> " << dye::yellow(UnixTimestampToDate(Timestamp)) << std::endl;
                }
                else
                {
                    std::cout << "    " << dye::red("Timestamp: (not found)") << std::endl;
                }
            }
        }

        RegCloseKey(KeyHandle);
        std::wcout << std::endl;
    }
}

void Registry::PrintMotherboardUUIDs()
{
    std::cout << dye::aqua("=== Motherboard UUIDs ===") << std::endl;

    {
        std::wstring BaseKeyPath = L"SYSTEM\\HardwareConfig";
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow(UTILS::WStringToString(BaseKeyPath)) << std::endl;

        std::cout << dye::light_purple("  LastConfig: ")
            << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, BaseKeyPath, L"LastConfig"))
            << std::endl;

        HKEY KeyHandle;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKeyPath.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
        {
            DWORD Index = 0;
            WCHAR SubKeyName[256];
            DWORD SubKeyLength;

            while (true)
            {
                SubKeyLength = 256;
                if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                {
                    break;
                }

                std::wstring SubKeyPath = BaseKeyPath + L"\\" + SubKeyName;

                std::cout << "\n  [" << dye::purple("Config UUID") << "] " << dye::green(UTILS::WStringToString(SubKeyName)) << std::endl;

                std::cout << "    " << dye::light_purple("SystemManufacturer: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"SystemManufacturer")) << std::endl;

                std::cout << "    " << dye::light_purple("SystemProductName: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"SystemProductName")) << std::endl;

                std::cout << "    " << dye::light_purple("SystemFamily: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"SystemFamily")) << std::endl;

                std::cout << "    " << dye::light_purple("SystemVersion: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"SystemVersion")) << std::endl;

                std::cout << "    " << dye::light_purple("SystemSKU: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"SystemSKU")) << std::endl;

                std::vector<std::string> BiosVersions;
                DWORD Type = 0;
                DWORD Size = 0;
                if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"SystemBiosVersion", RRF_RT_REG_MULTI_SZ, &Type, nullptr, &Size) == ERROR_SUCCESS)
                {
                    std::vector<wchar_t> Buffer(Size / sizeof(wchar_t));
                    if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"SystemBiosVersion", RRF_RT_REG_MULTI_SZ, nullptr, Buffer.data(), &Size) == ERROR_SUCCESS)
                    {
                        const wchar_t* Ptr = Buffer.data();
                        while (*Ptr)
                        {
                            BiosVersions.push_back(UTILS::WStringToString(Ptr));
                            Ptr += wcslen(Ptr) + 1;
                        }
                    }
                }

                if (!BiosVersions.empty())
                {
                    std::cout << "    " << dye::light_purple("SystemBiosVersion: ");
                    for (size_t I = 0; I < BiosVersions.size(); ++I)
                    {
                        if (I > 0)
                        {
                            std::cout << " | ";
                        }
                        std::cout << dye::green(BiosVersions[I]);
                    }
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "    " << dye::light_purple("SystemBiosVersion: ") << dye::green("(not found)") << std::endl;
                }

                std::cout << "    " << dye::light_purple("BaseBoardManufacturer: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"BaseBoardManufacturer")) << std::endl;

                std::cout << "    " << dye::light_purple("BaseBoardProduct: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"BaseBoardProduct")) << std::endl;

                std::cout << "    " << dye::light_purple("BIOSVendor: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"BIOSVendor")) << std::endl;

                std::cout << "    " << dye::light_purple("BIOSVersion: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"BIOSVersion")) << std::endl;

                std::cout << "    " << dye::light_purple("BIOSReleasedDate: ")
                    << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"BIOSReleasedDate")) << std::endl;

                DWORD LastUseType = 0;
                DWORD LastUseSize = 0;
                if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"LastUse", RRF_RT_REG_BINARY, &LastUseType, nullptr, &LastUseSize) == ERROR_SUCCESS)
                {
                    std::vector<BYTE> LastUseBuffer(LastUseSize);
                    if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"LastUse", RRF_RT_REG_BINARY, nullptr, LastUseBuffer.data(), &LastUseSize) == ERROR_SUCCESS)
                    {
                        std::cout << "    " << dye::light_purple("LastUse: ");
                        for (size_t I = 0; I < LastUseBuffer.size(); ++I)
                        {
                            std::cout << std::hex << std::setw(2) << std::setfill('0') << dye::green((int)LastUseBuffer[I]) << " ";
                        }
                        std::cout << std::dec << std::endl;
                    }
                }
            }
            RegCloseKey(KeyHandle);
        }
    }

    std::cout << std::endl;
}

void Registry::PrintNvidiaUUIDs()
{
    std::cout << dye::aqua("=== NVIDIA UUIDs ===") << std::endl;

    {
        std::wstring KeyPath = L"SOFTWARE\\NVIDIA Corporation\\Global";
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

        std::cout << dye::light_purple("  ClientUUID: ")
            << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"ClientUUID"))
            << std::endl;

        std::cout << dye::light_purple("  PersistenceIdentifier: ")
            << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"PersistenceIdentifier"))
            << std::endl;
    }

    std::cout << std::endl;

    {
        std::wstring KeyPath = L"SOFTWARE\\NVIDIA Corporation\\Global\\CoProcManager";
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

        std::cout << dye::light_purple("  ChipsetMatchID: ")
            << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"ChipsetMatchID"))
            << std::endl;
    }

    std::cout << std::endl;
}

void Registry::PrintVolumes()
{
    std::cout << dye::aqua("=== Volume GUIDs (MountedDevices + Explorer + Dfrg) ===") << std::endl;

    struct KeyInfo
    {
        HKEY Root;
        std::wstring Path;
    };

    std::vector<KeyInfo> Keys = {
        { HKEY_LOCAL_MACHINE, L"SYSTEM\\MountedDevices" },
        { HKEY_CURRENT_USER,  L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket\\Volume" },
        { HKEY_CURRENT_USER,  L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket" },
        { HKEY_CURRENT_USER,  L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MountPoints2\\CPC\\Volume" },
        { HKEY_CURRENT_USER,  L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MountPoints2" },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Dfrg\\Statistics" }
    };

    for (const auto& CurrentKey : Keys)
    {
        HKEY KeyHandle;
        if (RegOpenKeyExW(CurrentKey.Root, CurrentKey.Path.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
        {
            std::cout << "[Registry Key] -> "
                << dye::light_yellow(UTILS::WStringToString((CurrentKey.Root == HKEY_LOCAL_MACHINE ? L"HKEY_LOCAL_MACHINE\\" : L"HKEY_CURRENT_USER\\") + CurrentKey.Path))
                << " " << dye::red("(not found)") << std::endl;
            continue;
        }

        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow(UTILS::WStringToString((CurrentKey.Root == HKEY_LOCAL_MACHINE ? L"HKEY_LOCAL_MACHINE\\" : L"HKEY_CURRENT_USER\\") + CurrentKey.Path))
            << std::endl;

        bool Found = false;

        if (CurrentKey.Path == L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket")
        {
            DWORD Type;
            BYTE Data[4096];
            DWORD DataSize = sizeof(Data);

            if (RegQueryValueExW(KeyHandle, L"LastEnum", nullptr, &Type, Data, &DataSize) == ERROR_SUCCESS)
            {
                Found = true;

                if (Type == REG_MULTI_SZ)
                {
                    WCHAR* Ptr = (WCHAR*)Data;
                    while (*Ptr)
                    {
                        std::wstring WideString(Ptr);
                        std::string StringAnsi = UTILS::WStringToString(WideString);

                        std::cout << "  " << dye::light_purple("LastEnum: ")
                            << dye::green(StringAnsi) << std::endl;

                        Ptr += wcslen(Ptr) + 1;
                    }
                }
                else if (Type == REG_SZ)
                {
                    std::wstring WideString((WCHAR*)Data);
                    std::cout << "  " << dye::light_purple("LastEnum: ")
                        << dye::green(UTILS::WStringToString(WideString)) << std::endl;
                }
            }
        }
        else if (CurrentKey.Path == L"SYSTEM\\MountedDevices")
        {
            DWORD Index = 0;
            WCHAR ValueName[512];
            DWORD ValueNameLen;
            BYTE Data[2048];
            DWORD DataSize;
            DWORD Type;

            while (true)
            {
                ValueNameLen = 512;
                DataSize = sizeof(Data);
                if (RegEnumValueW(KeyHandle, Index++, ValueName, &ValueNameLen, nullptr, &Type, Data, &DataSize) != ERROR_SUCCESS)
                {
                    break;
                }

                Found = true;
                std::string ValueNameAnsi = UTILS::WStringToString(ValueName);

                std::cout << "  " << dye::light_purple("Name: ") << dye::green(ValueNameAnsi) << std::endl;

                if (Type == REG_BINARY)
                {
                    std::cout << "    " << dye::light_purple("Data (hex):") << std::endl;
                    for (DWORD J = 0; J < DataSize; J++)
                    {
                        if (J % 32 == 0)
                        {
                            std::cout << "      ";
                        }
                        std::cout << std::hex << std::uppercase << std::setw(2)
                            << std::setfill('0') << dye::green((int)Data[J]) << " ";
                        if ((J + 1) % 32 == 0)
                        {
                            std::cout << std::endl;
                        }
                    }
                    if (DataSize % 32 != 0)
                    {
                        std::cout << std::endl;
                    }
                    std::cout << std::dec;
                }
                else
                {
                    std::cout << "    " << dye::yellow("(unsupported type)") << std::endl;
                }
            }
        }
        else
        {
            DWORD Index = 0;
            WCHAR SubKeyName[256];
            DWORD SubKeyLength;

            while (true)
            {
                SubKeyLength = 256;
                if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                {
                    break;
                }

                Found = true;
                std::string GuidAnsi = UTILS::WStringToString(SubKeyName);
                std::cout << "  " << dye::light_purple("SubKey: ")
                    << dye::green(GuidAnsi) << std::endl;
            }
        }

        if (!Found)
        {
            std::cout << "  " << dye::green("(no values or subkeys)") << std::endl;
        }

        RegCloseKey(KeyHandle);
        std::cout << std::endl;
    }
}

void Registry::PrintExtraHardware()
{
    std::cout << dye::aqua("=== Extra Hardware IDs ===") << std::endl;

    {
        std::wstring KeyPath = L"SYSTEM\\CurrentControlSet\\Services\\TPM\\WMI";
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

        std::cout << dye::light_purple("  WindowsAIKHash: ")
            << dye::green(ReadRegistryBinary(HKEY_LOCAL_MACHINE, KeyPath, L"WindowsAIKHash"))
            << std::endl << std::endl;
    }

    {
        std::wstring KeyPath = L"SYSTEM\\CurrentControlSet\\Services\\TPM\\ODUID";
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

        std::cout << dye::light_purple("  RandomSeed: ")
            << dye::green(ReadRegistryBinary(HKEY_LOCAL_MACHINE, KeyPath, L"RandomSeed"))
            << std::endl << std::endl;
    }

    {
        std::wstring KeyPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows\\Win32kWPP\\Parameters";
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

        std::cout << dye::light_purple("  WppRecorder_TraceGuid: ")
            << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"WppRecorder_TraceGuid"))
            << std::endl << std::endl;
    }

    std::wstring AdapterBasePath = L"HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter\\0";
    HKEY AdapterKey;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, AdapterBasePath.c_str(), 0, KEY_READ, &AdapterKey) == ERROR_SUCCESS)
    {
        DWORD ControllerIndex = 0;
        WCHAR ControllerName[256];
        DWORD ControllerNameLen;

        while (true)
        {
            ControllerNameLen = 256;
            if (RegEnumKeyExW(AdapterKey, ControllerIndex++, ControllerName, &ControllerNameLen, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            {
                break;
            }

            std::wstring ControllerPath = AdapterBasePath + L"\\" + ControllerName + L"\\0";
            HKEY ControllerKey;

            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, ControllerPath.c_str(), 0, KEY_READ, &ControllerKey) == ERROR_SUCCESS)
            {
                DWORD PeripheralIndex = 0;
                WCHAR PeripheralName[256];
                DWORD PeripheralNameLen;

                while (true)
                {
                    PeripheralNameLen = 256;
                    if (RegEnumKeyExW(ControllerKey, PeripheralIndex++, PeripheralName, &PeripheralNameLen, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                    {
                        break;
                    }

                    for (int Index = 0; Index <= 9; Index++)
                    {
                        std::wstring FinalKeyPath = ControllerPath + L"\\" + PeripheralName + L"\\" + std::to_wstring(Index);
                        HKEY FinalKeyHandle;

                        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, FinalKeyPath.c_str(), 0, KEY_READ, &FinalKeyHandle) == ERROR_SUCCESS)
                        {
                            std::cout << "[" << dye::purple("Registry Key") << "] -> "
                                << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(FinalKeyPath)) << std::endl;

                            std::cout << dye::light_purple("  Identifier: ")
                                << dye::green(ReadRegistryStringA(HKEY_LOCAL_MACHINE, FinalKeyPath, L"Identifier"))
                                << std::endl << std::endl;

                            RegCloseKey(FinalKeyHandle);
                        }
                    }
                }
                RegCloseKey(ControllerKey);
            }
        }
        RegCloseKey(AdapterKey);
    }
}

void Registry::PrintUEFI_ESRT()
{
    std::cout << dye::aqua("=== UEFI ESRT ===") << std::endl;

    const std::wstring EsrtPath = L"HARDWARE\\UEFI\\ESRT";
    HKEY KeyHandle;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, EsrtPath.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
    {
        std::cout << "[" << dye::purple("Registry Key") << " -> " << dye::light_yellow("HKEY_LOCAL_MACHINE\\")
            << dye::light_yellow(UTILS::WStringToString(EsrtPath)) << std::endl;

        DWORD Index = 0;
        WCHAR SubKeyName[256];
        DWORD SubKeyLength = 256;

        while (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            std::wstring SubKey(SubKeyName);
            std::string SubKeyAnsi = UTILS::WStringToString(SubKey);

            std::cout << dye::light_purple("  Subkey: ") << dye::green(SubKeyAnsi) << std::endl;

            SubKeyLength = 256;
        }

        RegCloseKey(KeyHandle);
    }

    std::cout << std::endl;
}

void Registry::PrintVideoDeviceMap()
{
    std::cout << dye::aqua("=== DeviceMap ===") << std::endl;

    HKEY KeyHandle;
    const std::wstring DeviceMapPath = L"HARDWARE\\DEVICEMAP\\VIDEO";

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, DeviceMapPath.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
    {
        std::cout << "[" << dye::purple("Registry Key") << " -> " << dye::light_yellow("HKEY_LOCAL_MACHINE\\")
            << dye::light_yellow(UTILS::WStringToString(DeviceMapPath)) << std::endl;

        DWORD Index = 0;
        WCHAR ValueName[256];
        BYTE DataBuffer[512];
        DWORD ValueLength;
        DWORD DataSize;
        DWORD Type;

        std::vector<std::wstring> Guids;

        while (true)
        {
            ValueLength = 256;
            DataSize = sizeof(DataBuffer);
            LONG Result = RegEnumValueW(KeyHandle, Index++, ValueName, &ValueLength, NULL, &Type, DataBuffer, &DataSize);
            if (Result != ERROR_SUCCESS)
            {
                break;
            }

            if (Type == REG_SZ)
            {
                std::wstring WideString((WCHAR*)DataBuffer);
                std::string NameAnsi = UTILS::WStringToString(ValueName);
                std::string DataAnsi = UTILS::WStringToString(WideString);

                std::cout << "  " << dye::light_purple(NameAnsi)
                    << ": " << dye::green(DataAnsi) << std::endl;

                size_t Position = WideString.find(L"{");
                if (Position != std::wstring::npos)
                {
                    Guids.push_back(WideString.substr(Position, WideString.size() - Position));
                }
            }
        }
        RegCloseKey(KeyHandle);
    }

    std::cout << std::endl;
}

void Registry::PrintVideoRegistry()
{
    std::cout << dye::aqua("=== Video ===") << std::endl;

    const std::wstring VideoPath = L"SYSTEM\\CurrentControlSet\\Control\\Video";
    HKEY KeyHandle;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, VideoPath.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
    {
        std::cout << "[" << dye::purple("Registry Key") << " -> " << dye::light_yellow("HKEY_LOCAL_MACHINE\\")
            << dye::light_yellow(UTILS::WStringToString(VideoPath)) << std::endl;

        DWORD Index = 0;
        WCHAR SubKeyName[256];
        DWORD SubKeyLength = 256;

        while (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            std::wstring SubKey(SubKeyName);
            std::string SubKeyAnsi = UTILS::WStringToString(SubKey);

            std::cout << dye::light_purple("  Subkey: ") << dye::green(SubKeyAnsi) << std::endl;

            SubKeyLength = 256;
        }

        RegCloseKey(KeyHandle);
    }

    std::cout << std::endl;
}

void Registry::PrintNTCurrentVersion()
{
    std::wstring SubKeyNT = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

    DWORD InstallDate = ReadRegistryDword(HKEY_LOCAL_MACHINE, SubKeyNT, L"InstallDate");
    std::string ProductId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyNT, L"ProductId");
    std::string RegisteredOwner = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyNT, L"RegisteredOwner");

    // Novos campos adicionados
    std::string BuildLab = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyNT, L"BuildLab");
    std::string BuildLabEx = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyNT, L"BuildLabEx");

    std::cout << dye::aqua("=== NT CurrentVersion ===") << std::endl;

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(SubKeyNT))
        << std::endl;

    std::cout << dye::light_purple("  InstallDate: ")
        << dye::green(InstallDate) << std::endl;

    ULONGLONG InstallTime = 0;
    DWORD Type = 0;
    DWORD Size = sizeof(InstallTime);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyNT.c_str(), L"InstallTime", RRF_RT_REG_QWORD, &Type, &InstallTime, &Size) == ERROR_SUCCESS)
    {
        std::cout << dye::light_purple("  InstallTime: ")
            << dye::green(InstallTime) << std::endl;
    }
    else
    {
        std::cout << dye::light_purple("  InstallTime: ")
            << dye::red("(not found)") << std::endl;
    }

    std::cout << dye::light_purple("  ProductId: ")
        << dye::green(ProductId) << std::endl;

    std::cout << dye::light_purple("  DigitalProductId: ")
        << dye::green(ReadRegistryBinary(HKEY_LOCAL_MACHINE, SubKeyNT, L"DigitalProductId"))
        << std::endl;

    std::cout << dye::light_purple("  DigitalProductId4: ")
        << dye::green(ReadRegistryBinary(HKEY_LOCAL_MACHINE, SubKeyNT, L"DigitalProductId4"))
        << std::endl;

    std::cout << dye::light_purple("  BuildLab: ")
        << dye::green(BuildLab) << std::endl;

    std::cout << dye::light_purple("  BuildLabEx: ")
        << dye::green(BuildLabEx) << std::endl;

    std::cout << dye::light_purple("  RegisteredOwner: ")
        << dye::green(RegisteredOwner) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintWindowsUpdate()
{
    std::wstring SubKeyWindowsUpdate = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate";

    std::string SusClientId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyWindowsUpdate, L"SusClientId");

    std::cout << dye::aqua("=== Windows Update ===") << std::endl;

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(SubKeyWindowsUpdate))
        << std::endl;

    std::cout << dye::light_purple("  SusClientId: ")
        << dye::green(SusClientId) << std::endl;

    std::cout << dye::light_purple("  SusClientIdValidation: ")
        << dye::green(ReadRegistryBinary(HKEY_LOCAL_MACHINE, SubKeyWindowsUpdate, L"SusClientIdValidation"))
        << std::endl;

    std::cout << std::endl;
}

void Registry::PrintWAT()
{
    std::wstring SubKeyPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows Activation Technologies\\AdminObject\\Store";

    std::string MachineId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"MachineId");
    std::string Value64 = ReadRegistryBinary(HKEY_LOCAL_MACHINE, SubKeyPath, L"64-bit");
    std::string Value32 = ReadRegistryBinary(HKEY_LOCAL_MACHINE, SubKeyPath, L"32-bit");

    std::cout << dye::aqua("=== Windows Activation Technologies ===") << std::endl;

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(SubKeyPath))
        << std::endl;

    std::cout << dye::light_purple("  MachineId: ")
        << dye::green(MachineId) << std::endl;

    std::cout << dye::light_purple("  64-bit (REG_BINARY): ")
        << dye::green(Value64) << std::endl;

    std::cout << dye::light_purple("  32-bit (REG_BINARY): ")
        << dye::green(Value32) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintBIOSInfo()
{
    std::cout << dye::aqua("=== BIOS Info ===") << std::endl;

    std::wstring KeyPath = L"HARDWARE\\DESCRIPTION\\System\\BIOS";

    std::string BiosVendor = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"BIOSVendor");
    std::string BiosReleaseDate = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"BIOSReleaseDate");
    std::string SystemManufacturer = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"SystemManufacturer");
    std::string SystemProductName = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"SystemProductName");

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    std::cout << dye::light_purple("  BIOSVendor: ") << dye::green(BiosVendor) << std::endl;
    std::cout << dye::light_purple("  BIOSReleaseDate: ") << dye::green(BiosReleaseDate) << std::endl;
    std::cout << dye::light_purple("  SystemManufacturer: ") << dye::green(SystemManufacturer) << std::endl;
    std::cout << dye::light_purple("  SystemProductName: ") << dye::green(SystemProductName) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintScsiDevices()
{
    std::wstring BaseKey = L"HARDWARE\\DEVICEMAP\\Scsi";
    std::cout << dye::aqua("=== Enumerating SCSI Devices ===") << std::endl;

    EnumerateScsiDevices(BaseKey);
    std::cout << std::endl;
}

void Registry::PrintNICsFromRegistry()
{
    std::wstring BaseKey = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}";

    HKEY KeyHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
    {
        std::string BaseKeyAnsi = UTILS::WStringToString(BaseKey);
        std::cout << dye::light_red("[-] Falha ao abrir chave: ") << BaseKeyAnsi << std::endl;
        return;
    }

    std::cout << dye::aqua("=== Network Interfaces ===") << std::endl;

    DWORD Index = 0;
    WCHAR SubKeyName[256];
    DWORD SubKeyNameLen;
    FILETIME LastWriteTime;

    while (true)
    {
        SubKeyNameLen = sizeof(SubKeyName) / sizeof(wchar_t);
        LONG Result = RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyNameLen,
            NULL, NULL, NULL, &LastWriteTime);

        if (Result == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        if (Result == ERROR_SUCCESS)
        {
            bool IsNumeric = true;
            for (size_t i = 0; i < wcslen(SubKeyName); ++i)
            {
                if (!iswdigit(SubKeyName[i]))
                {
                    IsNumeric = false;
                    break;
                }
            }

            if (!IsNumeric)
            {
                continue;
            }

            std::wstring SubKeyPath = BaseKey + L"\\" + SubKeyName;

            std::string DriverDesc = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"DriverDesc");
            std::string NetCfgInstanceId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"NetCfgInstanceId");
            std::string NetworkAddress = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"NetworkAddress");

            std::string ComponentId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"ComponentId");
            std::string DeviceInstanceID = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"DeviceInstanceID");
            std::string MatchingDeviceId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"MatchingDeviceId");

            if (!DriverDesc.empty() || !NetCfgInstanceId.empty() || !NetworkAddress.empty() || !ComponentId.empty())
            {
                std::string SubKeyPathAnsi = UTILS::WStringToString(SubKeyPath);

                std::cout << "[" << dye::purple("Registry Key") << "] -> "
                    << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + SubKeyPathAnsi) << std::endl;

                std::cout << dye::light_purple("  DriverDesc: ") << dye::green((DriverDesc.empty() ? "(not found)" : DriverDesc)) << std::endl;
                std::cout << dye::light_purple("  NetCfgInstanceId: ") << dye::green((NetCfgInstanceId.empty() ? "(not found)" : NetCfgInstanceId)) << std::endl;
                std::cout << dye::light_purple("  NetworkAddress: ") << dye::green((NetworkAddress.empty() ? "(not found)" : NetworkAddress)) << std::endl;

                std::cout << dye::light_purple("  ComponentId: ") << dye::green((ComponentId.empty() ? "(not found)" : ComponentId)) << std::endl;
                std::cout << dye::light_purple("  DeviceInstanceID: ") << dye::green((DeviceInstanceID.empty() ? "(not found)" : DeviceInstanceID)) << std::endl;
                std::cout << dye::light_purple("  MatchingDeviceId: ") << dye::green((MatchingDeviceId.empty() ? "(not found)" : MatchingDeviceId)) << std::endl;

                DWORD TimeType = 0;
                DWORD TimeSize = 0;
                if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"InstallTimeStamp", RRF_RT_REG_BINARY, &TimeType, nullptr, &TimeSize) == ERROR_SUCCESS)
                {
                    std::vector<BYTE> TimeBuffer(TimeSize);
                    if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"InstallTimeStamp", RRF_RT_REG_BINARY, nullptr, TimeBuffer.data(), &TimeSize) == ERROR_SUCCESS)
                    {
                        std::cout << dye::light_purple("  InstallTimeStamp: ");
                        for (size_t I = 0; I < TimeBuffer.size(); ++I)
                        {
                            std::cout << std::hex << std::setw(2) << std::setfill('0') << dye::green((int)TimeBuffer[I]) << " ";
                        }
                        std::cout << std::dec << std::endl;
                    }
                }
                else
                {
                    std::cout << dye::light_purple("  InstallTimeStamp: ") << dye::green("(not found)") << std::endl;
                }

                ULONGLONG InterfaceTimestamp = 0;
                DWORD QwordType = 0;
                DWORD QwordSize = sizeof(InterfaceTimestamp);
                if (RegGetValueW(HKEY_LOCAL_MACHINE, SubKeyPath.c_str(), L"NetworkInterfaceInstallTimestamp", RRF_RT_REG_QWORD, &QwordType, &InterfaceTimestamp, &QwordSize) == ERROR_SUCCESS)
                {
                    std::cout << dye::light_purple("  NetworkInterfaceInstallTimestamp: ")
                        << dye::green(InterfaceTimestamp) << std::endl;
                }
                else
                {
                    std::cout << dye::light_purple("  NetworkInterfaceInstallTimestamp: ") << dye::green("(not found)") << std::endl;
                }

                std::cout << std::endl;
            }
        }
    }

    RegCloseKey(KeyHandle);
}

void Registry::PrintCryptographyInfo()
{
    std::cout << dye::aqua("=== Cryptography Info ===") << std::endl;

    std::wstring KeyPath = L"SOFTWARE\\Microsoft\\Cryptography";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    std::string MachineGuid = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"MachineGuid");

    std::cout << dye::light_purple("  MachineGuid: ")
        << dye::green(MachineGuid) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintHwProfileInfo()
{
    std::cout << dye::aqua("=== Hardware Profile Info ===") << std::endl;

    const std::wstring BaseKey = L"SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles";
    HKEY KeyHandle;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
    {
        DWORD Index = 0;
        WCHAR SubKeyName[256];
        DWORD SubKeyLength = 256;

        while (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            std::wstring SubKeyPath = BaseKey + L"\\" + SubKeyName;

            std::string HwProfileGuid = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"HwProfileGuid");

            if (!HwProfileGuid.empty() && HwProfileGuid != "(not found)")
            {
                std::cout << "[" << dye::purple("Registry Key") << "] -> "
                    << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(SubKeyPath)) << std::endl;

                std::cout << dye::light_purple("  HwProfileGuid: ") << dye::green(HwProfileGuid) << std::endl;
                std::cout << std::endl;
            }

            SubKeyLength = 256;
        }

        RegCloseKey(KeyHandle);
    }
}

void Registry::PrintSystemInformation()
{
    std::cout << dye::aqua("=== System Information (Extended) ===") << std::endl;

    std::wstring KeyPath = L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    std::string ComputerHardwareId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"ComputerHardwareId");
    std::cout << dye::light_purple("  ComputerHardwareId: ") << dye::green(ComputerHardwareId) << std::endl;

    std::vector<std::string> HardwareIdsList;
    DWORD Type = 0;
    DWORD Size = 0;

    if (RegGetValueW(HKEY_LOCAL_MACHINE, KeyPath.c_str(), L"ComputerHardwareIds", RRF_RT_REG_MULTI_SZ, &Type, nullptr, &Size) == ERROR_SUCCESS)
    {
        std::vector<wchar_t> Buffer(Size / sizeof(wchar_t));

        if (RegGetValueW(HKEY_LOCAL_MACHINE, KeyPath.c_str(), L"ComputerHardwareIds", RRF_RT_REG_MULTI_SZ, nullptr, Buffer.data(), &Size) == ERROR_SUCCESS)
        {
            const wchar_t* Ptr = Buffer.data();
            while (*Ptr)
            {
                HardwareIdsList.push_back(UTILS::WStringToString(Ptr));
                Ptr += wcslen(Ptr) + 1;
            }
        }
    }

    if (!HardwareIdsList.empty())
    {
        std::cout << dye::light_purple("  ComputerHardwareIds: ");
        for (size_t I = 0; I < HardwareIdsList.size(); ++I)
        {
            if (I > 0)
            {
                std::cout << " | ";
            }
            std::cout << dye::green(HardwareIdsList[I]);
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << dye::light_purple("  ComputerHardwareIds: ") << dye::green("(not found)") << std::endl;
    }

    std::cout << std::endl;
}

void Registry::PrintInternetExplorerMigration()
{
    std::cout << dye::aqua("=== Internet Explorer Migration ===") << std::endl;

    std::wstring KeyPath = L"SOFTWARE\\Microsoft\\Internet Explorer\\Migration";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    std::string InstalledDate = ReadRegistryBinary(HKEY_LOCAL_MACHINE, KeyPath, L"IE Installed Date");

    std::cout << dye::light_purple("  IE Installed Date: ")
        << dye::green(InstalledDate) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintSQMClientInfo()
{
    std::cout << dye::aqua("=== SQMClient Info ===") << std::endl;

    std::wstring KeyPath = L"SOFTWARE\\Microsoft\\SQMClient";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    std::string MachineId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"MachineId");
    DWORD64 FirstSessionStartTime = ReadRegistryQword(HKEY_LOCAL_MACHINE, KeyPath, L"WinSqmFirstSessionStartTime");

    std::cout << dye::light_purple("  MachineId: ")
        << dye::green(MachineId) << std::endl;

    std::cout << dye::light_purple("  WinSqmFirstSessionStartTime: ")
        << std::hex << dye::green(FirstSessionStartTime) << std::dec << std::endl;

    std::cout << std::endl;
}

void Registry::PrintOneSettingsInfo()
{
    std::cout << dye::aqua("=== OneSettings Device IDs ===") << std::endl;

    std::vector<std::wstring> KeyPaths = {
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OneSettings\\WSD\\UpdateAgent\\QueryParameters",
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OneSettings\\WSD\\Setup360\\QueryParameters"
    };

    for (const auto& KeyPath : KeyPaths)
    {
        std::cout << "[" << dye::purple("Registry Key") << "] -> "
            << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

        std::string DeviceId = ReadRegistryStringA(HKEY_LOCAL_MACHINE, KeyPath, L"deviceId");

        std::cout << dye::light_purple("  deviceId: ")
            << dye::green(DeviceId) << std::endl;

        std::cout << std::endl;
    }
}

void Registry::PrintDiagTrackInfo()
{
    std::cout << dye::aqua("=== DiagTrack Info ===") << std::endl;

    std::wstring KeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Diagnostics\\DiagTrack\\SevilleEventlogManager";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    DWORD64 LastEventlogWrittenTime = ReadRegistryQword(HKEY_LOCAL_MACHINE, KeyPath, L"LastEventlogWrittenTime");

    std::cout << dye::light_purple("  LastEventlogWrittenTime: ")
        << dye::green(LastEventlogWrittenTime) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintSoftwareProtectionPlatform()
{
    std::cout << dye::aqua("=== Software Protection Platform ===") << std::endl;

    std::wstring KeyPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform\\Activation";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    DWORD64 ProductActivationTime = ReadRegistryQword(HKEY_LOCAL_MACHINE, KeyPath, L"ProductActivationTime");

    std::cout << dye::light_purple("  ProductActivationTime: ")
        << dye::green(ProductActivationTime) << std::endl;

    std::cout << std::endl;
}

void Registry::PrintDiskInfoFromStorage()
{
    std::cout << dye::aqua("=== Disk Info From Storage ===") << std::endl;

    std::wstring KeyPath = L"SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath))
        << std::endl;

    HKEY KeyHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, KeyPath.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
    {
        DWORD Index = 0;
        WCHAR ValueName[256];
        BYTE DataBuffer[1024];
        DWORD ValueNameLength;
        DWORD DataSize;
        DWORD Type;

        while (true)
        {
            ValueNameLength = 256;
            DataSize = sizeof(DataBuffer);

            if (RegEnumValueW(KeyHandle, Index, ValueName, &ValueNameLength, nullptr, &Type, DataBuffer, &DataSize) != ERROR_SUCCESS)
            {
                break;
            }

            if (Type == REG_SZ)
            {
                std::string DataAnsi = UTILS::WStringToString((WCHAR*)DataBuffer);

                std::cout << dye::light_purple("  Disk[") << dye::yellow(Index) << dye::light_purple("]: ")
                    << dye::green(DataAnsi) << std::endl;
            }

            Index++;
        }

        RegCloseKey(KeyHandle);
    }

    std::cout << std::endl;
}

void Registry::PrintPCIDevices()
{
    std::wstring BaseKey = L"SYSTEM\\CurrentControlSet\\Enum\\PCI";
    std::cout << dye::aqua("=== PCI Devices ===") << std::endl;

    HKEY KeyHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &KeyHandle) == ERROR_SUCCESS)
    {
        DWORD Index = 0;
        WCHAR SubKeyName[256];
        DWORD SubKeyLength;

        while (true)
        {
            SubKeyLength = 256;
            if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            {
                break;
            }

            std::wstring VendorPath = BaseKey + L"\\" + SubKeyName;
            HKEY SubKeyHandle;

            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, VendorPath.c_str(), 0, KEY_READ, &SubKeyHandle) == ERROR_SUCCESS)
            {
                DWORD DeviceIndex = 0;
                WCHAR DeviceKeyName[256];
                DWORD DeviceKeyLength;

                while (true)
                {
                    DeviceKeyLength = 256;
                    if (RegEnumKeyExW(SubKeyHandle, DeviceIndex++, DeviceKeyName, &DeviceKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                    {
                        break;
                    }

                    std::wstring DevicePath = VendorPath + L"\\" + DeviceKeyName;

                    std::cout << "[" << dye::purple("Registry Key") << "] -> "
                        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(DevicePath))
                        << std::endl;

                    DWORD Type = 0;
                    DWORD Size = 0;
                    if (RegGetValueW(HKEY_LOCAL_MACHINE, DevicePath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, &Type, nullptr, &Size) == ERROR_SUCCESS)
                    {
                        std::vector<wchar_t> Buffer(Size / sizeof(wchar_t));
                        if (RegGetValueW(HKEY_LOCAL_MACHINE, DevicePath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, nullptr, Buffer.data(), &Size) == ERROR_SUCCESS)
                        {
                            const wchar_t* Ptr = Buffer.data();
                            while (*Ptr)
                            {
                                std::cout << dye::light_purple("  HardwareID: ")
                                    << dye::green(UTILS::WStringToString(Ptr)) << std::endl;
                                Ptr += wcslen(Ptr) + 1;
                            }
                        }
                    }

                    std::cout << std::endl;
                }
                RegCloseKey(SubKeyHandle);
            }
        }
        RegCloseKey(KeyHandle);
    }
}

void Registry::PrintACPIDevices()
{
    std::wstring BaseKey = L"SYSTEM\\CurrentControlSet\\Enum\\ACPI";
    std::cout << dye::aqua("=== ACPI Devices ===") << std::endl;

    HKEY BaseHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &BaseHandle) == ERROR_SUCCESS)
    {
        DWORD DeviceIndex = 0;
        WCHAR DeviceKeyName[256];
        DWORD DeviceKeyLength;

        while (true)
        {
            DeviceKeyLength = 256;
            if (RegEnumKeyExW(BaseHandle, DeviceIndex++, DeviceKeyName, &DeviceKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            {
                break;
            }

            std::wstring DevicePath = BaseKey + L"\\" + DeviceKeyName;
            HKEY DeviceHandle;

            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, DevicePath.c_str(), 0, KEY_READ, &DeviceHandle) == ERROR_SUCCESS)
            {
                DWORD InstanceIndex = 0;
                WCHAR InstanceKeyName[256];
                DWORD InstanceKeyLength;

                while (true)
                {
                    InstanceKeyLength = 256;
                    if (RegEnumKeyExW(DeviceHandle, InstanceIndex++, InstanceKeyName, &InstanceKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
                    {
                        break;
                    }

                    std::wstring InstancePath = DevicePath + L"\\" + InstanceKeyName;

                    std::cout << "[" << dye::purple("Registry Key") << "] -> "
                        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(InstancePath))
                        << std::endl;

                    std::string DeviceDesc = ReadRegistryStringA(HKEY_LOCAL_MACHINE, InstancePath, L"DeviceDesc");
                    if (!DeviceDesc.empty())
                    {
                        std::cout << dye::light_purple("  DeviceDesc: ") << dye::green(DeviceDesc) << std::endl;
                    }

                    DWORD Type = 0;
                    DWORD Size = 0;
                    if (RegGetValueW(HKEY_LOCAL_MACHINE, InstancePath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, &Type, nullptr, &Size) == ERROR_SUCCESS)
                    {
                        std::vector<wchar_t> Buffer(Size / sizeof(wchar_t));
                        if (RegGetValueW(HKEY_LOCAL_MACHINE, InstancePath.c_str(), L"HardwareID", RRF_RT_REG_MULTI_SZ, nullptr, Buffer.data(), &Size) == ERROR_SUCCESS)
                        {
                            const wchar_t* Ptr = Buffer.data();
                            while (*Ptr)
                            {
                                std::cout << dye::light_purple("  HardwareID: ")
                                    << dye::green(UTILS::WStringToString(Ptr)) << std::endl;
                                Ptr += wcslen(Ptr) + 1;
                            }
                        }
                    }

                    std::cout << std::endl;
                }
                RegCloseKey(DeviceHandle);
            }
        }
        RegCloseKey(BaseHandle);
    }
}

void Registry::PrintHardwareProfiles()
{
    std::cout << dye::aqua("=== Hardware Profiles ===") << std::endl;

    std::wstring BaseKey = L"SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles";
    HKEY KeyHandle;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, BaseKey.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
    {
        return;
    }

    DWORD Index = 0;
    WCHAR SubKeyName[256];
    DWORD SubKeyLength;

    while (true)
    {
        SubKeyLength = 256;
        if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
        {
            break;
        }

        std::wstring SubKeyPath = BaseKey + L"\\" + SubKeyName;
        std::string HwProfileGuid = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"HwProfileGuid");

        if (!HwProfileGuid.empty() && HwProfileGuid != "(not found)")
        {
            std::cout << "[" << dye::purple("Registry Key") << "] -> "
                << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(SubKeyPath))
                << std::endl;

            std::cout << dye::light_purple("  HwProfileGuid: ")
                << dye::green(HwProfileGuid) << std::endl;

            std::cout << std::endl;
        }
    }

    RegCloseKey(KeyHandle);
}

void Registry::PrintUSBDevices()
{
    EnumPnPTwoLevel(L"SYSTEM\\CurrentControlSet\\Enum\\USB", L"USB Devices (Enum\\USB)");
    EnumPnPTwoLevel(L"SYSTEM\\CurrentControlSet\\Enum\\USBSTOR", L"USB Storage (Enum\\USBSTOR)");
}

void Registry::PrintHIDDevices()
{
    EnumPnPTwoLevel(L"SYSTEM\\CurrentControlSet\\Enum\\HID", L"HID Devices (Enum\\HID)");
}

void Registry::PrintBluetoothDevices()
{
    std::cout << dye::aqua("=== Bluetooth Devices ===") << std::endl;

    std::wstring PortKeyPath = L"SYSTEM\\CurrentControlSet\\Services\\BTHPORT\\Parameters\\Devices";
    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(PortKeyPath)) << std::endl;

    HKEY PortKeyHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, PortKeyPath.c_str(), 0, KEY_READ, &PortKeyHandle) == ERROR_SUCCESS)
    {
        std::vector<std::wstring> WantedValues = {
            L"FingerprintString", L"LastConnected", L"LastSeen",
            L"LeContainerId", L"LEName", L"Name"
        };

        DWORD Index = 0;
        WCHAR SubKeyName[256];
        DWORD SubKeyLength;

        while (true)
        {
            SubKeyLength = 256;
            if (RegEnumKeyExW(PortKeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            {
                break;
            }

            std::wstring DevicePath = PortKeyPath + L"\\" + SubKeyName;
            std::string MacAddress = UTILS::WStringToString(SubKeyName);

            std::cout << "  [" << dye::purple("MAC") << "] -> " << dye::green(MacAddress) << std::endl;

            for (const auto& ValueName : WantedValues)
            {
                DWORD Type = 0;
                DWORD DataSize = 0;

                if (RegGetValueW(HKEY_LOCAL_MACHINE, DevicePath.c_str(), ValueName.c_str(), RRF_RT_ANY, &Type, nullptr, &DataSize) == ERROR_SUCCESS)
                {
                    std::vector<BYTE> DataBuffer(DataSize);

                    if (RegGetValueW(HKEY_LOCAL_MACHINE, DevicePath.c_str(), ValueName.c_str(), RRF_RT_ANY, nullptr, DataBuffer.data(), &DataSize) == ERROR_SUCCESS)
                    {
                        std::string ValueNameAnsi = UTILS::WStringToString(ValueName);
                        std::cout << "    " << dye::light_purple(ValueNameAnsi) << ": ";

                        if (Type == REG_SZ)
                        {
                            std::wstring StrValue((wchar_t*)DataBuffer.data());
                            if (!StrValue.empty() && StrValue.back() == L'\0') StrValue.pop_back();

                            std::cout << dye::green(UTILS::WStringToString(StrValue));
                        }
                        else if (Type == REG_DWORD)
                        {
                            DWORD Val = *(DWORD*)DataBuffer.data();
                            std::cout << dye::yellow(std::to_string(Val));
                        }
                        else if (Type == REG_QWORD)
                        {
                            unsigned long long Val = *(unsigned long long*)DataBuffer.data();
                            std::cout << dye::yellow(std::to_string(Val));
                        }
                        else if (Type == REG_BINARY)
                        {
                            for (size_t k = 0; k < DataSize; k++)
                            {
                                std::cout << dye::green(std::hex) << std::setw(2) << std::setfill('0') << (int)DataBuffer[k];
                            }
                            std::cout << std::dec;
                        }
                        else
                        {
                            std::cout << dye::red("(type not supported in display)");
                        }
                        std::cout << std::endl;
                    }
                }
            }
            std::cout << std::endl;
        }
        RegCloseKey(PortKeyHandle);
    }

    EnumPnPTwoLevel(L"SYSTEM\\CurrentControlSet\\Enum\\BTHENUM", L"Bluetooth (Enum\\BTHENUM)");

    std::cout << std::endl;
}

void Registry::PrintUserProfileList()
{
    std::cout << dye::aqua("=== User Profile List (ProfileList) ===") << std::endl;

    std::wstring KeyPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    HKEY KeyHandle;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, KeyPath.c_str(), 0, KEY_READ, &KeyHandle) != ERROR_SUCCESS)
    {
        return;
    }

    DWORD Index = 0;
    WCHAR SubKeyName[256];
    DWORD SubKeyLength;

    while (true)
    {
        SubKeyLength = 256;
        if (RegEnumKeyExW(KeyHandle, Index++, SubKeyName, &SubKeyLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
        {
            break;
        }

        std::wstring SubKeyPath = KeyPath + L"\\" + SubKeyName;

        std::string ProfileImagePath = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"ProfileImagePath");
        std::string ProfileGuid = ReadRegistryStringA(HKEY_LOCAL_MACHINE, SubKeyPath, L"ProfileGuid");

        if ((!ProfileImagePath.empty() && ProfileImagePath != "(not found)") || (!ProfileGuid.empty() && ProfileGuid != "(not found)"))
        {
            std::string SidAnsi = UTILS::WStringToString(SubKeyName);

            std::cout << "  [" << dye::purple("SID") << "] -> " << dye::green(SidAnsi) << std::endl;

            if (!ProfileImagePath.empty() && ProfileImagePath != "(not found)")
            {
                std::cout << dye::light_purple("    ProfileImagePath: ") << dye::yellow(ProfileImagePath) << std::endl;
            }

            if (!ProfileGuid.empty() && ProfileGuid != "(not found)")
            {
                std::cout << dye::light_purple("    ProfileGuid: ") << dye::yellow(ProfileGuid) << std::endl;
            }
        }
    }
    RegCloseKey(KeyHandle);

    std::cout << std::endl;
}

void Registry::PrintTPMEndorsementInfo()
{
    std::cout << dye::aqua("=== TPM Endorsement Info ===") << std::endl;

    std::wstring KeyPath = L"SYSTEM\\CurrentControlSet\\Services\\TPM\\WMI\\Endorsement";
    HKEY KeyHandle;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, KeyPath.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &KeyHandle) != ERROR_SUCCESS)
    {
        return;
    }

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath)) << std::endl;

    DWORD Index = 0;
    WCHAR ValueName[256];
    DWORD ValueNameLength;
    DWORD Type;
    DWORD DataSize;
    BYTE DataBuffer[4096];

    while (true)
    {
        ValueNameLength = sizeof(ValueName) / sizeof(WCHAR);
        DataSize = sizeof(DataBuffer);

        LONG Result = RegEnumValueW(KeyHandle, Index++, ValueName, &ValueNameLength, nullptr, &Type, DataBuffer, &DataSize);

        if (Result != ERROR_SUCCESS)
        {
            break;
        }

        std::string ValueNameAnsi = UTILS::WStringToString(ValueName);

        std::cout << dye::light_purple("  " + ValueNameAnsi + ": ");

        if (Type == REG_SZ || Type == REG_EXPAND_SZ)
        {
            std::string DataStr = UTILS::WStringToString((WCHAR*)DataBuffer);
            std::cout << dye::green(DataStr) << std::endl;
        }
        else if (Type == REG_DWORD)
        {
            DWORD Value = *(DWORD*)DataBuffer;
            std::cout << dye::green(Value) << std::endl;
        }
        else if (Type == REG_QWORD)
        {
            DWORD64 Value = *(DWORD64*)DataBuffer;
            std::cout << dye::green(Value) << std::endl;
        }
        else if (Type == REG_BINARY)
        {
            for (DWORD I = 0; I < DataSize; I++)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << dye::green((int)DataBuffer[I]) << " ";
            }
            std::cout << std::dec << std::endl;
        }
        else
        {
            std::cout << dye::red("(unsupported type)") << std::endl;
        }
    }

    RegCloseKey(KeyHandle);
    std::cout << std::endl;
}

void Registry::PrintSMBIOS()
{
    std::cout << dye::aqua("=== SMBIOS ===") << std::endl;

    std::wstring KeyPath = L"SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data";

    std::cout << "[" << dye::purple("Registry Key") << "] -> "
        << dye::light_yellow("HKEY_LOCAL_MACHINE\\" + UTILS::WStringToString(KeyPath))
        << std::endl << std::endl;

    RawSMBIOSData* RawData = GetSMBIOS();

    if (!RawData)
    {
        return;
    }

    const BYTE* CurrentPtr = RawData->SMBIOSTableData;
    const BYTE* EndPtr = reinterpret_cast<const BYTE*>(RawData) + RawData->Length;

    while (CurrentPtr < EndPtr)
    {
        const SMBIOSStruct* Header = reinterpret_cast<const SMBIOSStruct*>(CurrentPtr);

        if (Header->Length < 4)
        {
            break;
        }

        switch (Header->Type)
        {
        case 0:
            SMBIOS::QueryBiosInfo(reinterpret_cast<const SMBIOSStructType0*>(Header));
            break;
        case 1:
            SMBIOS::QuerySystemInfo(reinterpret_cast<const SMBIOSStructType1*>(Header));
            break;
        case 2:
            SMBIOS::QueryBaseboardInfo(reinterpret_cast<const SMBIOSStructType2*>(Header));
            break;
        case 3:
            SMBIOS::QueryEnclosureInfo(reinterpret_cast<const SMBIOSStructType3*>(Header));
            break;
        case 4:
            SMBIOS::QueryProcessorInfo(reinterpret_cast<const SMBIOSStructType4*>(Header));
            break;
        case 17:
            SMBIOS::QueryMemoryDeviceInfo(reinterpret_cast<const SMBIOSStructType17*>(Header));
            break;
        }

        if (Header->Type == 127)
        {
            break;
        }

        CurrentPtr = reinterpret_cast<const BYTE*>(SMBIOS::GetNextStruct(Header));
    }
}

void Registry::QueryAllInfo()
{
    UTILS::PrintFixedBox("Registry");

    EnumerateMonitors();
    EnumerateGraphicsDrivers();
    PrintMotherboardUUIDs();
    PrintNvidiaUUIDs();
    PrintVolumes();
    PrintExtraHardware();
    PrintUEFI_ESRT();
    PrintVideoDeviceMap();
    PrintVideoRegistry();
    PrintNTCurrentVersion();
    PrintWindowsUpdate();
    PrintWAT();
    PrintBIOSInfo();
    PrintScsiDevices();
    PrintNICsFromRegistry();
    PrintCryptographyInfo();
    PrintHwProfileInfo();
    PrintSystemInformation();
    PrintInternetExplorerMigration();
    PrintSQMClientInfo();
    PrintOneSettingsInfo();
    PrintDiagTrackInfo();
    PrintSoftwareProtectionPlatform();
    PrintDiskInfoFromStorage();
    PrintPCIDevices();
    PrintACPIDevices();
    PrintHardwareProfiles();
    PrintUSBDevices();
    PrintHIDDevices();
    PrintBluetoothDevices();
    PrintUserProfileList();
    PrintTPMEndorsementInfo();
    PrintSMBIOS();
}