#include "pch.h"
#include "lec-04-StringLib.h"

void PrintString(const char* s)
{
    printf("%s\n", s);
}

void PrintString(const wchar_t* s)
{
    wprintf(L"%s\n", s);
}
