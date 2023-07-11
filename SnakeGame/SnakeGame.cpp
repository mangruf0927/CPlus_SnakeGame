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
void Food_Place(HWND hWnd, RECT rect, int* fx, int* fy);
void Food_Draw(HDC hdc, int* fx, int* fy);


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
    if (!InitInstance (hInstance, nCmdShow))
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

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKEGAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNAKEGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

const int circleRadius = 10; //뱀 동그라미 반지름 

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HPEN hPen, oldPen;
    HBRUSH hBrush, oldBrush;

    static POINT ptCurPos;
    static RECT rectView;

    static int flag = 1; 
    static int score = 0; // 점수 
    static int hkey, ckey = 0; // 방향키 저장
    TCHAR str[128]; // 점수 출력 

    // 뱀의 시작 위치
    static int xSnake[100] = { 70, 50 };
    static int ySnake[100] = { 90, 90 };

    // 뱀의 시작 길이
    static int length = 1;

    // 먹이 위치
    static int xfood;
    static int yfood;
  

    switch (message)
    {
    case WM_CREATE :
    {
        GetClientRect(hWnd, &rectView);
        
        srand(time(NULL));
        SetTimer(hWnd, timer_ID_1, 100, NULL); // 뱀 이동 속도 
        
        srand(time(NULL));
        SetTimer(hWnd, timer_ID_2, 4000, NULL); // 먹이 생성 속도 
    }
        break;

    case WM_TIMER:
    {

        switch (wParam)
        {
        case timer_ID_2:
        {
            hdc = GetDC(hWnd);
            Food_Place(hWnd, rectView, &xfood, &yfood);
            //먹이 좌표 재생성

            ReleaseDC(hWnd, hdc);
        }

        case timer_ID_1:
        {
            switch (ckey)
            {
            case 1:
            {
                for (int i = 0; i < length; i++)
                {
                    xSnake[length - i] = xSnake[length - i - 1];
                    ySnake[length - i] = ySnake[length - i - 1];
                }
                xSnake[0] += 20;
            }
            break;
            case 2:
            {
                for (int i = 0; i < length; i++)
                {
                    xSnake[length - i] = xSnake[length - i - 1];
                    ySnake[length - i] = ySnake[length - i - 1];
                }
                xSnake[0] -= 20;
            }
            break;
            case 3:
            {
                for (int i = 0; i < length; i++) {
                    xSnake[length - i] = xSnake[length - i - 1];
                    ySnake[length - i] = ySnake[length - i - 1];
                }
                ySnake[0] -= 20;
            }
            break;
            case 4:
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
                MessageBox(hWnd, _T("GAME OVER"), _T("종료"), MB_OK);
                DestroyWindow(hWnd);
            }

            // 먹이 충돌
            if (xSnake[0] == xfood && ySnake[0] == yfood) {
                xSnake[length + 1] = xSnake[length];
                ySnake[length + 1] = ySnake[length];
                length++; // 뱀 꼬리 증가          
                score++; //점수 증가

                Food_Place(hWnd, rectView, &xfood, &yfood); //먹이 위치 재배치             

                KillTimer(hWnd, timer_ID_2);
                SetTimer(hWnd, timer_ID_2, 5000, NULL); //타이머 다시 발동
                
            }

            // 몸통 충돌
            for (int i = 1; i <= length; i++)
            {
                if (xSnake[0] == xSnake[i] && ySnake[0] == ySnake[i])
                {
                    flag = 0;
                    KillTimer(hWnd, timer_ID_1);
                    KillTimer(hWnd, timer_ID_2);
                    MessageBox(hWnd, _T("GAME OVER"), _T("종료"), MB_OK);
                    DestroyWindow(hWnd);
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
        
        InvalidateRect(hWnd, NULL, TRUE);
    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            
            
            // 게임 테두리
            hBrush = CreateSolidBrush(RGB(100, 100, 100));
            oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            DrawBorder(hdc, rectView);
            SelectObject(hdc, oldBrush);
            DeleteObject(hBrush);
            

            // 아이템 & 뱀 출력 
            if (flag > 0)
            {
                Food_Draw(hdc, &xfood, &yfood);
                Snake(hdc, xSnake, ySnake, length);
            }

            // 점수 출력
            wsprintf(str, TEXT("점수: %d"), score);
            TextOut(hdc, 15, 13, str, lstrlen(str));

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, timer_ID_1);
        KillTimer(hWnd, timer_ID_2);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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
            SelectObject(hdc, CreateSolidBrush(RGB(51, 51, 0))); 

        }
        else // 몸통 색
        {
            SelectObject(hdc, CreateSolidBrush(RGB(204, 204, 153))); 
        }
        DrawCircle(hdc, point, circleRadius);
    }
}

//Rectangle(hdc, 20, 60, 460, 520); 게임 영역

void Food_Place(HWND hWnd, RECT rectView, int* xfood, int* yfood) 
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

void Food_Draw(HDC hdc, int* xfood, int* yfood) 
{
    SelectObject(hdc, CreateSolidBrush(RGB(250, 244, 192)));
    POINT point = {*xfood, *yfood};
    DrawStar(hdc, point, 10);
}
