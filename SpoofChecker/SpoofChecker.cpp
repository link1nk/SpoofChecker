#include "pch.h"

int main(void)
{
	DISK::QueryAllInfo();
	TPM::ShowTPMInfo();
	UsnJrnl::QueryAllInfo();

	system("pause");
}