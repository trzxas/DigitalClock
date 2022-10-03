#include <windows.h>
#include <math.h>

#define ID_TIMER	1

#define PI 3.141592653589793238463
#define RAD(a) a*PI/180.0

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Draw7Seg(HDC hdc, short dig, int group);
void DrawSegment(HDC hdc, int index, RECT rect);
int TransformPoints(POINT* pts, POINT* ptd, int dx, int dy, int rotate = 0);
void FillSegmentData();

HWND hwnd;

// Segment data
int xInitSize = 100, xSize, ySize;	// default width and height of segments element;
float degree = 0;					// angle of segment
int thickness, hth;					// thickness and it half of segment
int space, gspace;					// distance between items in group and group itself
int groupSpace;						// Step of group
int totalWidth;						// total width

const int sizePT = 6;   // number of segments points 
POINT pt[7][sizePT];

WORD cx, cy; // Client area size

// Colors
COLORREF GREEN = RGB(64, 192, 64);
COLORREF DARKGREEN = RGB(0, 32, 0);
COLORREF BLACK = RGB(0, 0, 0);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow)
{
	TCHAR szClassName[] = TEXT("myApp");
	TCHAR szWinCaption[] = TEXT("7-segment digital clock");

	// use C++ style
	WNDCLASSEX wndclass{};
	//ZeroMemory(&wndclass, sizeof(wndclass)); / C-style

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
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
		TranslateMessage(&msg);		// Convert keybord messages to symbol 
		DispatchMessage(&msg);		// Call call-back window proc
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static SYSTEMTIME st{};
	PAINTSTRUCT ps;
	HDC hdc;
	switch (iMsg)
	{
	case WM_CREATE:  // without break! goto directly WM_TIMER to get init time
		FillSegmentData();
		if (!SetTimer(hwnd, ID_TIMER, 1000, NULL)) return EXIT_FAILURE;
	case WM_TIMER:
		GetLocalTime(&st);
		InvalidateRect(hwnd, nullptr, TRUE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		Draw7Seg(hdc, st.wHour, 0);		// Draw hours
		Draw7Seg(hdc, st.wMinute, 1);	// Draw minutes
		Draw7Seg(hdc, st.wSecond, 2);	// Draw seconds
		EndPaint(hwnd, &ps);
		break;
	case WM_SIZE:
		// Get client size 
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}

void FillSegmentData()
{
	int shiftX = xInitSize * tan(RAD(degree)); // skew
	xSize = xInitSize + shiftX;	// horisontal extension
	thickness = xInitSize / 5;	// thickness of segment
	hth = thickness / 2;		// half of thickness
	space = thickness;			// distance between items in group
	gspace = 4 * space;			// distance between  group
	groupSpace = 2 * xSize + space + gspace;// Step of group
	ySize = 2 * xInitSize - thickness;			// width of segment
	int centerY = ySize / 2;// vertical center

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
	TransformPoints(segmentSample, pt[0], shiftX+hth, hth);									// A
	TransformPoints(segmentSample, pt[1], shiftX / 2 + xSize - hth - 1, hth, 1);			// B
	TransformPoints(segmentSample, pt[2], xSize - hth - 1- shiftX / 2, centerY - 1, 1);		// C
	TransformPoints(segmentSample, pt[3], hth, ySize - hth - 2);							// D
	TransformPoints(segmentSample, pt[4], shiftX/2+hth, centerY - 1, 1);					// E
	TransformPoints(segmentSample, pt[5], shiftX+hth, hth, 1);								// F
	TransformPoints(segmentSample, pt[6], shiftX / 2 + hth, centerY - 1);					// G
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
	OffsetRect(&rect2, xSize + space, 0);

	int dig1 = dig / 10, dig2 = dig % 10;

	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));

	// Just for visual check real rect area
	//Rectangle(hdc, rect1.left, rect1.top, rect1.right, rect1.bottom);

	for (int i = 0; i < 7; i++)
	{
		// First digit
		SetDCBrushColor(hdc, GREEN);
		//SetDCPenColor(hdc, GREEN);
		if (data[dig1][i] == 0)
		{
			SetDCPenColor(hdc, DARKGREEN);
			SetDCBrushColor(hdc, BLACK);
		}
		DrawSegment(hdc, i, rect1);

		// Second digit
		SetDCBrushColor(hdc, GREEN);
		//SetDCPenColor(hdc, GREEN);
		if (data[dig2][i] == 0)
		{
			SetDCPenColor(hdc, DARKGREEN);
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
	double angle = (90 + degree) * PI / 180.0;
	for (int i = 0; i < sizePT; i++)
	{
		/*ptd[i].x = rotate == 0 ? pts[i].x + dx : pts[i].y + dx + pts[0].x - pts[0].y;
		ptd[i].y = rotate == 0 ? pts[i].y + dy : pts[i].x + dy - pts[0].x + pts[0].y;*/

		ptd[i].x = rotate == 0 ? pts[i].x + dx : pts[i].x * cos(angle) - pts[i].y * sin(angle) + dx;
		ptd[i].y = rotate == 0 ? pts[i].y + dy : pts[i].x * sin(angle) + pts[i].y * cos(angle) + dy;
	}
	return sizePT;
}
