// SnakeGame.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "SnakeGame.h"
#include <ctime>
#include <cmath>

#define PI 3.141592
#define degreeToRadian(degree) ((degree) * PI / 180)


// >> : 함수
void DrawRectangle(HDC hdc, POINT center, int s);
void DrawCircle(HDC hdc, POINT center, int r);
void DrawStar(HDC hdc, POINT center, int r);
void DrawBorder(HDC hdc, RECT rectView);
// <<

void Snake(HDC hdc, int* x, int* y, int length);
void FoodPlace(HWND hWnd, RECT rect, int* fx, int* fy);
void FoodDraw(HDC hdc, int* fx, int* fy);
void StartSetting(HWND hWnd);


// >> : WinAPI 사용하면서 콘솔창 동시에 띄우기
//#ifdef UNICODE
//#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
//#else
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") 
//#endif
// << 



#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SNAKEGAME, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 애플리케이션 초기화를 수행합니다:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKEGAME));

	MSG msg;

	// 기본 메시지 루프입니다:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKEGAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(229, 255, 204));
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SNAKEGAME);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, _T("SNAKEGAME"), WS_OVERLAPPEDWINDOW,
		715, 280, 496, 598, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

#define timer_ID_1 11
#define timer_ID_2 61

#define Title 1
#define Game 2
#define GameOver 3

#define REGAME 0		
#define EXIT 1	

const int circleRadius = 10; //뱀 동그라미 반지름 
static POINT ptCurPos;

static int flag = 1;
static int hkey, ckey = 0; // 방향키 저장
static int progress = 1; // 진행 상태  
static int score = 0; // 점수 

// 뱀의 시작 위치
static int xSnake[80] = { 70, 50 };
static int ySnake[80] = { 90, 90 };

// 뱀의 시작 길이
static int length = 1;

// 먹이 위치
static int xfood = 170;
static int yfood = 290;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rectView;
	TCHAR str[128]; // 점수 출력 
	static int menu; // 메뉴



	switch (message)
	{
	case WM_CREATE:
	{
		GetClientRect(hWnd, &rectView);
		StartSetting(hWnd);
	}
	break;

	case WM_TIMER:
	{
		switch (wParam)
		{
			hdc = GetDC(hWnd);
			FoodPlace(hWnd, rectView, &xfood, &yfood); //먹이 생성
			ReleaseDC(hWnd, hdc);

		case timer_ID_1:
		{
			switch (ckey)
			{
			case 1: // 오른쪽
			{
				for (int i = 0; i < length; i++)
				{
					xSnake[length - i] = xSnake[length - i - 1];
					ySnake[length - i] = ySnake[length - i - 1];
				}
				xSnake[0] += 20;
			}
			break;
			case 2: // 왼쪽
			{
				for (int i = 0; i < length; i++)
				{
					xSnake[length - i] = xSnake[length - i - 1];
					ySnake[length - i] = ySnake[length - i - 1];
				}
				xSnake[0] -= 20;
			}
			break;
			case 3: // 위
			{
				for (int i = 0; i < length; i++) {
					xSnake[length - i] = xSnake[length - i - 1];
					ySnake[length - i] = ySnake[length - i - 1];
				}
				ySnake[0] -= 20;
			}
			break;
			case 4: // 아래
			{
				for (int i = 0; i < length; i++) {
					xSnake[length - i] = xSnake[length - i - 1];
					ySnake[length - i] = ySnake[length - i - 1];
				}
				ySnake[0] += 20;
			}
			break;
			}

			// 벽 충돌
			if (xSnake[0] > rectView.right - 30 || xSnake[0] < rectView.left + 30 ||
				ySnake[0] < rectView.top + 20 * 3 || ySnake[0] > rectView.bottom - 20)
			{
				flag = 0;
				KillTimer(hWnd, timer_ID_1);
				KillTimer(hWnd, timer_ID_2);
				MessageBox(hWnd, _T("게임 오버"), _T("GAME OVER"), MB_OK);
				progress = GameOver;
				break;
				//DestroyWindow(hWnd);
			}

			// 먹이
			if (xSnake[0] == xfood && ySnake[0] == yfood) {
				xSnake[length + 1] = xSnake[length];
				ySnake[length + 1] = ySnake[length];
				length++; // 뱀 꼬리 증가          
				score++; //점수 증가

				FoodPlace(hWnd, rectView, &xfood, &yfood); //먹이 위치 재배치             
			}

			// 몸통 충돌
			for (int i = 2; i <= length; i++)
			{
				if (xSnake[0] == xSnake[i] && ySnake[0] == ySnake[i])
				{
					flag = 0;
					KillTimer(hWnd, timer_ID_1);
					MessageBox(hWnd, _T("게임 오버"), _T("GAME OVER"), MB_OK);
					progress = GameOver;
					break;
					//DestroyWindow(hWnd);
				}
			}

		}
		}
		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)	//ESC키일경우 탈출
		{
			DestroyWindow(hWnd);
		}

		switch (progress)
		{
		case Title:
		{
			if (wParam == VK_RETURN)
			{
				progress = Game; // 타이틀 -> 게임
				InvalidateRgn(hWnd, NULL, TRUE);
			}
		}
		break;
		case Game:
		{
			if (wParam == VK_RIGHT)
			{
				if (hkey != 1 && hkey != 2)
				{
					ckey = 1;
					hkey = ckey;
				}
			}
			else if (wParam == VK_LEFT)
			{
				if (hkey != 1 && hkey != 2)
				{
					ckey = 2;
					hkey = ckey;
				}

			}
			else if (wParam == VK_UP)
			{
				if (hkey != 3 && hkey != 4)
				{
					ckey = 3;
					hkey = ckey;
				}

			}
			else if (wParam == VK_DOWN)
			{
				if (hkey != 3 && hkey != 4)
				{
					ckey = 4;
					hkey = ckey;
				}
			}
		}
		case GameOver:
		{
			if (wParam == VK_RETURN)
			{
				switch (menu)
				{
				case REGAME:
					StartSetting(hWnd);
					InvalidateRgn(hWnd, NULL, TRUE);
					break;
				case EXIT:
					DestroyWindow(hWnd);
					break;
				}
			}
			else if (wParam == VK_UP && menu == EXIT)
				menu = REGAME;
			else if (wParam == VK_DOWN && menu == REGAME)
				menu = EXIT;
		}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		//Rectangle(hdc, 20, 60, 460, 520); 게임 영역

		switch (progress)
		{
		case Title: // 시작 화면
		{
			TextOut(hdc, 200, 210, _T("SNAKE GAME!"), _tcslen(_T("SNAKE GAME!")));
			TextOut(hdc, 192, 230, _T("Press Enter Key"), _tcslen(_T("Press Enter Key")));

		}
		break;
		case Game: // 게임 화면
		{
			// 아이템 & 뱀 출력 
			if (flag > 0)
			{
				FoodDraw(hdc, &xfood, &yfood);
				Snake(hdc, xSnake, ySnake, length);
			}

			// 게임 테두리
			DrawBorder(hdc, rectView);


			// 점수 출력
			wsprintf(str, TEXT("점수: %d"), score);
			TextOut(hdc, 15, 13, str, lstrlen(str));
		}
		break;
		case GameOver: // 종료 화면
		{
			TextOut(hdc, 200, 210, _T("GAME OVER"), _tcslen(_T("GAME OVER")));
			wsprintf(str, _T("최종점수 : %d"), score);
			TextOut(hdc, 196, 230, str, _tcslen(str));
		
			TextOut(hdc, 200, 270, _T("다시 하기"), _tcslen(_T("다시 하기")));
			TextOut(hdc, 200, 290, _T("종료"), _tcslen(_T("종료")));
			
			switch (menu)
			{
			case REGAME:
				TextOut(hdc, 150, 270, _T("▶"), _tcslen(_T("▶")));
				break;
			case EXIT :
				TextOut(hdc, 150, 290, _T("▶"), _tcslen(_T("▶")));
				break;
			}
			
		}
		break;
		}
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		KillTimer(hWnd, timer_ID_1);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
	}
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//도형 함수
void DrawRectangle(HDC hdc, POINT center, int s)
{
	Rectangle(hdc, center.x - s, center.y - s, center.x + s, center.y + s);
}

void DrawCircle(HDC hdc, POINT center, int r)
{
	Ellipse(hdc, center.x - r, center.y - r, center.x + r, center.y + r);
}

void DrawStar(HDC hdc, POINT center, int r)
{
	POINT point[10];
	double theta = degreeToRadian(360 / 5);

	for (int i = 0; i < 5; i++)
	{
		point[2 * i].x = r * cos(theta * i) + center.x;
		point[2 * i].y = r * sin(theta * i) + center.y;
	}

	for (int i = 0; i < 5; i++)
	{
		point[2 * i + 1].x = r / 2 * cos(theta / 2 + theta * i) + center.x;
		point[2 * i + 1].y = r / 2 * sin(theta / 2 + theta * i) + center.y;

	}

	Polygon(hdc, point, 10);
}

// 맵 테두리
void DrawBorder(HDC hdc, RECT rectView)
{
	SelectObject(hdc, CreateSolidBrush(RGB(96, 96, 96)));
	SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(64, 64, 64)));
	for (int i = rectView.left; i < rectView.right; i += 20)
	{
		POINT point = { 10 + i, 50 };
		DrawRectangle(hdc, point, 10);
		POINT point2 = { 10 + i, 530 };
		DrawRectangle(hdc, point2, 10);
	}



	for (int j = rectView.top + 40; j < rectView.bottom; j += 20)
	{
		POINT point = { 10, 10 + j };
		DrawRectangle(hdc, point, 10);
		POINT point2 = { 470, 10 + j };
		DrawRectangle(hdc, point2, 10);
	}
}

// 뱀
void Snake(HDC hdc, int* x, int* y, int length) {
	for (int i = 0; i <= length; i++)
	{
		POINT point = { x[i], y[i] };
		if (i == 0) // 머리 색
		{
			SelectObject(hdc, CreateSolidBrush(RGB(142, 128, 89)));
			SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(142, 128, 89)));

		}
		else // 몸통 색
		{
			SelectObject(hdc, CreateSolidBrush(RGB(196, 182, 143)));
			SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(196, 182, 143)));
		}
		DrawCircle(hdc, point, circleRadius);
	}
}

//Rectangle(hdc, 20, 60, 460, 520); 게임 영역

void FoodPlace(HWND hWnd, RECT rectView, int* xfood, int* yfood)
{
	int temp;
	do {
		*xfood = (rand() % 440 + 20);
		temp = *xfood % 10;
		*xfood -= temp;
	} while (*xfood % 20 != 10); // 먹이 x좌표 

	do {
		*yfood = (rand() % 460 + 60);
		temp = *yfood % 10;
		*yfood -= temp;
	} while (*yfood % 20 != 10); // 먹이 y좌표

	InvalidateRgn(hWnd, NULL, TRUE);
}

void FoodDraw(HDC hdc, int* xfood, int* yfood)
{
	SelectObject(hdc, CreateSolidBrush(RGB(255, 102, 102)));
	SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(255, 102, 102)));
	POINT point = { *xfood, *yfood };
	DrawStar(hdc, point, 11);
}

void StartSetting(HWND hWnd)
{
	flag = 1;
	hkey = 0, ckey = 0; // 방향키 저장
	progress = 1; // 진행 상태  
	score = 0; // 점수 

	xSnake[0] = 70;
	xSnake[1] = 50;
	ySnake[0] = 90;
	ySnake[1] = 90;

	// 뱀의 시작 위치
	for (int i = 2; i < 80; i++)
	{
		xSnake[i] = 0;
		ySnake[i] = 0;
	}

	// 뱀의 시작 길이
	length = 1;

	// 먹이 위치
	xfood = 170;
	yfood = 290;


	srand(time(NULL));
	SetTimer(hWnd, timer_ID_1, 95, NULL); // 뱀 이동 속도 
}