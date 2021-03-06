#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include <array>
#include <numbers>
#include <cmath>




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


constexpr std::array<int,20>  sectorScore{6,10,15,2,17,3,19,7,16,8,11,14,9,12,5,20,1,18,4,13};

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


namespace Darts          // TODO : Gdiplus leaked horribly into here.
{

constexpr int  numDarts   {500};

extern std::array<Gdiplus::PointF, numDarts> darts;           // -1.0 -> 1.0

}


auto inline   radians(double degrees)
{
    return degrees * 2 * std::numbers::pi / 360;
}


auto inline   degrees(double radians)
{
    return 360 * radians / (2 * std::numbers::pi );
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


BoardDimensions boardDimensions(HWND h);



struct RadiusDimensions // pixels
{
    Gdiplus::REAL   theta;

    Gdiplus::Point  outerDouble;   
    Gdiplus::Point  innerDouble;
    Gdiplus::Point  outerTriple;
    Gdiplus::Point  innerTriple;
    Gdiplus::Point  center;
};


RadiusDimensions radiusDimensions(BoardDimensions const &board, int sectorNumber);

Gdiplus::PointF sectorTextLocation(BoardDimensions const &board, int sectorNumber);
