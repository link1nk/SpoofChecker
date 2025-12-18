#include "pch.h"

int main(void)
{
	DISK::QueryAllInfo();
	TPM::ShowTPMInfo();
	UsnJrnl::QueryAllInfo();
	MAC::QueryAllInfo();

	system("pause");
}