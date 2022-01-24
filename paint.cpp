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



void paintBoard(Gdiplus::Graphics   &window, BoardDimensions const &board)
{
    static Gdiplus::Pen         blackPen  {Gdiplus::Color::Black};

    static Gdiplus::SolidBrush  blackBrush{Gdiplus::Color::DarkGray};
    static Gdiplus::SolidBrush  whiteBrush{Gdiplus::Color::White};
    static Gdiplus::SolidBrush  greenBrush{Gdiplus::Color::DarkGreen};
    static Gdiplus::SolidBrush  redBrush  {Gdiplus::Color::DarkRed};

    static Gdiplus::FontFamily  family    {L"Times New Roman"};
    static Gdiplus::Font        font      {&family, 32, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel};



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
        auto const sectorText  { ::sectorTextLocation(board,i)};

        Gdiplus::RectF  textRect{};
        window.MeasureString (number.c_str(), -1, &font, sectorText, &textRect);

        textRect.X -= textRect.Width/2;
        textRect.Y -= textRect.Height/2;

        window.DrawString(number.c_str(),-1,&font,textRect,nullptr,&blackBrush);
    }


    window.FillEllipse(&greenBrush,board.rect.outerBullseye);
    window.FillEllipse(&redBrush,  board.rect.innerBullseye);
}


void paintAimAndDarts(Gdiplus::Graphics   &window,BoardDimensions const &board)
{
    static Gdiplus::Pen         whitePen    {Gdiplus::Color::White};
    static Gdiplus::Pen         redPen      {Gdiplus::Color::Red};
    static Gdiplus::Pen         greenPen    {Gdiplus::Color::Green};

    auto radius = static_cast<int>(board.radius.outerTriple * (accuracy / 100.0));

    for(auto const &dart : Darts::darts)
    {
        auto x = static_cast<int>(mousePosition.x + radius * dart.X);
        auto y = static_cast<int>(mousePosition.y + radius * dart.Y);

        window.DrawEllipse(&whitePen, x, y, 1,1);
        window.DrawEllipse(&greenPen, x-1, y-1, 2,2);
    }


    window.DrawEllipse(&whitePen, mousePosition.x-1,      mousePosition.y-1, 2,2);
    window.DrawEllipse(&redPen,   mousePosition.x-2,      mousePosition.y-2, 4,4);
    window.DrawEllipse(&redPen,   mousePosition.x-radius, mousePosition.y-radius, radius*2,radius*2);
}


void paint(HWND h,WPARAM w, LPARAM l)
{
    auto board{ ::boardDimensions(h) };

    PAINTSTRUCT paint;
    BeginPaint(h,&paint);

    RECT        client{};
    GetClientRect(h,&client);


    Gdiplus::Bitmap     bmp{client.right-client.left, client.bottom-client.top};
    Gdiplus::Graphics   window{&bmp};    

    window.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);


    window.Clear(Gdiplus::Color::White);
    paintBoard      (window,board);
    paintAimAndDarts(window,board);
    



    Gdiplus::Graphics screen(paint.hdc);
    screen.DrawImage(&bmp, 0, 0);


    EndPaint(h,&paint);
}

