#pragma once

#include "pch.h"

class Files
{
	struct ParsedEntry
	{
		std::wstring hwid;
		std::wstring serial;
	};

private:
	static std::string ReadMachineGuidFromShadow(const std::wstring& shadowPath);
	static std::vector<std::wstring> GetShadowCopies(int maxCheck = 64);
	static std::vector<std::wstring> GetLogFiles();
	static std::vector<ParsedEntry> ParseLogFile(const std::wstring& path);
	static void CheckRecycleBin(const std::wstring& volumePath);
	static void ResolveSID(const std::wstring& sidStr);

	static void PrintShadowCopies();
	static void PrintSIDFromRecycleBin();
	static void PrintHwidSerialFromLogFiles(bool verbose = false);

public:
	static void QueryAllInfo();
};