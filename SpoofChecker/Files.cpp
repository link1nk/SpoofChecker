#include "pch.h"

std::string Files::ReadMachineGuidFromShadow(const std::wstring& ShadowPath)
{
    std::wstring FilePath = ShadowPath + L"Windows\\System32\\restore\\MachineGuid.txt";

    std::string Bytes;
    if (!UTILS::ReadFileBytes(FilePath, Bytes))
    {
        return "";
    }

    std::string Hex32 = UTILS::ExtractGUIDHexUpper(Bytes);
    if (Hex32.size() != 32)
    {
        return "";
    }

    return UTILS::Hex32ToGUID(Hex32);
}

std::vector<std::wstring> Files::GetShadowCopies(int MaxCheck)
{
    std::vector<std::wstring> Copies;
    for (int Index = 1; Index < MaxCheck; Index++)
    {
        std::wstringstream PathStream;
        PathStream << L"\\\\?\\GLOBALROOT\\Device\\HarddiskVolumeShadowCopy" << Index << L"\\";
        std::wstring TestPath = PathStream.str() + L"Windows\\System32\\restore\\MachineGuid.txt";

        DWORD Attributes = GetFileAttributesW(TestPath.c_str());
        if (Attributes != INVALID_FILE_ATTRIBUTES && !(Attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            Copies.push_back(PathStream.str());
        }
    }
    return Copies;
}

std::vector<std::wstring> Files::GetLogFiles()
{
    WCHAR WindowsDirectory[MAX_PATH];
    GetWindowsDirectoryW(WindowsDirectory, MAX_PATH);

    std::vector<std::wstring> LogFiles;
    LogFiles.push_back(std::wstring(WindowsDirectory) + L"\\INF\\setupapi.dev.log");
    LogFiles.push_back(std::wstring(WindowsDirectory) + L"\\INF\\setupapi.setup.log");
    return LogFiles;
}

std::vector<Files::ParsedEntry> Files::ParseLogFile(const std::wstring& Path)
{
    std::vector<ParsedEntry> Entries;
    std::wifstream FileStream(Path);
    if (!FileStream.is_open())
    {
        std::wcerr << L"[!] Unable to open: " << Path << std::endl;
        return Entries;
    }

    std::wstring Line;
    std::wregex HwidRegex(L"(USBSTOR[#\\\\][^\\s]+|USB[#\\\\][^\\s]+|PCI[#\\\\][^\\s]+|SCSI[#\\\\][^\\s]+|HID[#\\\\][^\\s]+)");
    std::wregex DevInstRegex(L"Device Instance Id\\s*[=:]\\s*([^\\s]+)");

    while (std::getline(FileStream, Line))
    {
        std::wsmatch Match;

        if (std::regex_search(Line, Match, HwidRegex))
        {
            ParsedEntry entry;
            entry.hwid = Match.str(1);

            size_t pos = entry.hwid.find_last_of(L'\\');

            if (pos == std::wstring::npos)
            {
                pos = entry.hwid.find_last_of(L'#');
            }

            if (pos != std::wstring::npos && pos + 1 < entry.hwid.size())
            {
                std::wstring rawSerial = entry.hwid.substr(pos + 1);
                size_t guidPos = rawSerial.find(L'#');

                if (guidPos != std::wstring::npos)
                {
                    entry.serial = rawSerial.substr(0, guidPos);
                }
                else
                {
                    entry.serial = rawSerial;
                }
            }

            Entries.push_back(entry);
        }
        else if (std::regex_search(Line, Match, DevInstRegex))
        {
            ParsedEntry Entry;
            Entry.hwid = Match.str(1);

            size_t Position = Entry.hwid.find_last_of(L'\\');
            if (Position != std::wstring::npos && Position + 1 < Entry.hwid.size())
            {
                Entry.serial = Entry.hwid.substr(Position + 1);
            }

            Entries.push_back(Entry);
        }
    }

    FileStream.close();
    return Entries;
}

void Files::CheckRecycleBin(const std::wstring& VolumePath)
{
    std::wstring RecyclePath = VolumePath + L"\\$Recycle.Bin";
    if (!std::filesystem::exists(RecyclePath))
    {
        std::string RecyclePathAnsi = UTILS::WStringToString(RecyclePath);

        std::cout << dye::light_red("[!] ") << dye::light_red(RecyclePathAnsi) << dye::light_red(" not found.") << std::endl;
        std::cout << std::endl;
        return;
    }

    std::string VolumePathAnsi = UTILS::WStringToString(VolumePath);

    std::cout << "[" << dye::purple(VolumePathAnsi) << "]" << std::endl;

    bool Found = false;

    for (const auto& Entry : std::filesystem::directory_iterator(RecyclePath))
    {
        if (Entry.is_directory())
        {
            std::wstring FolderName = Entry.path().filename().wstring();
            if (FolderName.rfind(L"S-", 0) == 0)
            {
                ResolveSID(FolderName);
                Found = true;
            }
        }
    }

    if (!Found)
    {
        std::cout << dye::light_purple("  SID: ") << dye::light_red("Not found any SID");
    }

    std::cout << std::endl;
}

void Files::PrintShadowCopies()
{
    std::cout << dye::aqua("=== Searching for Shadow Copies ===") << std::endl;

    auto Shadows = GetShadowCopies();
    if (Shadows.empty())
    {
        std::cout << dye::light_red("[-] No shadow copies with MachineGuid.txt found.\n") << std::endl;
        return;
    }

    for (auto Shadow : Shadows)
    {
        std::string Guid = ReadMachineGuidFromShadow(Shadow);
        std::string ShadowAnsi = UTILS::WStringToString(Shadow);

        std::cout << dye::light_purple("[ShadowCopy] ") << dye::green(ShadowAnsi);
        if (!Guid.empty())
        {
            std::cout << "\n" << dye::light_purple("  MachineGuid: ") << dye::green(Guid) << "\n";
        }
        else
        {
            std::cout << " -> " << dye::light_red("(No MachineGuid or invalid format)\n");
        }
    }
    std::cout << std::endl;
}

void Files::ResolveSID(const std::wstring& SidString)
{
    PSID SidPointer = nullptr;
    if (!ConvertStringSidToSidW(SidString.c_str(), &SidPointer))
    {
        std::string SidStringAnsi = UTILS::WStringToString(SidString);

        std::cout << dye::light_red("  [!] Failed to convert SID: ") << dye::light_red(SidStringAnsi) << std::endl;
        return;
    }

    WCHAR Name[256], Domain[256];
    DWORD NameLength = 256, DomainLength = 256;
    SID_NAME_USE SidType;

    std::string SidStringAnsi = UTILS::WStringToString(SidString);

    if (LookupAccountSidW(NULL, SidPointer, Name, &NameLength, Domain, &DomainLength, &SidType))
    {
        std::string DomainAnsi = UTILS::WStringToString(Domain);
        std::string NameAnsi = UTILS::WStringToString(Name);

        std::cout << dye::light_purple("  SID: ") << dye::green(SidStringAnsi)
            << " -> [" << dye::yellow(DomainAnsi) << "\\" << dye::yellow(NameAnsi) << "]" << std::endl;
    }
    else
    {
        std::cout << dye::light_purple("  SID: ") << dye::green(SidStringAnsi) << dye::yellow(" (unresolved)") << std::endl;
    }

    LocalFree(SidPointer);
}

void Files::PrintSIDFromRecycleBin()
{
    std::cout << dye::aqua("=== SIDs From Recycle Bin ===") << std::endl;

    WCHAR VolumeName[MAX_PATH];
    HANDLE FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        std::wstring VolumePath(VolumeName);

        if (VolumePath.back() == L'\\')
        {
            VolumePath.pop_back();
        }

        CheckRecycleBin(VolumePath);

    } while (FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName)));

    std::cout << std::endl;
}

void Files::PrintHwidSerialFromLogFiles(bool Verbose)
{
    std::cout << dye::aqua("=== HWIDs From Log Files ===") << std::endl;

    auto LogFiles = GetLogFiles();
    for (auto& LogFile : LogFiles)
    {
        auto Entries = ParseLogFile(LogFile);

        std::string LogFileAnsi = UTILS::WStringToString(LogFile);

        std::cout << "[" << dye::purple(LogFileAnsi) << "]\n";
        std::cout << dye::light_purple("  Total HWIDs found: ") << std::dec << dye::green(Entries.size()) << std::endl;

        if (Verbose)
        {
            for (auto& Entry : Entries)
            {
                std::wcout << L"[HWID] " << Entry.hwid;
                if (!Entry.serial.empty())
                {
                    std::wcout << L"  |  [Serial] " << Entry.serial;
                }
                std::wcout << std::endl;
            }
        }

        std::cout << std::endl;
    }
}

void Files::QueryAllInfo()
{
    UTILS::PrintFixedBox("Files");

    PrintShadowCopies();
    PrintSIDFromRecycleBin();
    PrintHwidSerialFromLogFiles();
}