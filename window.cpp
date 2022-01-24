#include <Windows.h>
#include <commctrl.h>
#include <gdiplus.h>

#pragma comment(lib,"gdiplus")

#include <cassert>
#include <cmath>
#include <system_error>
#include <numbers>

#include "print.h"
#include "window.h"
#include "resource.h"

/*

                -y
                |
                |
                |
      -x  ------+------ +x
                |\) +θ
                | \
                |  \
                +y


*/


namespace Board
{
constexpr  double ringWidth         {8.0};


std::array<int,20>  sectorScore{6,10,15,2,17,3,19,7,16,8,11,14,9,12,5,20,1,18,4,13};

constexpr int  sectorWidth          {360/20};
constexpr int  sector0Start         {-sectorWidth/2};


namespace Radius
{

// dimensions in millimeters
constexpr  double board         {170.0};
constexpr  double outerDouble   {board};
constexpr  double innerDouble   {board-ringWidth};
constexpr  double outerTriple   {107.0};
constexpr  double innerTriple   {outerTriple-ringWidth};
constexpr  double outerBullseye {32.0/2.0};
constexpr  double innerBullseye {12.7/2.0};

}
}

namespace
{

HWND                theWindow   {};
HWND                theDialog   {};

constexpr int       WM_REFRESH  {WM_APP};
constexpr auto      windowStyle { WS_OVERLAPPEDWINDOW | WS_VISIBLE    };

auto    radians(double degrees)
{
    return degrees * 2 * std::numbers::pi / 360;
}


struct BoardPixels
{
    Gdiplus::Point  center;
    int     outerDouble;   
    int     innerDouble;
    int     outerTriple;
    int     innerTriple;
    int     outerBullseye;
    int     innerBullseye;
};

auto dimensions(HWND h)
{
    BoardPixels dimensions{};
    RECT        client{};
    GetClientRect(h,&client);

    auto const clientWidth{client.right-client.left};
    auto const clientHeight{client.bottom-client.top};

    const auto boardRadius   = clientWidth/2 - 50;

    dimensions.center.X      = clientWidth/2;
    dimensions.center.Y      = clientWidth/2;

    dimensions.outerDouble   = boardRadius;
    dimensions.innerDouble   = static_cast<int>(dimensions.outerDouble * ( Board::Radius::innerDouble / Board::Radius::board ));
    dimensions.outerTriple   = static_cast<int>(dimensions.outerDouble * ( Board::Radius::outerTriple / Board::Radius::board ));
    dimensions.innerTriple   = static_cast<int>(dimensions.outerDouble * ( Board::Radius::innerTriple / Board::Radius::board ));
    dimensions.outerBullseye = static_cast<int>(dimensions.outerDouble * ( Board::Radius::outerBullseye / Board::Radius::board ));
    dimensions.innerBullseye = static_cast<int>(dimensions.outerDouble * ( Board::Radius::innerBullseye / Board::Radius::board ));


    return dimensions;
}

void paint(HWND h,WPARAM w, LPARAM l)
{
    static Gdiplus::Pen blackPen{Gdiplus::Color::Black};


    auto dimensions{ ::dimensions(h) };

    PAINTSTRUCT paint;
    BeginPaint(h,&paint);

    Gdiplus::Graphics   board{paint.hdc};    


    for(int i=0;i<20;i++)
    {
        auto startAngle = radians((Board::sector0Start + i*Board::sectorWidth));

        Gdiplus::Point  end{    dimensions.center.X + static_cast<INT>(dimensions.outerDouble * std::sin(startAngle)),
                                dimensions.center.Y + static_cast<INT>(dimensions.outerDouble * std::cos(startAngle))};

        board.DrawLine(&blackPen, 
                       dimensions.center,
                       end);

    }


    
    EndPaint(h,&paint);
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
    
    case WM_NCHITTEST:
    case WM_MOUSEMOVE:
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
        break;        
    }


    default:
        print("msg {:#x}\n",m);
        break;
    }

    return 0;
}

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
        "bitmapWindowClass"
    };

    if(!RegisterClassA(&Class))
    {
        throw std::system_error{ static_cast<int>(GetLastError()), std::system_category(), "RegisterClass"};
    }

    theWindow = CreateWindowA(Class.lpszClassName,
                              "Palette",
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