#include "pch.h"

void Volumes::PrintVolumeSerial_BootSector(std::wstring VolumePath)
{
    if (VolumePath.back() == L'\\')
    {
        VolumePath.pop_back();
    }

    HANDLE VolumeHandle = CreateFileW(
        VolumePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (VolumeHandle == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"[BootSector]: Failed to open volume " << VolumePath << L". Error: " << GetLastError() << "\n\n";
        return;
    }

    BYTE SectorBuffer[512] = { 0 };
    DWORD BytesRead = 0;

    if (!ReadFile(VolumeHandle, SectorBuffer, sizeof(SectorBuffer), &BytesRead, nullptr) || BytesRead < 64)
    {
        std::cerr << "[" << dye::light_purple("BootSector") << "]: Failed to read sector. Code: " << GetLastError() << "\n\n";
        CloseHandle(VolumeHandle);
        return;
    }

    DWORD SerialNumber = 0;

    if (memcmp(&SectorBuffer[3], "NTFS", 4) == 0)
    {
        SerialNumber = *(DWORD*)&SectorBuffer[0x48];
    }
    else if (memcmp(&SectorBuffer[82], "FAT32", 5) == 0)
    {
        SerialNumber = *(DWORD*)&SectorBuffer[0x43];
    }
    else if (memcmp(&SectorBuffer[54], "FAT", 3) == 0)
    {
        SerialNumber = *(DWORD*)&SectorBuffer[0x27];
    }
    else
    {
        std::cerr << "[" << dye::light_purple("BootSector") << "]: Unidentified file system.\n\n";
        CloseHandle(VolumeHandle);
        return;
    }

    std::wstringstream StringStream;

    StringStream << std::hex << std::uppercase
        << ((SerialNumber >> 16) & 0xFFFF)
        << L"-"
        << (SerialNumber & 0xFFFF)
        << std::dec << std::nouppercase
        << std::endl;

    std::string SerialString = UTILS::WStringToString(StringStream.str());

    std::cout << "[" << dye::light_purple("BootSector") << "] - Serial: " << dye::green(SerialString);

    CloseHandle(VolumeHandle);
}

void Volumes::PrintVolumeSerial(const std::wstring& VolumePath)
{
    DWORD SerialNumber = 0;
    DWORD MaxComponentLen = 0;
    DWORD FileSystemFlags = 0;
    WCHAR FileSystemName[MAX_PATH] = { 0 };
    WCHAR VolumeName[MAX_PATH] = { 0 };

    std::wstring Path = VolumePath;
    if (Path.back() != L'\\')
    {
        Path += L"\\";
    }

    BOOL Success = GetVolumeInformationW(
        Path.c_str(),
        VolumeName,
        ARRAYSIZE(VolumeName),
        &SerialNumber,
        &MaxComponentLen,
        &FileSystemFlags,
        FileSystemName,
        ARRAYSIZE(FileSystemName)
    );

    if (Success)
    {
        std::cout << "[" << dye::light_purple("GetVolumeInformationW") << "]" << " - Serial : "
            << std::hex << std::uppercase
            << dye::green(((SerialNumber >> 16) & 0xFFFF)) << dye::green("-") << dye::green((SerialNumber & 0xFFFF))
            << std::dec << std::nouppercase << std::endl;
    }
    else
    {
        std::cerr << "Error retrieving information. Code: " << GetLastError() << std::endl;
    }
}

void Volumes::PrintVolumeUniqueId(const std::wstring& VolumePath)
{
    std::string SymbolicLinkName = UTILS::WStringToString(VolumePath);
    std::cout << "# SymbolicLinkName: " << dye::green(SymbolicLinkName) << std::endl;

    HANDLE DeviceHandle = CreateFileW(
        VolumePath.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (DeviceHandle == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Error opening volume. Code: " << GetLastError() << std::endl;
        return;
    }

    BYTE Buffer[512] = { 0 };
    VolumesIOCTL.SetIOCTL(DeviceHandle, IOCTL_MOUNTDEV_QUERY_UNIQUE_ID, "IOCTL_MOUNTDEV_QUERY_UNIQUE_ID");

    if (VolumesIOCTL.IOCTL(nullptr, 0, Buffer, sizeof(Buffer)))
    {
        PMOUNTDEV_UNIQUE_ID UniqueId = reinterpret_cast<PMOUNTDEV_UNIQUE_ID>(Buffer);
        VolumesIOCTL.PrintHex(UniqueId->UniqueId, UniqueId->UniqueIdLength, "UniqueID");
    }

    CloseHandle(DeviceHandle);
}

void Volumes::PrintVolumeSerial_FSCTL(const std::wstring& VolumePath)
{
    HANDLE VolumeHandle = CreateFileW(VolumePath.c_str(),
        GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);

    if (VolumeHandle == INVALID_HANDLE_VALUE)
    {
        printf("Erro: %lu\n", GetLastError());
        return;
    }

    NTFS_VOLUME_DATA_BUFFER NtfsData = { 0 };

    VolumesIOCTL.SetIOCTL(VolumeHandle, FSCTL_GET_NTFS_VOLUME_DATA, "FSCTL_GET_NTFS_VOLUME_DATA");

    if (VolumesIOCTL.IOCTL(nullptr, 0, &NtfsData, sizeof(NtfsData)))
    {
        std::ostringstream StringStream;

        StringStream << std::uppercase << std::setfill('0')
            << std::setw(8) << std::hex << (DWORD)(NtfsData.VolumeSerialNumber.HighPart)
            << "-"
            << std::setw(8) << std::hex << (DWORD)(NtfsData.VolumeSerialNumber.LowPart)
            << std::dec
            << std::endl;

        std::cout << "[" << dye::light_purple("FSCTL_GET_NTFS_VOLUME_DATA") << "] Serial: " << dye::green(StringStream.str());
    }

    CloseHandle(VolumeHandle);
    return;
}

void Volumes::EnumerateVolumes()
{
    WCHAR VolumeName[MAX_PATH] = { 0 };

    HANDLE FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Erro ao enumerar volumes. Código: " << GetLastError() << std::endl;
        return;
    }

    do
    {
        std::wstring VolumePath(VolumeName);
        if (VolumePath.back() == L'\\')
        {
            VolumePath.pop_back();
        }

        PrintVolumeUniqueId(VolumePath);
        PrintVolumeSerial(VolumeName);
        PrintVolumeSerial_BootSector(VolumeName);

        WCHAR FileSystemName[MAX_PATH] = { 0 };
        if (GetVolumeInformationW(std::wstring{ VolumeName }.c_str(), nullptr, 0, nullptr, nullptr, nullptr, FileSystemName, ARRAYSIZE(FileSystemName)))
        {
            if (wcscmp(FileSystemName, L"NTFS") == 0)
            {
                PrintVolumeSerial_FSCTL(VolumePath);
            }
            std::cout << std::endl;
        }
        else
        {
            std::wcerr << L"Error GetVolumeInformationW: " << GetLastError() << std::endl;
        }

    } while (FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName)));

    FindVolumeClose(FindHandle);
}

void Volumes::PrintVolumesUniqueId_UsingMountMgr()
{
    HANDLE MountMgrHandle = CreateFileW(
        L"\\\\.\\MountPointManager",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (MountMgrHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Erro ao abrir MountPointManager. Codigo: " << GetLastError() << std::endl;
        return;
    }

    MOUNTMGR_MOUNT_POINT Input = {};

    std::vector<BYTE> Buffer(64 * 1024);

    while (true)
    {
        VolumesIOCTL.SetIOCTL(MountMgrHandle, IOCTL_MOUNTMGR_QUERY_POINTS, "IOCTL_MOUNTMGR_QUERY_POINTS");

        if (VolumesIOCTL.IOCTL(&Input, sizeof(Input), Buffer.data(), (DWORD)Buffer.size()))
        {
            auto Points = reinterpret_cast<PMOUNTMGR_MOUNT_POINTS>(Buffer.data());
            if (Points->Size > Buffer.size())
            {
                Buffer.resize(Points->Size);
                continue;
            }
            break;
        }
        else
        {
            CloseHandle(MountMgrHandle);
            return;
        }
    }

    PMOUNTMGR_MOUNT_POINTS Points = reinterpret_cast<PMOUNTMGR_MOUNT_POINTS>(Buffer.data());

    for (ULONG Index = 0; Index < Points->NumberOfMountPoints; ++Index)
    {
        const MOUNTMGR_MOUNT_POINT& MountPoint = Points->MountPoints[Index];

        std::wstring SymbolicLink;
        if (MountPoint.SymbolicLinkNameLength > 0)
        {
            auto Ptr = reinterpret_cast<const WCHAR*>(Buffer.data() + MountPoint.SymbolicLinkNameOffset);
            SymbolicLink.assign(Ptr, MountPoint.SymbolicLinkNameLength / sizeof(WCHAR));
        }

        std::wstring DeviceName;
        if (MountPoint.DeviceNameLength > 0)
        {
            auto Ptr = reinterpret_cast<const WCHAR*>(Buffer.data() + MountPoint.DeviceNameOffset);
            DeviceName.assign(Ptr, MountPoint.DeviceNameLength / sizeof(WCHAR));
        }

        std::string UniqueIdHex;
        if (MountPoint.UniqueIdLength > 0)
        {
            auto Ptr = reinterpret_cast<const UCHAR*>(Buffer.data() + MountPoint.UniqueIdOffset);

            std::string SymbolicLinkAnsi = UTILS::WStringToString(SymbolicLink);

            std::cout << "# SymbolicLinkName : "
                << (SymbolicLinkAnsi.empty() ? dye::red("(NO LINK)") : dye::green(SymbolicLinkAnsi))
                << std::endl;

            VolumesIOCTL.PrintHex(Ptr, MountPoint.UniqueIdLength, "UniqueID");
            std::wcout << std::endl;
        }
        else
        {
            if (!SymbolicLink.empty() || !DeviceName.empty())
            {
                std::wcout << L"# SymbolicLinkName : " << (SymbolicLink.empty() ? L"(NO LINK)" : SymbolicLink) << std::endl;
                std::wcout << L"UniqueId  : (Unavailable)" << std::endl;
            }
        }
    }

    CloseHandle(MountMgrHandle);
}

void Volumes::QueryAllInfo()
{
    UTILS::PrintFixedBox("Volumes");

    PrintVolumesUniqueId_UsingMountMgr();

    EnumerateVolumes();
}