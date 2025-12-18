#include "pch.h"

bool IOCTL_Base::IOCTL(PVOID InputBuffer, DWORD InputLength, PVOID OutputBuffer, DWORD OutputLength)
{
	DWORD BytesReturned = 0;

	if (!DeviceIoControl(m_Device, m_Code, InputBuffer, InputLength, OutputBuffer, OutputLength, &BytesReturned, nullptr))
	{
		PrintError(GetLastError());
		return false;
	}
	return true;
}

void IOCTL_Base::SetId(std::string ID)
{
	m_ID = ID;
}

void IOCTL_Base::PrintSerial(std::string Serial, std::string ID)
{
	if (ID.empty() && !m_ID.empty())
	{
		ID = m_ID;
	}

	if (ID.empty())
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << "] Serial: " << dye::green(Serial) << std::endl;
	}
	else
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << " -> " << dye::aqua(ID) << "] Serial: " << dye::green(Serial) << std::endl;
	}

	m_ID.clear();
}

void IOCTL_Base::PrintGUID(GUID guid, std::string ID)
{
	if (ID.empty() && !m_ID.empty())
	{
		ID = m_ID;
	}

	std::ostringstream oss;
	oss << std::uppercase << std::hex << std::setfill('0');

	oss << '{'
		<< std::setw(8) << guid.Data1 << '-'
		<< std::setw(4) << guid.Data2 << '-'
		<< std::setw(4) << guid.Data3 << '-'
		<< std::setw(2) << static_cast<int>(guid.Data4[0])
		<< std::setw(2) << static_cast<int>(guid.Data4[1]) << '-';

	for (int i = 2; i < 8; i++)
	{
		oss << std::setw(2) << static_cast<int>(guid.Data4[i]);
	}

	oss << '}';

	if (ID.empty())
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << "] GUID: " << dye::green(oss.str()) << std::endl;
	}
	else
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << " -> " << dye::aqua(ID) << "] Serial: " << dye::green(oss.str()) << std::endl;
	}

	m_ID.clear();
}

void IOCTL_Base::PrintHex(const UCHAR* data, USHORT len, std::string label, std::string ID)
{
	if (ID.empty() && !m_ID.empty())
	{
		ID = m_ID;
	}

	std::ostringstream oss;
	oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0');

	for (USHORT i = 0; i < len; ++i)
	{
		oss << static_cast<int>(data[i]);
	}

	if (ID.empty())
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << "] " << label << ": " << dye::green(oss.str()) << std::endl;
	}
	else
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << " -> " << dye::aqua(ID) << "] " << label << ": " << dye::green(oss.str()) << std::endl;
	}

	m_ID.clear();
}

void IOCTL_Base::PrintMAC(BYTE* addr, ULONG len, std::string label, std::string ID)
{
	if (ID.empty() && !m_ID.empty())
	{
		ID = m_ID;
	}

	std::ostringstream oss;
	for (ULONG i = 0; i < len; i++)
	{
		if (i > 0) oss << "-";
		oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)addr[i];
	}

	if (ID.empty())
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << "] " << label << ": " << dye::green(oss.str()) << std::endl;
	}
	else
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << " -> " << dye::aqua(ID) << "] " << label << ": " << dye::green(oss.str()) << std::endl;
	}

	m_ID.clear();
}

void IOCTL_Base::PrintJournalID(DWORDLONG UsnJournalID, std::string label, std::string ID)
{
	if (ID.empty() && !m_ID.empty())
	{
		ID = m_ID;
	}

	std::cout << "[" << dye::light_purple(m_Name.c_str()) << "] " << label << ": " << std::hex << std::uppercase << dye::green(UsnJournalID) << std::endl;

	m_ID.clear();
}

void IOCTL_Base::PrintError(ULONG ErrorCode)
{
	std::cout << std::dec;

	if (m_ID.empty())
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << "] " << dye::red("ERROR ") << ErrorCode;
	}
	else
	{
		std::cout << "[" << dye::light_purple(m_Name.c_str()) << " -> " << dye::aqua(m_ID) << "] " << dye::red("ERROR ") << ErrorCode;
	}

	switch (ErrorCode)
	{
	case ERROR_INVALID_FUNCTION:
		std::cout << " -> " << dye::light_red("ERROR_INVALID_FUNCTION") << std::endl;
		break;
	case ERROR_ACCESS_DENIED:
		std::cout << " -> " << dye::light_red("ERROR_ACCESS_DENIED") << std::endl;
		break;
	case ERROR_INVALID_HANDLE:
		std::cout << " -> " << dye::light_red("ERROR_INVALID_HANDLE") << std::endl;
		break;
	case ERROR_NOT_SUPPORTED:
		std::cout << " -> " << dye::light_red("ERROR_NOT_SUPPORTED") << std::endl;
		break;
	case ERROR_DEV_NOT_EXIST:
		std::cout << " -> " << dye::light_red("ERROR_DEV_NOT_EXIST") << std::endl;
		break;
	case ERROR_NO_SUCH_DEVICE:
		std::cout << " -> " << dye::light_red("ERROR_NO_SUCH_DEVICE") << std::endl;
		break;
	case ERROR_IO_DEVICE:
		std::cout << " -> " << dye::light_red("ERROR_IO_DEVICE") << std::endl;
		break;
	case ERROR_JOURNAL_NOT_ACTIVE:
		std::cout << " -> " << dye::light_red("ERROR_JOURNAL_NOT_ACTIVE") << std::endl;
		break;
	default:
		std::cout << std::endl;

	}

	m_ID.clear();
}