#include "pch.h"

int main(void)
{
	DISK::QueryAllInfo();
	TPM::ShowTPMInfo();
	UsnJrnl::QueryAllInfo();
	MAC::QueryAllInfo();
	ARP::QueryAllInfo();
	Volumes::QueryAllInfo();

	system("pause");
}