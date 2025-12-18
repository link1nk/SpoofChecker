#pragma once

#include "pch.h"

class UsnJrnl
{
private:
    inline static IOCTL_Base JournalIOCTL;

    static void QueryUsnJournal(const std::wstring& volumePath);
    static void EnumerateUsnJournals();

public:
    static void QueryAllInfo();
};