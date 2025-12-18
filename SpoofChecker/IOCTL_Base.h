#pragma once

#include "pch.h"

class IOCTL_Base
{
private:
	ULONG m_Code;
	HANDLE m_Device;
	std::string m_Name;
	std::string m_ID;

public:
	IOCTL_Base() = default;
	IOCTL_Base(HANDLE Device, ULONG Code, std::string Name) :
		m_Device(Device), m_Code(Code), m_Name(Name)
	{
	}

	void SetIOCTL(HANDLE Device, ULONG Code, std::string Name)
	{
		m_Device = Device;
		m_Code = Code;
		m_Name = Name;
	}

	bool IOCTL(PVOID InputBuffer, DWORD InputLength, PVOID OutputBuffer, DWORD OutputLength);
	void PrintSerial(std::string Serial, std::string ID = "");
	void PrintGUID(GUID guid, std::string ID = "");
	void PrintHex(const UCHAR* data, USHORT len, std::string label, std::string ID = "");
	void PrintMAC(BYTE* addr, ULONG len, std::string label, std::string ID = "");
	void PrintJournalID(DWORDLONG UsnJournalID, std::string label, std::string ID = "");
	void PrintError(ULONG ErrorCode);
	void SetId(std::string ID);
};