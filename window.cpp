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


struct BoardDimensions  // pixels
{
    Gdiplus::Point  center;

    struct 
    {
        int     outerDouble;   
        int     innerDouble;
        int     outerTriple;
        int     innerTriple;
        int     outerBullseye;
        int     innerBullseye;
    } radius;

    struct 
    {
        Gdiplus::Rect outerDouble;   
        Gdiplus::Rect innerDouble;
        Gdiplus::Rect outerTriple;
        Gdiplus::Rect innerTriple;
        Gdiplus::Rect outerBullseye;
        Gdiplus::Rect innerBullseye;
    } rect;
};





auto boardDimensions(HWND h)
{
    RECT            client{};
    GetClientRect(h,&client);

    auto const clientWidth{client.right-client.left};
    auto const clientHeight{client.bottom-client.top};
    auto const boardRadius   = clientWidth/2 - 50;


    auto makeRect = [&](int radius)
    {
        return Gdiplus::Rect
        {
            clientWidth/2 - radius,
            clientWidth/2 - radius,
            radius*2,
            radius*2,
        };
        
    };


    BoardDimensions dimensions
    {
        {clientWidth/2,clientWidth/2},
    
        {
           boardRadius,
           static_cast<int>(boardRadius * ( Board::Radius::innerDouble   / Board::Radius::board )),
           static_cast<int>(boardRadius * ( Board::Radius::outerTriple   / Board::Radius::board )),
           static_cast<int>(boardRadius * ( Board::Radius::innerTriple   / Board::Radius::board )),
           static_cast<int>(boardRadius * ( Board::Radius::outerBullseye / Board::Radius::board )),
           static_cast<int>(boardRadius * ( Board::Radius::innerBullseye / Board::Radius::board ))
        },

        {
            makeRect(dimensions.radius.outerDouble),
            makeRect(dimensions.radius.innerDouble),
            makeRect(dimensions.radius.outerTriple),
            makeRect(dimensions.radius.innerTriple),
            makeRect(dimensions.radius.outerBullseye),
            makeRect(dimensions.radius.innerBullseye),
        }
    };


    return dimensions;
}

struct RadiusDimensions // pixels
{
    int             angle;

    Gdiplus::Point  outerDouble;   
    Gdiplus::Point  innerDouble;
    Gdiplus::Point  outerTriple;
    Gdiplus::Point  innerTriple;
    Gdiplus::Point  center;
};



auto radiusDimensions(BoardDimensions  &board, int sectorNumber)
{
    auto angle = Board::sector0Start + sectorNumber*Board::sectorWidth;

    auto makePoint = [&](int radius)
    {
        return Gdiplus::Point 
        {
            board.center.X + static_cast<INT>(radius * std::cos(radians(angle))),
            board.center.Y + static_cast<INT>(radius * std::sin(radians(angle)))
        };
    };

    RadiusDimensions    dimensions
    {
        angle,
        makePoint(board.radius.outerDouble),
        makePoint(board.radius.innerDouble),
        makePoint(board.radius.outerTriple),
        makePoint(board.radius.innerTriple),
        board.center
    };

    return dimensions;
}


auto sectorText(BoardDimensions  &board, int sectorNumber)
{
    auto angle = sectorNumber*Board::sectorWidth;

    auto makePoint = [&](int radius)
    {
        return Gdiplus::PointF 
        {
            board.center.X + static_cast<Gdiplus::REAL>(radius * std::cos(radians(angle))),
            board.center.Y + static_cast<Gdiplus::REAL>(radius * std::sin(radians(angle)))
        };
    };

    return makePoint(board.radius.outerDouble+30);

}




void paint(HWND h,WPARAM w, LPARAM l)
{
    static Gdiplus::Pen         blackPen  {Gdiplus::Color::Black};

    static Gdiplus::SolidBrush  blackBrush{Gdiplus::Color::Black};
    static Gdiplus::SolidBrush  whiteBrush{Gdiplus::Color::White};
    static Gdiplus::SolidBrush  greenBrush{Gdiplus::Color::DarkGreen};
    static Gdiplus::SolidBrush  redBrush  {Gdiplus::Color::DarkRed};

    static Gdiplus::FontFamily  family    {L"Times New Roman"};
    static Gdiplus::Font        font      {&family, 24, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel};


    auto board{ ::boardDimensions(h) };

    PAINTSTRUCT paint;
    BeginPaint(h,&paint);

    Gdiplus::Graphics   window{paint.hdc};    

    window.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);


    for(int i=0;i<20;i++)
    {
        auto  sectorStart { ::radiusDimensions(board,i)};
        auto  sectorEnd   { ::radiusDimensions(board,i+1)};
        auto  sectorText  { ::sectorText(board,i)};

        Gdiplus::GraphicsPath   sector;
        Gdiplus::GraphicsPath   doubleArc;
        Gdiplus::GraphicsPath   tripleArc;

        sector.AddLine(sectorStart.center  ,            sectorStart.outerDouble);
        sector.AddArc (board.rect.outerDouble,sectorStart.angle,Board::sectorWidth);

        doubleArc.AddLine(sectorStart.innerDouble, sectorStart.outerDouble);
        doubleArc.AddArc (board.rect.outerDouble, sectorStart.angle,Board::sectorWidth);
        doubleArc.AddLine(sectorEnd.outerDouble, sectorEnd.innerDouble);
        doubleArc.AddArc (board.rect.innerDouble, sectorEnd.angle,-Board::sectorWidth);

        tripleArc.AddLine(sectorStart.innerTriple, sectorStart.outerTriple);
        tripleArc.AddArc (board.rect.outerTriple, sectorStart.angle,Board::sectorWidth);
        tripleArc.AddLine(sectorEnd.outerTriple, sectorEnd.innerTriple);
        tripleArc.AddArc (board.rect.innerTriple, sectorEnd.angle,-Board::sectorWidth);

        window.FillPath((i % 2) ? &blackBrush : &whiteBrush, &sector);
        window.FillPath((i % 2) ? &redBrush   : &greenBrush, &doubleArc);
        window.FillPath((i % 2) ? &redBrush   : &greenBrush, &tripleArc);

        window.DrawString(std::to_wstring(Board::sectorScore[i]).c_str(),-1,&font,sectorText,&blackBrush);


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