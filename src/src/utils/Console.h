#pragma once

#include <windows.h> 
#include <iostream>

extern HANDLE hConsole;

#define skCrypt(x) x


void set_console_error();
void set_console_success();
void set_console_purple();
void set_console_magenta();
void set_console_warning();
void reset_console();