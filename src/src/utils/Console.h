#pragma once

#include <windows.h> 
#include <iostream>

extern HANDLE hConsole;

#define skCrypt(x) x

namespace ConsoleColors
{
	void Error();
	void Success();
	void Purple();
	void Magenta();
	void Warning();
	void Reset();
}
