/* File custom.cc
 * (custom control implementation)
 */

#include "segment.h"

 /* Math macros*/
#define RAD(a) a*M_PI/180.0

 //  Segment structure
static int segmentsData[12][7] = {
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
		{ 1,1,1,0,1,1,0 },	//10		//
		{ 0,0,0,1,1,1,1 },	//11		//
};

static const int sizePT = 6;   // number of segments points 
static POINT pt[7][sizePT];

// Segment data
int wndWidth, wndHeight;	// width and height element window



// Colors
COLORREF lightPen;
static COLORREF darkPen;
static COLORREF BLACK = RGB(0, 0, 0);

typedef struct CustomClassData_tag {
	COLORREF lightPen;
	COLORREF darkPen;
} CustomClassData;

typedef struct CustomWndData_tag {
	byte data;
} CustomWndData;

static CustomClassData* extClassData;


// static  proc from winCallBack
static void CustomPaint(HWND hwnd);
static void Draw7Seg(HDC hdc, byte dig);
static void DrawSegment(HDC hdc, int index);
static int  TransformPoints(POINT* pts, POINT* ptd, int dx, int dy, int rotate = 0);

// function declaration
void SetSegmentColor(COLORREF color);

static LRESULT CALLBACK CustomProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CustomWndData* extWndData;

	COLORREF newSegmentColor;
	switch (uMsg) {
	case WM_NCCREATE:
	{
		if (extClassData == NULL) return FALSE;
		SetClassLongPtr(hwnd, 0, (LONG_PTR)extClassData);

		extWndData = (CustomWndData*)malloc(sizeof(CustomWndData));
		if (extWndData == NULL) return FALSE;;
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)extWndData);

		// Just in case set default color 
		SendMessage(hwnd, SSM_CHANGECOLOR, RGB(255, 255, 255), NULL);

		// Set init data
		SetSegmentWidth(40);

		return TRUE;
	}
	case WM_PAINT:
		CustomPaint(hwnd);
		return 0;

	case SSM_CHANGECOLOR:
		newSegmentColor = (COLORREF)wParam;
		SetSegmentColor(newSegmentColor);
		return 0;

	case WM_NCDESTROY:
		extWndData = (CustomWndData*)GetWindowLongPtr(hwnd, 0);
		if (extWndData != NULL) { // <-- "If" required as we get here even when WM_NCCREATE fails.
			// ... free any resources stored in the data structure
			free(extWndData);
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void CustomPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	GetClientRect(hwnd, &rect);

	// Get color from class extraBytes // must be removed from paint!
	CustomClassData* ptrc = (CustomClassData*)GetClassLongPtr(hwnd, 0);
	lightPen = (ptrc->lightPen);
	darkPen = (ptrc->darkPen);

	// Get number from window extra Bytes
	CustomWndData* ptrw = (CustomWndData*)GetWindowLongPtr(hwnd, 0);
	byte digit = (ptrw->data) % 16;

	hdc = BeginPaint(hwnd, &ps);
	//MoveTo(hdc ,0,0);
	//LineTo(hdc, rect.right, 0);
	//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	Draw7Seg(hdc, digit);			// Draw segments
	EndPaint(hwnd, &ps);
}


bool CustomRegister(void)
{
	WNDCLASS wc = { 0 };

	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = CustomProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbClsExtra = sizeof(CustomClassData*);
	wc.cbWndExtra = sizeof(CustomWndData*);
	wc.lpszClassName = SEGMENT_WC;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wc);

	extClassData = (CustomClassData*)malloc(sizeof(CustomClassData));
	if (extClassData == NULL) return FALSE;

	return TRUE;
}

void CustomUnregister(void)
{
	UnregisterClass(SEGMENT_WC, NULL);

	if (extClassData != NULL) free(extClassData);
}

void SetSegmentWidth(int wndX)
{
	static float degree = 5;								// skew degree angle of segment (0-6 optimal)

	wndWidth = wndX;										// total window height (incl padding, extX)
	int primeCoef = wndWidth / 5;

	int padding = primeCoef/4;								// window padding
	int segLength = wndWidth - 2 * padding - primeCoef;		// length of segment (not depend on skew)
	int segThickness = max(primeCoef, 2);					// thickness of segment
	int th2 = segThickness / 2;								// thickness/2 , just for convenience, it's use often
	int xSize = wndWidth - 2 * padding;						// width of element (w/o padding)
	int ySize = 2 * segLength + segThickness;				// height of element  (w/o padding)

	int extX = (int)(segLength * tan(RAD(degree)));			// skew (horisontal extension)
	int thickExt = (int)(th2 * tan(RAD(degree)));			// skew depend on thickness

	int horSegLength = segLength - extX;					// horisontal segment length depend on skew

	wndHeight = ySize + 2 * padding;						// total window height

	int top = padding + th2;								// top line
	int vertCenter = wndHeight / 2;							// vertical center line
	int bottom = wndHeight - padding - th2 - 1;				// bottom line
	int left = th2 + padding;								// left line

	// Vertical segment sample 
	POINT vertSegment[sizePT]{							//		  *
		{extX,0},										//		*   *
		{th2+extX-thickExt,th2},						//		|	|
		{th2,segLength - th2 },							//		|	|
		{0, segLength - 1},								//		*	*
		{-th2 ,segLength - th2 },						//		  *
		{-th2 + extX - thickExt,th2},
	};

	// Horizontal segment sample 		
	POINT horSegment[sizePT]{
		{0,0},											//		 *---------*	
		{th2,-th2},										//		*			*	
		{horSegLength - th2,-th2},						//		 *---------*	
		{horSegLength,0},
		{horSegLength - th2,th2},
		{th2,th2},
		};								
	
	// Samples for all (7) segment  
	TransformPoints(horSegment, pt[0], left + 2*extX, top);							// A
	TransformPoints(vertSegment, pt[1], left + horSegLength+extX, top, 0);			// B	
	TransformPoints(vertSegment, pt[2], left + horSegLength, vertCenter, 0);		// C
	TransformPoints(horSegment, pt[3], left, bottom);								// D	(no changed by a skew)
	TransformPoints(vertSegment, pt[4], left, vertCenter,0 );						// E
	TransformPoints(vertSegment, pt[5], left+extX, top, 0);							// F
	TransformPoints(horSegment, pt[6], left + extX, vertCenter);					// G


}



static void Draw7Seg(HDC hdc, byte dig)
{
	//RECT rect1{ 0,0,xSize,ySize };

	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, darkPen);

	// Just for visual check real rect area
	//Rectangle(hdc, rect1.left, rect1.top, rect1.right, rect1.bottom);

	for (int i = 0; i < 7; i++)
	{
		// set brush color and draw each segment
		if (segmentsData[dig][i] == 0)  SetDCBrushColor(hdc, BLACK);
		else SetDCBrushColor(hdc, lightPen);

		DrawSegment(hdc, i);
	}
}


static void DrawSegment(HDC hdc, int index)
{
	static POINT ptDraw[sizePT]{};

	// set segment points to rect position 
	for (int i = 0; i < sizePT; i++)
	{
		ptDraw[i].x = pt[index][i].x;
		ptDraw[i].y = pt[index][i].y;
	}
	Polygon(hdc, ptDraw, sizePT);
}

static int TransformPoints(POINT* pts, POINT* ptd, int dx, int dy, int rotate)
{
	//int grdAngle = (int)(0 + degree); // not necessary
	//double angle = RAD(grdAngle);
	for (int i = 0; i < sizePT; i++)
	{
		ptd[i].x = pts[i].x + dx;
		ptd[i].y = pts[i].y + dy;

		/*ptd[i].x = (LONG)(rotate == 0 ? pts[i].x + dx : pts[i].x * cos(angle) - pts[i].y * sin(angle) + dx);
		ptd[i].y = (LONG)(rotate == 0 ? pts[i].y + dy : pts[i].x * sin(angle) + pts[i].y * cos(angle) + dy);*/
	}
	return sizePT;
}

void SetSegmentColor(COLORREF newColor)
{
	COLORREF tempColor, darkPen = 0;
	// Set darkpen as color/8
	for (int i = 0; i < 3; i++)
	{
		tempColor = (newColor >> (8 * i) & 0xFF) >> 3;
		darkPen |= (tempColor << (8 * i));
	}
	extClassData->lightPen = newColor;
	extClassData->darkPen = darkPen;
}

COLORREF GetSegmentColor()
{
	return extClassData->lightPen;
}

void SetSegmentData(HWND hwnd, WORD data)
{
	CustomWndData* ptrw = (CustomWndData*)GetWindowLongPtr(hwnd, 0);
	ptrw->data = data;
	InvalidateRect(hwnd, nullptr, TRUE);
}