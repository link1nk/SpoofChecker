#pragma once

#include "pch.h"

class TPM
{
private:
	static std::string BytesToString(const std::vector<BYTE>& bytes);
	static std::vector<BYTE> GetEK();
	static std::string GetKeyHash(const std::vector<BYTE>& input, ALG_ID algo);

public:
	static void ShowTPMInfo();
};