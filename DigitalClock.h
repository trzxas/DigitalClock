#pragma once

#include <windows.h>

#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

#define ID_TIMER	1
#define IDM_EXIT	100


#define FIRSTCOLOR 200
#define IDM_SETREDCOLOR FIRSTCOLOR+1
#define IDM_SETYELLOWCOLOR FIRSTCOLOR+2
#define IDM_SETGREENCOLOR FIRSTCOLOR+3
#define IDM_SETTURQUOISECOLOR FIRSTCOLOR+4
#define IDM_SETBLUECOLOR FIRSTCOLOR+5
#define IDM_SETPURPLECOLOR FIRSTCOLOR+6
#define IDM_CYCLIC FIRSTCOLOR+7

#define COLORMENUPOSITION(color) color-FIRSTCOLOR

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HMENU CreateMainMenu();

void CheckColorMenuItem(DWORD item);
void SetCyclicColor();
int GetCyclicColorMode();
void DrawTime(SYSTEMTIME* st);
void SetColor(COLORREF color);

#endif    DIGITALCLOCK_H 