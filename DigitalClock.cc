#include <windows.h>
#include <math.h>

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

#define PI 3.141592653589793238463
#define RAD(a) a*PI/180.0

#define GETR(rgb) rgb>>0 & 0xFF
#define GETG(rgb) rgb>>8 & 0xFF
#define GETB(rgb) rgb>>16 & 0xFF

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Draw7Seg(HDC hdc, short dig, int group);
void DrawSegment(HDC hdc, int index, RECT rect);
int TransformPoints(POINT* pts, POINT* ptd, int dx, int dy, int rotate = 0);
void FillSegmentData();
HMENU CreateMainMenu();
void SetPenColor(COLORREF color);
void CheckColorMenuItem(DWORD item);
void SetCyclicColor();
int GetCyclicColorMode();

HWND hwnd;		// main window descriptor
HMENU hMenu;	// main menu descriptor

BOOL isCyclicColor = false;
int currentMode;

int colorInc[7][3] = {
	{0,0,0},	// not used
	{0,1,0},	// R->RG
	{-1,0,0},	// RG->G
	{0,0,1},	// G->GB
	{0,-1,0},	// GB->B
	{1,0,0},	// B->RB
	{0,0,-1},	// RB->R
};

// Segment data
int xInitSize = 120, xSize, ySize;	// default width and height of segments element;
float degree = 5;					// skew degree angle of segment (0-6 optimal)
int thickness, hth;					// thickness and it half of segment
int space, gspace;					// distance between items in group and group itself
int groupSpace;						// Step of group
int totalWidth;						// total width

const int sizePT = 6;   // number of segments points 
POINT pt[7][sizePT];

WORD cx, cy; // Client area size

// Colors
COLORREF lightPen;
COLORREF darkPen;
COLORREF BLACK = RGB(0, 0, 0);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow)
{
	TCHAR szClassName[] = TEXT("myApp");
	TCHAR szWinCaption[] = TEXT("7-segment digital clock");

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
	while (GetMessage(&msg, NULL, 0, 0) > 0)  // Read from message queue and fill msg structure
	{
		//TranslateMessage(&msg);		// Convert keybord messages to symbol 
		DispatchMessage(&msg);			// Call call-back window proc
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL isFullScreenMode = false;

	static SYSTEMTIME st{};
	PAINTSTRUCT ps;
	HDC hdc;
	DWORD dwStyle;
	DWORD commandID;
	TCHAR colorInfo[64]{};

	switch (iMsg)
	{
	case WM_CREATE:  // without break! goto directly WM_TIMER to get init time
		// Create and set main menu
		hMenu = CreateMainMenu();
		SetMenu(hwnd, hMenu);
		// Set init data
		FillSegmentData();
		// Set color
		PostMessage(hwnd, WM_COMMAND, IDM_SETREDCOLOR, NULL);
		PostMessage(hwnd, WM_COMMAND, IDM_CYCLIC, NULL);
		// Set timer
		if (!SetTimer(hwnd, ID_TIMER, 1000, NULL)) return EXIT_FAILURE;
	case WM_TIMER:
		if (isCyclicColor) SetCyclicColor();
		GetLocalTime(&st);
		InvalidateRect(hwnd, nullptr, TRUE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		Draw7Seg(hdc, st.wHour, 0);		// Draw hours
		Draw7Seg(hdc, st.wMinute, 1);	// Draw minutes
		Draw7Seg(hdc, st.wSecond, 2);	// Draw seconds
		TextOut(hdc, 0, 0, colorInfo,
			wsprintf(colorInfo, TEXT("Color: %i"), lightPen));
		EndPaint(hwnd, &ps);
		break;
	case WM_SIZE:
		// Get client size
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);
		xInitSize = cx / 9;
		FillSegmentData();
		break;
	case WM_LBUTTONDBLCLK:
		isFullScreenMode = ~isFullScreenMode;
		dwStyle = GetWindowLong(hwnd, GWL_STYLE);
		if (isFullScreenMode) // Set full-screen mode
		{
			SetMenu(hwnd, NULL);  // hide menu
			SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
		else // Set windowed mode
		{
			SetMenu(hwnd, hMenu); // show menu
			SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
			SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		break;
	case WM_COMMAND:
		commandID = LOWORD(wParam); // get command ID
		switch (commandID)
		{
		case IDM_SETREDCOLOR:
			SetPenColor(RGB(255, 0, 0));
			break;
		case IDM_SETYELLOWCOLOR:
			SetPenColor(RGB(255, 255, 0));
			break;
		case IDM_SETGREENCOLOR:
			SetPenColor(RGB(0, 255, 0));
			break;
		case IDM_SETTURQUOISECOLOR:
			SetPenColor(RGB(0, 255, 255));
			break;
		case IDM_SETBLUECOLOR:
			SetPenColor(RGB(0, 0, 255));
			break;
		case IDM_SETPURPLECOLOR:
			SetPenColor(RGB(255, 0, 255));
			break;
		case IDM_CYCLIC:
			isCyclicColor = true; // Enable cyclic color mode;
			currentMode = 1;
			break;
		case  IDM_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0L);
			break;
		}
		CheckColorMenuItem(commandID);
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
	AppendMenu(hMenuPopup, MF_STRING, IDM_SETREDCOLOR, TEXT("Red"));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SETYELLOWCOLOR, TEXT("Yellow"));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SETGREENCOLOR, TEXT("Green"));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SETTURQUOISECOLOR, TEXT("Turquoise"));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SETBLUECOLOR, TEXT("Blue"));
	AppendMenu(hMenuPopup, MF_STRING, IDM_SETPURPLECOLOR, TEXT("Purple"));
	AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenuPopup, MF_STRING, IDM_CYCLIC, TEXT("Ñyclic"));


	AppendMenu(hMenu, MF_POPUP, (UINT)hMenuPopup, TEXT("&Color"));
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, TEXT("E&xit"));



	return hMenu;
}

void FillSegmentData()
{
	thickness = xInitSize / 5;					// thickness of segment
	hth = thickness / 2;						// half of thickness
	space = thickness;							// distance between items in group
	gspace = 3 * space;							// distance between  group
	ySize = 2 * xInitSize - thickness;			// width of segment
	int centerY = ySize / 2;					// vertical center
	int shiftX = ySize * tan(RAD(degree));		// skew (horisontal extension)
	xSize = xInitSize + shiftX;					// total width of element
	groupSpace = 2 * xSize + space + gspace;	// Step of group

	totalWidth = 3 * (2 * xSize + space) + 2 * gspace; // total width of 3 group + 2 space between groups 

	// points for origin sample segment 
	POINT segmentSample[] = {				//	Segment origin sample (* is point)
	{ 0, 0 },								//		 *---------*
	{ hth,-hth },							//		*			*
	{ xInitSize - 3 * hth - 1,-hth },		//		 *---------*
	{ xInitSize - thickness - 1,0 },
	{ xInitSize - 3 * hth - 1,hth },
	{ hth,hth },
	};

	// Segment samples
	TransformPoints(segmentSample, pt[0], hth + shiftX, hth);									// A
	TransformPoints(segmentSample, pt[1], xSize - hth - 1, hth, 1);								// B	(no changed by a skew)
	TransformPoints(segmentSample, pt[2], xSize - hth - 1 - shiftX / 2, centerY - 1, 1);		// C
	TransformPoints(segmentSample, pt[3], hth, ySize - hth - 2);								// D	(no changed by a skew)
	TransformPoints(segmentSample, pt[4], hth + shiftX / 2, centerY - 1, 1);					// E
	TransformPoints(segmentSample, pt[5], hth + shiftX, hth, 1);								// F
	TransformPoints(segmentSample, pt[6], shiftX / 2 + hth, centerY - 1);						// G
}

void Draw7Seg(HDC hdc, short dig, int group)
{
	//  Segment structure

	static int data[10][7] = {
		//	  A	B C D E F G
			{ 1,1,1,1,1,1,0 },	//0			//		 ---A---
			{ 0,1,1,0,0,0,0 },	//1			//		|		|
			{ 1,1,0,1,1,0,1 },	//2			//		F		B
			{ 1,1,1,1,0,0,1 },	//3			//		|		|
			{ 0,1,1,0,0,1,1 },	//4			//		 ---G---
			{ 1,0,1,1,0,1,1 },	//5			//		|		|
			{ 1,0,1,1,1,1,1 },	//6			//		E		C
			{ 1,1,1,0,0,0,0 },	//7			//		|		|
			{ 1,1,1,1,1,1,1 },	//8			//		 ---D---
			{ 1,1,1,1,0,1,1 },	//9			//
	};

	int xPos = (cx - totalWidth) / 2, yPos = (cy - ySize) / 2; // Position of first group

	RECT rect1{ 0,0,xSize,ySize };
	OffsetRect(&rect1, xPos + groupSpace * group, yPos);
	RECT rect2 = rect1;
	OffsetRect(&rect2, xInitSize + space, 0);

	int dig1 = dig / 10, dig2 = dig % 10;

	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, darkPen);

	// Just for visual check real rect area
	//Rectangle(hdc, rect1.left, rect1.top, rect1.right, rect1.bottom);

	for (int i = 0; i < 7; i++)
	{
		// First digit
		SetDCBrushColor(hdc, lightPen);

		if (data[dig1][i] == 0)
		{
			SetDCBrushColor(hdc, BLACK);
		}
		DrawSegment(hdc, i, rect1);

		// Second digit
		SetDCBrushColor(hdc, lightPen);

		if (data[dig2][i] == 0)
		{
			SetDCBrushColor(hdc, BLACK);
		}
		DrawSegment(hdc, i, rect2);
	}
}


void DrawSegment(HDC hdc, int index, RECT rect)
{
	static POINT ptDraw[sizePT]{};

	// set segment points to rect position 
	for (int i = 0; i < sizePT; i++)
	{
		ptDraw[i].x = pt[index][i].x + rect.left;
		ptDraw[i].y = pt[index][i].y + rect.top;
	}

	Polygon(hdc, ptDraw, sizePT);
}

int TransformPoints(POINT* pts, POINT* ptd, int dx, int dy, int rotate)
{
	int grdAngle = 90 + degree;
	double angle = RAD(grdAngle);
	for (int i = 0; i < sizePT; i++)
	{
		/*ptd[i].x = rotate == 0 ? pts[i].x + dx : pts[i].y + dx + pts[0].x - pts[0].y;
		ptd[i].y = rotate == 0 ? pts[i].y + dy : pts[i].x + dy - pts[0].x + pts[0].y;*/

		ptd[i].x = rotate == 0 ? pts[i].x + dx : pts[i].x * cos(angle) - pts[i].y * sin(angle) + dx;
		ptd[i].y = rotate == 0 ? pts[i].y + dy : pts[i].x * sin(angle) + pts[i].y * cos(angle) + dy;
	}
	return sizePT;
}

void SetPenColor(COLORREF color)
{
	int tempColor;
	lightPen = color;
	darkPen = 0;
	//isCyclicColor = false; // Disable cyclic color mode;

	// Set DARKPEN as LIGHTPEN/8
	for (int i = 0; i < 3; i++)
	{
		tempColor = (color >> (8 * i) & 0xFF) >> 3;
		darkPen |= (tempColor << (8 * i));
	}

}

void CheckColorMenuItem(DWORD item)
{
	if (item > FIRSTCOLOR && item <= IDM_CYCLIC)  // Check if its Menu Color Command 
	{
		for (DWORD i = FIRSTCOLOR + 1; i <= IDM_CYCLIC; i++)
		{
			if (item == i) CheckMenuItem(hMenu, item, MF_CHECKED); else CheckMenuItem(hMenu, i, MF_UNCHECKED);
		}
	}
}


void GetCurrentMode()
{

}

int GetCyclicColorMode()
{
	int colorModeIndex = 0;

	short colorR = GETR(lightPen) == 255 ? 1 : 0;
	short colorG = GETG(lightPen) == 255 ? 1 : 0;
	short colorB = GETB(lightPen) == 255 ? 1 : 0;

	colorModeIndex |= (colorR << 2) | (colorG << 1) | (colorB << 0);
	return colorModeIndex;
}

void SetCyclicColor()
{
	static int step=0;
	static int  incR=0, incG=1, incB=0;

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

	int colorR = GETR(lightPen);
	int colorG = GETG(lightPen);
	int colorB = GETB(lightPen);
	SetPenColor(COLORREF RGB(colorR+incR, colorG+incG, colorB+incB));
}