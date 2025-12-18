#include "pch.h"

int main(void)
{
	DISK::QueryAllInfo();
	TPM::ShowTPMInfo();
	UsnJrnl::QueryAllInfo();
	MAC::QueryAllInfo();
	ARP::QueryAllInfo();

	system("pause");
}