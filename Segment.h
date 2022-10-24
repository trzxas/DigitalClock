#pragma once

//#include <tchar.h>
#include <windows.h>

#define _USE_MATH_DEFINES
#include <math.h>

//#ifndef CUSTOM_H
//#define CUSTOM_H



/* Window class */
#define SEGMENT_WC   TEXT("Segment")

#define SSM_CHANGECOLOR  (WM_USER + 100)

// extern variable
extern int  wndWidth, wndHeight;

/* Register/unregister the window class */
bool CustomRegister(void);
void CustomUnregister(void);

/* Other function descriptions */
void SetSegmentWidth(int);
void SetSegmentData(HWND hwnd, WORD data);
void SetSegmentColor(COLORREF color);
COLORREF GetSegmentColor();

//#endif  /* CUSTOM_H */