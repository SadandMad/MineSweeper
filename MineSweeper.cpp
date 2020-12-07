#include "pch.h"
#include "framework.h"
#include "MineSweeper.h"
using namespace std;

#define SavePath	"SaveGame.msg"
#define StatsPath	"Stats.mst"
#define MineWidth   30

HINSTANCE hInst;
bool playing = false;
bool DrawingStats = false;
bool NewGame = false;

HWND hButtonClassic;
HWND hButtonNewbie;
HWND hButtonAmateur;
HWND hButtonExperienced;
HWND hButtonMySelection;

HBITMAP MS0;
HBITMAP MS1;
HBITMAP MS2;
HBITMAP MS3;
HBITMAP MS4;
HBITMAP MS5;
HBITMAP MS6;
HBITMAP MS7;
HBITMAP MS8;
HBITMAP MS9;
HBITMAP MS10;
HBITMAP MS11;
HBITMAP MS13;
HBITMAP MS15;

class GameSession
{
public:
    int width;
    int height;
    int mines;
    int steps = 0;
    int timeM = 0;
    int timeS = 0;
    int flags = 0;
    bool placed = false;
    int** Real;
    int** Field;

    GameSession()
    {
        width = 0;
        height = 0;
        mines = 0;
        Field = NULL;
        Real = NULL;
    }
    GameSession(int x, int y, int m)
    {
        mines = m;
        width = x;
        height = y;
        Real = new int*[height];
        Field = new int* [height];
        for (int line = 0; line < height; line++)
        {
            Real[line] = new int[width];
            Field[line] = new int[width];
            for (int col = 0; col < width; col++)
            {
                Real[line][col] = 0;
                Field[line][col] = 10;
            }
        }
    }

    void Place(int X, int Y)
    {
        int mine = 0;
        while (mine < mines)
        {
            int posX = rand() % width;
            int posY = rand() % height;
            if (abs(posX - X) > 1 || abs(posY - Y) > 1) //((width*height <= 100 && posX != X && posY != Y) || (width * height > 100 && (abs(posX-X)>1 || abs(posY - Y) > 1)))
            {
                if (Real[posY][posX] != 9)
                {
                    Real[posY][posX] = 9;
                    mine++;
                }
            }
        }
        placed = true;
    }
    
    void Count()
    {
        for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            if (Real[y][x] != 9)
                for (int dx = -1; dx < 2; dx++)
                for (int dy = -1; dy < 2; dy++)
                    if (x + dx >= 0 && x + dx < width && y + dy >= 0 && y + dy < height)
                        if (Real[y + dy][x + dx] == 9)
                        {
                            Real[y][x] = Real[y][x]++;
                        }
    }

    bool Defuse(int X, int Y)
    {
        if (!placed)
        {
            Place(X, Y);
            Count();
        }
        if (Field[Y][X] != 15)
        {
            if (Real[Y][X] == 9)
            {
                Field[Y][X] = 13;
                steps++;
                return false;
            }
            else
            {
                if (Field[Y][X] != Real[Y][X])
                {
                    Field[Y][X] = Real[Y][X];
                    steps++;
                    if (Real[Y][X] == 0)
                        Open(X, Y);
                    return true;
                }
            }
        }
        return true;
    }

    void Open(int X, int Y)
    {
        for (int dx = -1; dx < 2; dx++)
        for (int dy = -1; dy < 2; dy++)
            if (X + dx >= 0 && X + dx < width && Y + dy >= 0 && Y + dy < height && !(dx == 0 && dy == 0))
            {
                if (Real[Y + dy][X + dx] == 0 && Field[Y + dy][X + dx] != 0)
                {
                    Field[Y + dy][X + dx] = Real[Y + dy][X + dx];
                    Open(X + dx, Y + dy);
                }
                else
                    Field[Y + dy][X + dx] = Real[Y + dy][X + dx];
            }
    }

    void Mark(int X, int Y)
    {
        if (placed)
            switch (Field[Y][X])
            {
            case 10:
                Field[Y][X] = 15;
                flags++;
                steps++;
                break;
            case 11:
                Field[Y][X] = 10;
                break;
            case 15:
                Field[Y][X] = 11;
                flags--;
                break;
            }
    }

    bool NotWinState()
    {
        if (!placed)
            return true;
        // int flags = 0;
        bool wrfl = false;
        for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            if (Real[y][x] == 9 && !(Field[y][x] == 11 || Field[y][x] == 15))
                return true;
            if (Real[y][x] != 9 && Field[y][x] == 15)
                return true;
            /*if (Field[y][x] == 15)
                flags++;*/
            if (Field[y][x] == 11 && Real[y][x] != 9)
                wrfl = true;
        }
        if (flags == mines || !wrfl)
            return false;
        return true;
    }

    void Save()
    {
        fstream SF;
        SF.open(SavePath, fstream::out | fstream::trunc);
        if (SF.is_open())
        {
            SF.put((char)width);
            SF.put((char)height);
            SF.put((char)mines);
            SF.put((char)steps);
            SF.put((char)timeM);
            SF.put((char)timeS);
            SF.put((char)flags);
            SF.put((char)placed);

            for (int line = 0; line < height; line++)
                for (int col = 0; col < width; col++)
                {
                    SF.put((char)Real[line][col]);
                    SF.put((char)Field[line][col]);
                }
        }
        SF.close();
    }

    bool Load()
    {
        fstream SF;
        SF.open(SavePath, fstream::in);
        if (SF.is_open())
        {
            if (SF.peek() == std::ifstream::traits_type::eof())
            {
                SF.close();
                return false;
            }
            else
            {
                width = SF.get();
                height = SF.get();
                mines = SF.get();
                steps = SF.get();
                timeM = SF.get();
                timeS = SF.get();
                flags = SF.get();
                placed = SF.get();

                Real = new int* [height];
                Field = new int* [height];
                for (int line = 0; line < height; line++)
                {
                    Real[line] = new int[width];
                    Field[line] = new int[width];
                }

                for (int line = 0; line < height; line++)
                    for (int col = 0; col < width; col++)
                    {
                        Real[line][col] = SF.get();
                        Field[line][col] = SF.get();
                    }
                SF.close();
                return true;
            }
        }
        SF.close();
        return false;
    }
};

class GameStat
{
public:
    int width;
    int height;
    int mines;
    int steps;
    int timeM;
    int timeS;

    GameStat()
    {
        width = height = mines = steps = timeM = timeS = 0;
    }
    GameStat(GameSession Game)
    {
        width = Game.width;
        height = Game.height;
        mines = Game.mines;
        steps = Game.steps;
        timeM = Game.timeM;
        timeS = Game.timeS;
    }
};

struct GameStats
{
    GameStat stat;
    GameStats* next = NULL;

    void Save()
    {
        fstream SF;
        SF.open(StatsPath, fstream::out | fstream::trunc);
        if (SF.is_open() && (this != NULL))
        {
            GameStats* Cur = this;
            do
            {
                SF.put((char)(*Cur).stat.width);
                SF.put((char)(*Cur).stat.height);
                SF.put((char)(*Cur).stat.mines);
                SF.put((char)(*Cur).stat.steps);
                SF.put((char)(*Cur).stat.timeM);
                SF.put((char)(*Cur).stat.timeS);
                Cur = (*Cur).next;
            } while (Cur != NULL);
        }
        SF.close();
    }
};

GameSession Game;

GameStats* Stats = NULL;
GameStats* CurStat = NULL;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                DrawGrid(HDC);
void                DrawStats(HDC);
void                DrawNew();
void                DestroyNew();
void                WinGame(HWND);
void                LoseGame(HWND);
bool                LoadStats();


int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPER));
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MINESWEEPER);
    wcex.lpszClassName  = L"MainWNDW";

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   int posX, posY, width, height;
   if (Game.Load())
   {
       width = Game.width * MineWidth;
       height = Game.height * MineWidth;
       posX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
       posY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
       playing = true;
   }
   else
   {
       posX = CW_USEDEFAULT;
       posY = 0;
       width = 250;
       height = 350;
       
       Stats = NULL;
       CurStat = Stats;
       if (LoadStats())
       {
           DrawingStats = true;
       }
       else
       {
           NewGame = true;
       }
   }

   HWND hWnd = CreateWindowW(L"MainWNDW",L"САПёР", WS_OVERLAPPEDWINDOW,
                             posX, posY, width, height,
                             nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int xPos;
    int yPos;
    RECT Rect;

    switch (message)
    {
    case WM_CREATE:
        MS0  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS1  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS2  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS3  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS4  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS5  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS6  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS7  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS8  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_8.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS9  = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_mine.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS10 = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_10.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS11 = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_question.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS13 = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_mine.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        MS15 = (HBITMAP)LoadImageA(NULL, "Sprites/Minesweeper_flag.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        hButtonClassic = CreateWindowW(L"BUTTON", L"Олдскул", WS_CHILD | BS_PUSHBUTTON,
            25, 25, 175, 50, hWnd, (HMENU)IDM_Classic, hInst, NULL);
        hButtonNewbie = CreateWindowW(L"BUTTON", L"Новичёк", WS_CHILD | BS_PUSHBUTTON,
            25, 75, 175, 50, hWnd, (HMENU)IDM_Newbie, hInst, NULL);
        hButtonAmateur = CreateWindowW(L"BUTTON", L"Любитель", WS_CHILD | BS_PUSHBUTTON,
            25, 125, 175, 50, hWnd, (HMENU)IDM_Amateur, hInst, NULL);
        hButtonExperienced = CreateWindowW(L"BUTTON", L"Опытный", WS_CHILD | BS_PUSHBUTTON,
            25, 175, 175, 50, hWnd, (HMENU)IDM_Classic, hInst, NULL);
        hButtonMySelection = CreateWindowW(L"BUTTON", L"Выбор команды", WS_CHILD | BS_PUSHBUTTON,
            25, 225, 175, 50, hWnd, (HMENU)IDM_MySelection, hInst, NULL);
        SetTimer(hWnd, IDT_TIMER, 1000, NULL);
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (playing)
        {
            xPos = LOWORD(lParam) / MineWidth;
            yPos = HIWORD(lParam) / MineWidth;
            if (message == WM_LBUTTONDOWN)
                playing = Game.Defuse(xPos, yPos);
            else
                Game.Mark(xPos, yPos);
            InvalidateRect(hWnd, NULL, true);
            if (playing)
            {
                playing = Game.NotWinState();
                if (!playing)
                    WinGame(hWnd);
                else
                    Game.Save();
            }
            else
                LoseGame(hWnd);
        }
        break;
    case WM_COMMAND:
        {
            DestroyNew();
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_Continue:
                if (Game.Load())
                {
                    DestroyNew();
                    playing = true;
                    NewGame = false;
                    DrawingStats = false;
                    GetWindowRect(hWnd, &Rect);
                    MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
                    InvalidateRect(hWnd, NULL, true);
                }
                break;
            case IDM_Classic:
                DestroyNew();
                Game = GameSession(8, 8, 10);
                playing = true;
                NewGame = false;
                DrawingStats = false;
                GetWindowRect(hWnd, &Rect);
                MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
                InvalidateRect(hWnd, NULL, true);
                break;
            case IDM_Newbie:
                DestroyNew();
                Game = GameSession(9, 9, 10);
                playing = true;
                NewGame = false;
                DrawingStats = false;
                GetWindowRect(hWnd, &Rect);
                MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
                InvalidateRect(hWnd, NULL, true);
                break;
            case IDM_Amateur:
                DestroyNew();
                Game = GameSession(16, 16, 40);
                playing = true;
                NewGame = false;
                DrawingStats = false;
                GetWindowRect(hWnd, &Rect);
                MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
                InvalidateRect(hWnd, NULL, true);
                break;
            case IDM_Experienced:
                DestroyNew();
                Game = GameSession(30, 16, 99);
                playing = true;
                NewGame = false;
                DrawingStats = false;
                GetWindowRect(hWnd, &Rect);
                MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
                InvalidateRect(hWnd, NULL, true);
                break;
            case IDM_MySelection:
                DestroyNew();
                Game = GameSession(19, 19, 50);
                playing = true;
                NewGame = false;
                DrawingStats = false;
                GetWindowRect(hWnd, &Rect);
                MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
                InvalidateRect(hWnd, NULL, true);
                break;
            case IDM_STATS:
                if (playing)
                    Game.Save();
                DestroyNew();
                playing = false;
                NewGame = false;
                DrawingStats = true;
                GetWindowRect(hWnd, &Rect);
                MoveWindow(hWnd, Rect.left, Rect.top, 250, 350, TRUE);
                InvalidateRect(hWnd, NULL, true);
                break;
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
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            if (DrawingStats)
            {
                DrawStats(hdc);
            }
            else if (NewGame)
            {
                DrawNew();
            }
            else
            {
                DrawGrid(hdc);
            }
            EndPaint(hWnd, &ps);
            UpdateWindow(hWnd);
        }
        break;
    case WM_SIZE:
        if (playing)
        {
            GetWindowRect(hWnd, &Rect);
            MoveWindow(hWnd, Rect.left, Rect.top, MineWidth * Game.width + 15, MineWidth * Game.height + 88, TRUE);
        }
        break;
    case WM_DESTROY:
        if (playing)
            Game.Save();
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        if (playing)
        {
            Game.timeS++;
            if (Game.timeS == 60)
            {
                Game.timeM++;
                Game.timeS = 0;
            }
            string time;
            if (Game.timeM > 0)
                time = to_string(Game.timeM) + ":";
            time += to_string(Game.timeS);
            RECT Rect;
            Rect.top = MineWidth * Game.height;
            Rect.left = 25;
            Rect.right = 25 + strlen(time.c_str()) * 8;
            Rect.bottom = Rect.top + 20;
            InvalidateRect(hWnd, &Rect, true);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
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

void DrawGrid(HDC hdc)
{
    for (int y = 0; y < Game.height; y++)
    for (int x = 0; x < Game.width; x++)
    {
        HDC hCompatibleDC = CreateCompatibleDC(hdc);
        switch (Game.Field[y][x])
        {
        case 0:
            SelectObject(hCompatibleDC, MS0);
            break;
        case 1:
            SelectObject(hCompatibleDC, MS1);
            break;
        case 2:
            SelectObject(hCompatibleDC, MS2);
            break;
        case 3:
            SelectObject(hCompatibleDC, MS3);
            break;
        case 4:
            SelectObject(hCompatibleDC, MS4);
            break;
        case 5:
            SelectObject(hCompatibleDC, MS5);
            break;
        case 6:
            SelectObject(hCompatibleDC, MS6);
            break;
        case 7:
            SelectObject(hCompatibleDC, MS7);
            break;
        case 8:
            SelectObject(hCompatibleDC, MS8);
            break;
        case 9:
            SelectObject(hCompatibleDC, MS9);
            break;
        case 10:
            SelectObject(hCompatibleDC, MS10);
            break;
        case 11:
            SelectObject(hCompatibleDC, MS11);
            break;
        case 13:
            SelectObject(hCompatibleDC, MS13);
            break;
        case 15:
            SelectObject(hCompatibleDC, MS15);
            break;
        }
        StretchBlt(hdc, MineWidth * x, MineWidth * y, MineWidth, MineWidth, hCompatibleDC, 0, 0, 76, 76, SRCCOPY);
        DeleteObject(hCompatibleDC);
    }
    string time;
    if (Game.timeM > 0)
        time = to_string(Game.timeM) + ":";
    time += to_string(Game.timeS);
    TextOutA(hdc, 25, MineWidth* Game.height + 5, (time.c_str()), strlen(time.c_str()));
    string flString = to_string(Game.flags) + "/" + to_string(Game.mines);
    TextOutA(hdc, (MineWidth * Game.width - strlen(flString.c_str()) * 8) / 2, MineWidth * Game.height + 5, (flString.c_str()), strlen(flString.c_str()));
    TextOutA(hdc, MineWidth * Game.width - 25 - strlen(to_string(Game.steps).c_str()) * 8, MineWidth * Game.height + 5, (to_string(Game.steps).c_str()), strlen(to_string(Game.steps).c_str()));
}

void DrawStats(HDC hdc)
{
    RECT rect;
    rect.top = 0;
    rect.left = 10;
    rect.right = 230;
    rect.bottom = 40;
    
    GameStats* CurStats = Stats;
    while (CurStats != NULL)
    {
        string otp = to_string(CurStats->stat.width) + " x " + to_string(CurStats->stat.height) + " | " + to_string(CurStats->stat.mines) + " мин | " +
                     to_string(CurStats->stat.steps) + " шаг | " + to_string(CurStats->stat.timeM) + ":" + to_string(CurStats->stat.timeS);
        int D = DrawTextA(hdc, otp.c_str(), strlen(otp.c_str()), &rect, DT_NOCLIP || DT_CALCRECT);
        CurStats = CurStats->next;
        rect.top += D;
        rect.bottom += D;
    }
}

void DrawNew()
{
    ShowWindow(hButtonClassic, SW_SHOW);
    ShowWindow(hButtonNewbie, SW_SHOW);
    ShowWindow(hButtonAmateur, SW_SHOW);
    ShowWindow(hButtonExperienced, SW_SHOW);
    ShowWindow(hButtonMySelection, SW_SHOW);
}

void DestroyNew()
{
    ShowWindow(hButtonClassic, SW_HIDE);
    ShowWindow(hButtonNewbie, SW_HIDE);
    ShowWindow(hButtonAmateur, SW_HIDE);
    ShowWindow(hButtonExperienced, SW_HIDE);
    ShowWindow(hButtonMySelection, SW_HIDE);

}

void WinGame(HWND wnd)
{
    PlaySound(L"Victory.wav", NULL, SND_ASYNC | SND_FILENAME);
    DeleteFileA(SavePath);
    GameStat CurGame = GameStat(Game);
    if (Stats == NULL)
    {
        CurStat = new GameStats;
        CurStat->stat = CurGame;
        CurStat->next = NULL;
        Stats = CurStat;
    }
    else
    {
        GameStats* temp = new GameStats;
        temp->stat = CurGame;
        temp->next = NULL;
        CurStat->next = temp;
        CurStat = temp;
    }
    Stats->Save();
    MessageBoxA(wnd, "Поздравляем вас с решнием головоломки!", "Вы победили!", MB_OK);
}

void LoseGame(HWND wnd)
{
    PlaySound(L"Lose.wav", NULL, SND_ASYNC | SND_FILENAME);
    DeleteFileA(SavePath);
    MessageBoxA(wnd, "Не бойтесь попробовать свои силы ещё раз!", "Вас разорвало!", MB_OK);
}

bool LoadStats()
{
    fstream SF;
    SF.open(StatsPath, fstream::in);
    if (SF.is_open())
    {
        if (SF.peek() == std::ifstream::traits_type::eof())
        {
            SF.close();
            return false;
        }
        else
        {
            while (SF.peek() != std::ifstream::traits_type::eof())
            {
                GameStat CurGame;
                char ch;
                SF.get(ch);
                CurGame.width = ch;
                SF.get(ch);
                CurGame.height = ch;
                SF.get(ch);
                CurGame.mines = ch;
                SF.get(ch);
                CurGame.steps = ch;
                SF.get(ch);
                CurGame.timeM = ch;
                SF.get(ch);
                CurGame.timeS = ch;

                if (Stats == NULL)
                {
                    Stats = new GameStats;
                    Stats->stat = CurGame;
                    Stats->next = NULL;
                    CurStat = Stats;
                }
                else
                {
                    GameStats* temp = new GameStats;
                    temp->stat = CurGame;
                    temp->next = NULL;
                    CurStat->next = temp;
                    CurStat = temp;
                }
            }
            return true;
        }
    }
    SF.close();
    return false;
}