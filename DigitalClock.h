#pragma once

#include <windows.h>

#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

#define ID_TIMER	1

#define IDM_EXIT			100
#define IDM_CHOOSECOLOR		101
#define IDM_SHOWDATE		102
#define IDM_SHOWWEEKDAY		103
#define IDM_FULLSCREEN		104


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HMENU CreateMainMenu();

void CheckMenuItems();
void SetCyclicColor();
void DrawTime(SYSTEMTIME* st);
void ChooseDrawColor();
void SetColor(COLORREF color);
void SetScreenMode(bool flag);

#endif    DIGITALCLOCK_H 