#pragma once

#include "pch.h"

class Registry
{
private:
	static std::string ReadRegistryBinary(HKEY hKeyRoot, const std::wstring& subKey, const std::wstring& valueName);
	static std::string ReadRegistryStringA(HKEY hKeyRoot, const std::wstring& subKey, const std::wstring& valueName);
	static DWORD ReadRegistryDword(HKEY hKeyRoot, const std::wstring& subKey, const std::wstring& valueName);
	static DWORD64 ReadRegistryQword(HKEY hKeyRoot, const std::wstring& subKey, const std::wstring& valueName);
	static std::vector<std::wstring> ReadRegistryMultiSzW(HKEY hKeyRoot, const std::wstring& subKey, const std::wstring& valueName);
	static std::vector<std::string> ReadRegistryMultiSzA(HKEY hRoot, const std::wstring& subKey, const std::wstring& valueName);

	static std::string UnixTimestampToDate(DWORD timestamp);
	static void EnumerateScsiDevices(const std::wstring& baseKey);
	static void PrintHexString(const std::string& name, const std::string& hexString, size_t perLine = 32);
	static void EnumPnPTwoLevel(const std::wstring& baseKey, const wchar_t* title);
	static void PrintMaybeName(HKEY root, const std::wstring& path);
	static std::string BytesToHex(const BYTE* data, DWORD size, size_t maxLen = 0);
	static RawSMBIOSData* GetSMBIOS();

	static void EnumerateMonitors();
	static void EnumerateGraphicsDrivers();
	static void PrintMotherboardUUIDs();
	static void PrintNvidiaUUIDs();
	static void PrintVolumes();
	static void PrintExtraHardware();
	static void PrintUEFI_ESRT();
	static void PrintVideoDeviceMap();
	static void PrintVideoRegistry();
	static void PrintNTCurrentVersion();
	static void PrintWindowsUpdate();
	static void PrintWAT();
	static void PrintBIOSInfo();
	static void PrintScsiDevices();
	static void PrintNICsFromRegistry();
	static void PrintCryptographyInfo();
	static void PrintHwProfileInfo();
	static void PrintSystemInformation();
	static void PrintInternetExplorerMigration();
	static void PrintSQMClientInfo();
	static void PrintOneSettingsInfo();
	static void PrintDiagTrackInfo();
	static void PrintSoftwareProtectionPlatform();
	static void PrintDiskInfoFromStorage();
	static void PrintPCIDevices();
	static void PrintACPIDevices();
	static void PrintHardwareProfiles();
	static void PrintUSBDevices();
	static void PrintHIDDevices();
	static void PrintBluetoothDevices();
	static void PrintUserProfileList();
	static void PrintTPMEndorsementInfo();
	static void PrintSMBIOS();

public:
	static void QueryAllInfo();
};