#pragma once

#include "pch.h"

class UTILS
{
public:
    static void PrintFixedBox(const std::string& Text, int Width = 50)
    {
        if (Width < 4)
        {
            Width = 4;
        }

        std::vector<std::string> Lines;
        std::stringstream StringStream(Text);
        std::string Line;

        while (std::getline(StringStream, Line, '\n'))
        {
            Lines.push_back(Line);
        }

        std::cout << dye::light_green(std::string(Width, '=')) << "\n";

        for (auto& CurrentLine : Lines)
        {
            int InnerWidth = Width - 2;
            std::string Trimmed = CurrentLine;

            if (Trimmed.size() > static_cast<size_t>(InnerWidth))
            {
                Trimmed = Trimmed.substr(0, InnerWidth);
            }

            int PaddingLeft = (InnerWidth - static_cast<int>(Trimmed.size())) / 2;
            int PaddingRight = InnerWidth - static_cast<int>(Trimmed.size()) - PaddingLeft;

            std::cout
                << dye::light_green("|")
                << std::string(PaddingLeft, ' ')
                << dye::yellow(Trimmed)
                << std::string(PaddingRight, ' ')
                << dye::light_green("|")
                << "\n";
        }

        std::cout << dye::light_green(std::string(Width, '=')) << "\n\n";
    }

    static std::string WStringToString(const std::wstring& WStr)
    {
        if (WStr.empty())
        {
            return std::string();
        }

        int SizeNeeded = WideCharToMultiByte(
            CP_UTF8, 0,
            WStr.c_str(), (int)WStr.size(),
            nullptr, 0, nullptr, nullptr
        );

        std::string Str(SizeNeeded, 0);

        WideCharToMultiByte(
            CP_UTF8, 0,
            WStr.c_str(), (int)WStr.size(),
            &Str[0], SizeNeeded,
            nullptr, nullptr
        );

        return Str;
    }

    static std::string RunPowerShell(const std::wstring& Command)
    {
        std::wstring FullCommand = L"powershell -NoProfile -Command \"" + Command + L"\"";

        HANDLE ReadHandle, WriteHandle;
        SECURITY_ATTRIBUTES SecurityAttributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

        if (!CreatePipe(&ReadHandle, &WriteHandle, &SecurityAttributes, 0))
        {
            return "";
        }

        SetHandleInformation(ReadHandle, HANDLE_FLAG_INHERIT, 0);

        PROCESS_INFORMATION ProcessInfo{};
        STARTUPINFOW StartupInfo{};

        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.hStdError = WriteHandle;
        StartupInfo.hStdOutput = WriteHandle;
        StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

        if (!CreateProcessW(
            nullptr,
            &FullCommand[0],
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &StartupInfo,
            &ProcessInfo))
        {
            CloseHandle(ReadHandle);
            CloseHandle(WriteHandle);
            return "";
        }

        CloseHandle(WriteHandle);

        std::string Output;
        char Buffer[4096];
        DWORD BytesRead;

        while (ReadFile(ReadHandle, Buffer, sizeof(Buffer) - 1, &BytesRead, nullptr) && BytesRead > 0)
        {
            Buffer[BytesRead] = '\0';
            Output += Buffer;
        }

        CloseHandle(ReadHandle);
        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);

        return Output;
    }

    static std::vector<std::string> SplitLines(const std::string& Str)
    {
        std::vector<std::string> Lines;
        std::istringstream StringStream(Str);
        std::string Line;

        while (std::getline(StringStream, Line))
        {
            size_t Start = Line.find_first_not_of(" \t\r\n");
            size_t End = Line.find_last_not_of(" \t\r\n");

            if (Start != std::string::npos && End != std::string::npos)
            {
                Lines.push_back(Line.substr(Start, End - Start + 1));
            }
        }
        return Lines;
    }

    static std::wstring ReadRegistryValue(HKEY KeyHandle, const std::wstring& ValueName)
    {
        DWORD Type = 0;
        DWORD DataSize = 0;

        if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, nullptr, &DataSize) != ERROR_SUCCESS)
        {
            return L"";
        }

        std::vector<wchar_t> Buffer(DataSize / sizeof(wchar_t) + 1);

        if (RegQueryValueExW(KeyHandle, ValueName.c_str(), nullptr, &Type, reinterpret_cast<LPBYTE>(Buffer.data()), &DataSize) == ERROR_SUCCESS)
        {
            return std::wstring(Buffer.data());
        }

        return L"";
    }

    static std::string Trim(const std::string& Str)
    {
        auto IterStart = Str.begin();

        while (IterStart != Str.end() && (unsigned char)*IterStart <= ' ')
        {
            ++IterStart;
        }

        auto IterEnd = Str.end();

        while (IterEnd != IterStart && (unsigned char)*(IterEnd - 1) <= ' ')
        {
            --IterEnd;
        }

        return std::string(IterStart, IterEnd);
    }

    static std::string SafeAscii(const uint8_t* Ptr, size_t Count)
    {
        std::string Output;
        Output.reserve(Count);

        for (size_t Index = 0; Index < Count; ++Index)
        {
            unsigned char CharVal = Ptr[Index];

            if (CharVal == 0)
            {
                continue;
            }

            if (CharVal < 32 || CharVal > 126)
            {
                continue;
            }

            Output.push_back((char)CharVal);
        }

        return Trim(Output);
    }

    static bool ReadFileBytes(const std::wstring& Path, std::string& OutBuffer)
    {
        HANDLE FileHandle = CreateFileW(Path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (FileHandle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        std::vector<char> BufferVector;
        BufferVector.reserve(4096);
        char TempBuffer[4096];
        DWORD BytesRead = 0;
        BOOL Success = FALSE;

        for (;;)
        {
            Success = ReadFile(FileHandle, TempBuffer, sizeof(TempBuffer), &BytesRead, nullptr);
            if (!Success)
            {
                CloseHandle(FileHandle);
                return false;
            }
            if (BytesRead == 0)
            {
                break;
            }
            BufferVector.insert(BufferVector.end(), TempBuffer, TempBuffer + BytesRead);
        }

        CloseHandle(FileHandle);
        OutBuffer.assign(BufferVector.begin(), BufferVector.end());
        return true;
    }

    static std::string ExtractGUIDHexUpper(const std::string& Bytes)
    {
        std::string HexStr;
        HexStr.reserve(64);

        for (unsigned char CharVal : Bytes)
        {
            if (std::isxdigit(CharVal))
            {
                HexStr.push_back(static_cast<char>(std::toupper(CharVal)));
            }
        }

        if (HexStr.size() >= 32)
        {
            return HexStr.substr(0, 32);
        }

        return std::string();
    }

    static std::string Hex32ToGUID(const std::string& Hex32Upper)
    {
        return Hex32Upper.substr(0, 8) + "-" +
            Hex32Upper.substr(8, 4) + "-" +
            Hex32Upper.substr(12, 4) + "-" +
            Hex32Upper.substr(16, 4) + "-" +
            Hex32Upper.substr(20, 12);
    }

    static std::string BytesToHex(const BYTE* Data, DWORD Size, DWORD BytesPerLine = 16, size_t MaxLen = 0)
    {
        std::ostringstream StringStream;
        size_t Limit = Size;

        if (MaxLen)
        {
            size_t Limit = min((size_t)Size, MaxLen);
        }

        for (size_t Index = 0; Index < Limit; Index++)
        {
            StringStream << std::hex << std::setw(2) << std::setfill('0') << (int)Data[Index] << " ";

            if ((Index + 1) % BytesPerLine == 0)
            {
                StringStream << "\n";
            }
        }

        return StringStream.str();
    }
};