#include "DigitalClock.h"
#include "segment.h"

HWND hwnd;			// main window descriptor
HMENU hMenu;		// main menu descriptor
HWND hwndSegment[6];	// 7segmnet element descriptors (for 6 digits of time)
//HWND segHwnd;
int countSegElem = sizeof(hwndSegment) / sizeof(HWND);

HINSTANCE hInst;

//BOOL isCyclicColor = false;
int currentMode;

bool isFullScreenMode = false;
bool showDate = true;
bool showWeekDay = true;

int colorInc[7][3] = {
	{0,0,0},	// not used
	{0,1,0},	// R->RG
	{-1,0,0},	// RG->G
	{0,0,1},	// G->GB
	{0,-1,0},	// GB->B
	{1,0,0},	// B->RB
	{0,0,-1},	// RB->R
};

// day of week
const TCHAR* dow[] = { TEXT("Sunday"), TEXT("Monday"), TEXT("Tuesday"),
		TEXT("Wednesday"), TEXT("Thursday"), TEXT("Friday"), TEXT("Saturday") };

// array of custom colors
static COLORREF arrCustClr[16] = { RGB(255,0,0), RGB(0,255,0), RGB(0,0,255) };

WORD cx, cy; // Client area size



int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow)
{
	TCHAR szClassName[] = TEXT("myApp");
	TCHAR szWinCaption[] = TEXT("7-segment digital clock");

	hInst = hInstance;

	CustomRegister();

	// use C++ style
	WNDCLASSEX wndclass{};
	//ZeroMemory(&wndclass, sizeof(wndclass)); / C-style

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szClassName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


	if (!RegisterClassEx(&wndclass)) return GetLastError();

	hwnd = CreateWindow(
		szClassName, // window class name
		szWinCaption, // window caption
		WS_OVERLAPPEDWINDOW, // window style
		CW_USEDEFAULT, CW_USEDEFAULT, // initial x y position
		CW_USEDEFAULT, CW_USEDEFAULT, // initial x y size
		NULL,			// parent window handle
		NULL,			// window menu handle
		hInstance,		// program instance handle
		NULL
	); // creation parameters

	if (!hwnd) return EXIT_FAILURE;

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	BOOL bRet;
	while (bRet=GetMessage(&msg, NULL, 0, 0) > 0)  // Read from message queue and fill msg structure
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}

		//TranslateMessage(&msg);		// Convert keybord messages to symbol 
		DispatchMessage(&msg);			// Call call-back window proc
	}

	CustomUnregister();
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static SYSTEMTIME st{};
	PAINTSTRUCT ps;
	HDC hdc;
	
	DWORD commandID;
	
	int space;					// distance between  group
	int totalWidth;				// total width of 6 segment elements with space

	RECT rectColor = { 0,0,100,30 };	// color code position
	RECT rectText{};					// text position
	
	TCHAR colorInfo[64]{};
	TCHAR text[256];
	TCHAR date[12];

	static HFONT hFontOriginal, hFont1;
	SIZE size, sizeText;
	int strCount;

	switch (iMsg)
	{
	case WM_NCCREATE:
		// Create additional fonts
		hFont1 = CreateFont(100, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, NULL);
		return true;

	case WM_CREATE:  // without break! goto directly WM_TIMER to get init time
		// Create and set main menu
		hMenu = CreateMainMenu();
		SetMenu(hwnd, hMenu);
		CheckMenuItems();

		// Create 7-segment elements
		for (int i = 0; i < countSegElem; i++)
		{
			hwndSegment[i] = CreateWindow(SEGMENT_WC, NULL, WS_CHILD | WS_VISIBLE,
				0, 0, 0, 0, hwnd, NULL, hInst, NULL);
		}

		// Set init color 
		SendMessage(hwndSegment[0], SSM_CHANGECOLOR, RGB(255, 0, 0), NULL);
		//PostMessage(hwnd, WM_COMMAND, IDM_CYCLIC, NULL);

		// Set timer
		if (!SetTimer(hwnd, ID_TIMER, 1000, NULL)) return EXIT_FAILURE;
	case WM_TIMER:
		GetLocalTime(&st);
		DrawTime(&st);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		SetTextColor(hdc, GetSegmentColor()); // get current color of segment indicator
		SetBkColor(hdc, RGB(0, 0, 0));

		TextOut(hdc, 0, 0, colorInfo, wsprintf(colorInfo, TEXT("Color: %i"), GetSegmentColor()));

		// set big size font
		hFontOriginal = (HFONT)SelectObject(hdc, hFont1);

		if (showWeekDay)
		{
			// Draw a day of week
			lstrcpy(text, dow[st.wDayOfWeek]);
			GetTextExtentPoint(hdc, text, lstrlen(text), &sizeText);
			rectText.left = (cx - sizeText.cx) / 2;
			rectText.right = rectText.left + sizeText.cx;
			rectText.bottom = rectText.top + sizeText.cy;
			DrawText(hdc, text, -1, &rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
		if (showDate)
		{
			// fill string with current date and get string length  
			strCount = wsprintf(date, TEXT("%02d.%02d.%04d"), st.wDay, st.wMonth, st.wYear);
			// get string in pixels and draw date
			GetTextExtentPoint(hdc, date, strCount, &size);
			TextOut(hdc, (cx - size.cx) / 2, cy - size.cy, date, strCount);
		}

		EndPaint(hwnd, &ps);
		break;
	case WM_SIZE:
		// Get client size
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);
		SetSegmentWidth(cx / 7);

		space = wndWidth / 2;
		totalWidth = 6 * wndWidth + 2 * space;

		// Set 7-segmnet element position
		for (int i = 0; i < countSegElem; i++)
		{
			SetWindowPos(hwndSegment[i], NULL,
				(cx - totalWidth) / 2 + wndWidth * i + i / 2 * space, (cy - wndHeight) / 2,
				wndWidth, wndHeight, SWP_NOZORDER);
		}
		break;
	
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (isFullScreenMode) SetScreenMode(false);
			break;
		case VK_F11:
			SetScreenMode(!isFullScreenMode);
			break;
		}
	case WM_COMMAND:
		commandID = LOWORD(wParam); // get command ID
		switch (commandID)
		{
		case IDM_CHOOSECOLOR:
			ChooseDrawColor();
			break;
		case IDM_SHOWDATE:
			showDate = !showDate;
			CheckMenuItems(); 
			InvalidateRect(hwnd, NULL, true);
			break;
		case IDM_SHOWWEEKDAY:
			showWeekDay = !showWeekDay;
			CheckMenuItems();
			InvalidateRect(hwnd, NULL, true);
			break;
		case IDM_FULLSCREEN: // Set full-screen mode
			SetScreenMode(true);
			break;
		case  IDM_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0L);
			break;
		}
		break;

	case WM_NCDESTROY:
		if (hFont1 != NULL) DeleteObject(hFont1);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}

HMENU CreateMainMenu()
{
	HMENU hMenu = CreateMenu();

	HMENU hMenuPopup = CreateMenu();
	AppendMenu(hMenuPopup, MF_STRING, IDM_CHOOSECOLOR, TEXT("Color..."));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SHOWWEEKDAY, TEXT("Show weekday"));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SHOWDATE, TEXT("Show date"));
	
	AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenuPopup, MF_STRING, IDM_FULLSCREEN, TEXT("Full screen\tF11"));


	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuPopup, TEXT("&Settings"));
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, TEXT("E&xit"));

	
	return hMenu;
}

void ChooseDrawColor()
{
	CHOOSECOLOR cc;                

	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hwnd;
	cc.lpCustColors = arrCustClr;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult = GetSegmentColor();

	if (ChooseColor(&cc) == TRUE)
	{
		//hbrush = CreateSolidBrush(cc.rgbResult);
		SetColor(cc.rgbResult);
	}

}

void SetColor(COLORREF color)
{
	//isCyclicColor = false;
	PostMessage(hwndSegment[0], SSM_CHANGECOLOR, color, NULL);
	// Redraw window
	InvalidateRect(hwnd, NULL, false);

}

void CheckMenuItems()
{
	//MF_CHECKED=8 or MF_UNCHECKED=0
	
	CheckMenuItem(hMenu, IDM_SHOWWEEKDAY, showWeekDay << 3); 
	CheckMenuItem(hMenu, IDM_SHOWDATE, showDate << 3);
	CheckMenuItem(hMenu, IDM_FULLSCREEN, isFullScreenMode << 3);
}

void DrawTime(SYSTEMTIME* st)
{
	static SYSTEMTIME pst{ 99,99,99,99,99,99,99 };  // prev time value
	// Draw seconds
	SetSegmentData(hwndSegment[5], (*st).wSecond % 10);
	if (pst.wSecond / 10 != (*st).wSecond / 10)
	{
		SetSegmentData(hwndSegment[4], (*st).wSecond / 10);
		pst.wSecond = (*st).wSecond;

		//InvalidateRect(hwnd,&rectColor, TRUE);
	}
	// Draw minutes
	if (pst.wMinute != (*st).wMinute)
	{
		SetSegmentData(hwndSegment[3], (*st).wMinute % 10);
		SetSegmentData(hwndSegment[2], (*st).wMinute / 10);
		pst.wMinute = (*st).wMinute;
	}
	// Draw hours
	if (pst.wHour != (*st).wHour)
	{
		SetSegmentData(hwndSegment[1], (*st).wHour % 10);
		SetSegmentData(hwndSegment[0], (*st).wHour / 10);
		pst.wHour = (*st).wHour;

		// Redraw date (each hour)
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

void SetScreenMode(bool flag)
{
	DWORD dwStyle= GetWindowLong(hwnd, GWL_STYLE);
	isFullScreenMode = flag;
	if (!flag)  
	{
		SetMenu(hwnd, hMenu); // show menu
		SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW); // Change window style
		SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}
	else
	{
		SetMenu(hwnd, NULL);  // hide menu
		SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW); // Change window style
		SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
}

void SetCyclicColor()
{
	static int step = 0;
	static int  incR = 0, incG = 1, incB = 0;

	step += 1;
	if (step > 255)
	{
		step = 0;
		if (++currentMode > 6)
		{
			currentMode = 1;

		};
		incR = colorInc[currentMode][0];
		incG = colorInc[currentMode][1];
		incB = colorInc[currentMode][2];
	};

	COLORREF color = GetSegmentColor();

	int colorR = GetRValue(color);
	int colorG = GetGValue(color);
	int colorB = GetBValue(color);

	SendMessage(hwndSegment[0], SSM_CHANGECOLOR, RGB(colorR + incR, colorG + incG, colorB + incB), NULL);

}