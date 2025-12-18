#include "pch.h"

void UsnJrnl::QueryUsnJournal(const std::wstring& volumePath)
{
    HANDLE hVol = CreateFileW(
        volumePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (hVol == INVALID_HANDLE_VALUE)
    {
        return;
    }

    USN_JOURNAL_DATA JournalData{};

    JournalIOCTL.SetIOCTL(hVol, FSCTL_QUERY_USN_JOURNAL, "FSCTL_QUERY_USN_JOURNAL");

    if (JournalIOCTL.IOCTL(nullptr, 0, &JournalData, sizeof(JournalData)))
    {
        JournalIOCTL.PrintJournalID(JournalData.UsnJournalID, "USN Journal ID");
    }

    CloseHandle(hVol);
}

void UsnJrnl::EnumerateUsnJournals()
{
    WCHAR VolumeName[MAX_PATH] = { 0 };
    HANDLE hFind = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        std::wstring VolPath = VolumeName;

        if (!VolPath.empty() && VolPath.back() == L'\\')
        {
            VolPath.pop_back();
        }

        WCHAR FsName[MAX_PATH] = { 0 };

        if (GetVolumeInformationW(VolumeName, nullptr, 0, nullptr, nullptr, nullptr, FsName, ARRAYSIZE(FsName)))
        {
            if (wcscmp(FsName, L"NTFS") == 0)
            {
                std::wstringstream wss;
                wss << L"Volume: " << VolPath;

                std::string volume = UTILS::WStringToString(wss.str());
                std::cout << "[" << dye::aqua(volume) << "]\n";

                QueryUsnJournal(VolPath);
            }
            std::cout << std::endl;
        }

    } while (FindNextVolumeW(hFind, VolumeName, ARRAYSIZE(VolumeName)));

    FindVolumeClose(hFind);
}

void UsnJrnl::QueryAllInfo()
{
    UTILS::PrintFixedBox("USN Journal ID");
    EnumerateUsnJournals();
}
