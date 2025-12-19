#include "pch.h"

int main(void)
{
	DISK::QueryAllInfo();
	TPM::ShowTPMInfo();
	HID::QueryAllInfo();
	UsnJrnl::QueryAllInfo();
	MAC::QueryAllInfo();
	ARP::QueryAllInfo();
	Volumes::QueryAllInfo();
	Files::QueryAllInfo();
	SMBIOS::QueryAllInfo();
	Registry::QueryAllInfo();

	system("pause");
}