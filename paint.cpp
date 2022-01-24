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

