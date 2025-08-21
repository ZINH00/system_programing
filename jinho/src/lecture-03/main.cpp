#include "pch.h"

int main()
{
	HANDLE hFile = CreateFile(TEXT("d:/hello.txt"), GENERIC_READ_, OPEN_EXISTING_, 0);
	if (NULL == hFile)
		return -1;

	char szBuffer[10000 + 1];
	DWORD dwReadSize = 0;
	ReadFile(hFile, szBuffer, 10000, &dwReadSize);

	szBuffer[dwReadSize] = 0;
	printf("%s\n", szBuffer);

	CloseFile(hFile);
	return 0;
}
