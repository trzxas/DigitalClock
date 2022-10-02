#include <windows.h>
#define ID_TIMER	1


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Draw7Seg(HDC hdc, short dig, int group);
void DrawSegment(HDC hdc, int index, RECT rect);

HWND hwnd;


int thickness; // thickness of segment
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
	case WM_CREATE:
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



void  Draw7Seg(HDC hdc, short dig, int group)
{
	int xSize = 80;
	thickness = xSize / 5;
	int space = thickness; // distance between items in group
	int gspace = 4 * space;// distance between  group
	int groupSpace = 2 * xSize + space + gspace; // Step of group
	int ySize = 2 * xSize - thickness;

	int totalWidth = 3 * (2 * xSize + space) + 2 * gspace; // total width of 3 group + 2 space between groups 

	int xPos = (cx - totalWidth) / 2, yPos = (cy - ySize) / 2; // Position of first group

	RECT rect1{ 0,0,xSize,ySize };
	OffsetRect(&rect1, xPos + groupSpace * group, yPos);
	RECT rect2 = rect1;
	OffsetRect(&rect2, xSize + space, 0);

	int dig1 = dig / 10, dig2 = dig % 10;

	int data[10][7] = {
		{ 1,1,1,1,1,1,0 },
		{ 0,1,1,0,0,0,0 },
		{ 1,1,0,1,1,0,1 },
		{ 1,1,1,1,0,0,1 },
		{ 0,1,1,0,0,1,1 },
		{ 1,0,1,1,0,1,1 },
		{ 1,0,1,1,1,1,1 },
		{ 1,1,1,0,0,0,0 },
		{ 1,1,1,1,1,1,1 },
		{ 1,1,1,1,0,1,1 },
	};

	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));

	for (int i = 0; i < 7; i++)
	{
		// First digit
		SetDCBrushColor(hdc, GREEN);
		if (data[dig1][i] == 0)
		{
			SetDCPenColor(hdc, DARKGREEN);
			SetDCBrushColor(hdc, BLACK);
		}
		DrawSegment(hdc, i, rect1);

		// Second digit
		SetDCBrushColor(hdc, GREEN);
		if (data[dig2][i] == 0)
		{
			SetDCPenColor(hdc, DARKGREEN);
			SetDCBrushColor(hdc, BLACK);
		}
		DrawSegment(hdc, i, rect2);
	}



}

int TransformPoint(POINT* pts, POINT* ptd, int dx, int dy, int rotate = 0)
{
	int sizeP = 6;

	for (int i = 0; i < sizeP; i++)
	{
		ptd[i].x = rotate == 0 ? pts[i].x + dx : pts[i].y + dx + pts[0].x - pts[0].y;
		ptd[i].y = rotate == 0 ? pts[i].y + dy : pts[i].x + dy - pts[0].x + pts[0].y;
	}

	return sizeP;
}
void DrawSegment(HDC hdc, int index, RECT rect)
{
	int hth = thickness / 2; // half of thickness
	int middle = (rect.bottom + rect.top) / 2; // position of vertical center
	int sizeX = rect.right - rect.left, sizeY = rect.bottom - rect.top;
	int centerY = sizeY / 2;

	POINT pth[] = {
		{rect.left + hth, rect.top + hth},
		{rect.left + thickness, rect.top},
		{rect.right - thickness, rect.top},
		{rect.right - hth, rect.top + hth},
		{rect.right - thickness, rect.top + thickness},
		{rect.left + thickness, rect.top + thickness},
	};
	POINT pt[sizeof(pth) / sizeof(pth[0])]{};

	switch (index)
	{
	case 0: // A
		TransformPoint(pth, pt, 0, 0);
		break;
	case 1: // B
		TransformPoint(pth, pt, sizeX - thickness, 0, 1);
		break;
	case 2: // C
		TransformPoint(pth, pt, sizeX - thickness, centerY - thickness / 2, 1);
		break;
	case 3: // D
		TransformPoint(pth, pt, 0, sizeY - thickness);
		break;
	case 4: // E
		TransformPoint(pth, pt, 0, centerY - thickness / 2, 1);
		break;
	case 5: // F
		TransformPoint(pth, pt, 0, 0, 1);
		break;
	case 6: // G
		TransformPoint(pth, pt, 0, centerY - thickness / 2);
		break;
	default:
		return;
	}
	Polygon(hdc, pt, sizeof(pt) / sizeof(POINT));
}
