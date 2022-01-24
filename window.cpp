#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <gdiplus.h>

#pragma comment(lib,"gdiplus")

#include <cassert>
#include <cmath>
#include <system_error>
#include <numbers>
#include <tuple>

#include "print.h"
#include "window.h"
#include "resource.h"

#include "dimensions.h"



HWND                theWindow   {};
HWND                theDialog   {};

constexpr int       WM_REFRESH  {WM_APP};
constexpr auto      windowStyle { WS_OVERLAPPEDWINDOW | WS_VISIBLE    };

POINT               mousePosition{};
int                 accuracy{};             





void mouseMove(HWND h, int x, int y)
{
    mousePosition = {x,y};

    auto board { boardDimensions(h)};

    x-=board.center.X;
    y-=board.center.Y;

    auto [score, multiplier] = scoreFromPoint(board,x,y);

    std::string totalScore;

    if(score == 0)
    {
        totalScore="Miss";
    }
    else
    {
        if(multiplier == 2)
        {
            totalScore = "Double ";
        }
        else if(multiplier == 3)
        {
            totalScore = "Triple ";
        }

        totalScore += std::to_string(score);
    }

    SetDlgItemText(theDialog, IDC_AIMING_AT, totalScore.c_str());
}



LRESULT CALLBACK windowProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch(m)
    {
    case WM_CREATE:
    {        
        SetForegroundWindow(h);

        return 0;
    }

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        paint(h,w,l);
        return 0;

    case WM_REFRESH:
        InvalidateRect(h,nullptr,FALSE);
        return 0;
    
    case WM_MOUSEMOVE:

        mouseMove(h, GET_X_LPARAM(l), GET_Y_LPARAM(l));
        PostMessage(theWindow,WM_REFRESH,0,0);

        return DefWindowProc(h,m,w,l);

    case WM_NCHITTEST:
    case WM_NCMOUSEMOVE:
    case WM_SETCURSOR:
        break;

    default:
        //print("msg {:#x}\n",m);
        break;
    }

    return DefWindowProc(h,m,w,l);
}


INT_PTR CALLBACK dialogProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch(m)
    {
    case WM_COMMAND:

        switch(LOWORD(w))
        {
        case IDCANCEL:
            PostQuitMessage(0);
            EndDialog(h,0);
            break;
        }
        break;

    case WM_INITDIALOG:
        ShowWindow(h,SW_SHOW);
        return false;

    case WM_NOTIFY:
    case WM_SETCURSOR:
    case WM_NCHITTEST:
    case WM_MOUSEMOVE:
    case WM_CTLCOLORSTATIC:
        break;

    case WM_HSCROLL:
    {
        accuracy= 2 + static_cast<int>(SendDlgItemMessage(h,IDC_SATURATION,TBM_GETPOS,0,0));

        PostMessage(theWindow,WM_REFRESH,0,0);
        break;        
    }


    default:
//        print("msg {:#x}\n",m);
        break;
    }

    return 0;
}



void createWindow()
{
    ULONG_PTR                           token{};
    Gdiplus::GdiplusStartupInput const  input{};
    Gdiplus::GdiplusStartupOutput       output{};

    Gdiplus::GdiplusStartup(&token,&input,&output);

    WNDCLASSA    Class
    {
        CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        windowProc,
        0,
        0,
        GetModuleHandle(nullptr),
        nullptr,
        LoadCursor(nullptr,IDC_ARROW),
        static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)),
        nullptr,
        "dartScore"
    };

    if(!RegisterClassA(&Class))
    {
        throw std::system_error{ static_cast<int>(GetLastError()), std::system_category(), "RegisterClass"};
    }

    theWindow = CreateWindowA(Class.lpszClassName,
                              "Dart score",
                              windowStyle,
                              CW_USEDEFAULT,CW_USEDEFAULT,
                              800,900,
                              nullptr,
                              nullptr,
                              GetModuleHandle(nullptr),
                              nullptr);

    if(theWindow==nullptr)
    {
        throw std::system_error{ static_cast<int>(GetLastError()), std::system_category(), "RegisterClass"};
    }
}


void createDialog()
{
    theDialog=CreateDialog(nullptr,MAKEINTRESOURCE(IDD_DIALOG),nullptr,dialogProc);
}


void windowMessageLoop()
{
    MSG     msg;
    while(GetMessage(&msg,0,0,0) > 0)
    {
        if(!IsDialogMessage(theDialog, &msg))
        {
            DispatchMessage(&msg);
        }
    }
}



int main()
{
    createWindow();
    createDialog();
    windowMessageLoop();
}