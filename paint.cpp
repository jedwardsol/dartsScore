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
    auto const boardRadius   = min(clientWidth,clientHeight) /2 - 50;


    auto makeRect = [&](int radius)
    {
        return Gdiplus::Rect
        {
            clientWidth/2 - radius,
            clientHeight/2 - radius,
            radius*2,
            radius*2,
        };
        
    };


    BoardDimensions dimensions
    {
        {clientWidth/2,clientHeight/2},
    
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
    Gdiplus::REAL   angle;

    Gdiplus::Point  outerDouble;   
    Gdiplus::Point  innerDouble;
    Gdiplus::Point  outerTriple;
    Gdiplus::Point  innerTriple;
    Gdiplus::Point  center;
};



auto radiusDimensions(BoardDimensions  &board, int sectorNumber)
{
    Gdiplus::REAL angle = 1.0f * Board::sector0Start + sectorNumber*Board::sectorWidth;

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
    static Gdiplus::Font        font      {&family, 32, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel};


    auto board{ ::boardDimensions(h) };

    PAINTSTRUCT paint;
    BeginPaint(h,&paint);

    Gdiplus::Graphics   window{paint.hdc};    

    window.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);


    for(int i=0;i<20;i++)
    {
        auto  sectorStart { ::radiusDimensions(board,i)};
        auto  sectorEnd   { ::radiusDimensions(board,i+1)};

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

        auto const number      {std::to_wstring(Board::sectorScore[i])};
        auto const sectorText  { ::sectorText(board,i)};

        Gdiplus::RectF  textRect{};
        window.MeasureString (number.c_str(), -1, &font, sectorText, &textRect);

        textRect.X -= textRect.Width/2;
        textRect.Y -= textRect.Height/2;

        window.DrawString(number.c_str(),-1,&font,textRect,nullptr,&blackBrush);
    }


    window.FillEllipse(&greenBrush,board.rect.outerBullseye);
    window.FillEllipse(&redBrush,  board.rect.innerBullseye);

    EndPaint(h,&paint);
}

