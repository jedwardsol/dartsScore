
#include <cmath>
#include <numbers>
#include <random>

#include "print.h"
#include "dimensions.h"
#include "window.h"


auto genDartsSquare()
{
    static std::mt19937                                     rng{ std::random_device{}()};
    static std::uniform_real_distribution<Gdiplus::REAL>    square{-1.0,1.0};

    std::array<Gdiplus::PointF, Darts::numDarts>            darts{};

    for(auto &dart : darts)
    {
        do
        {
            dart = { square(rng),square(rng)};
        } while( std::hypot(dart.X, dart.Y) > 1);
    }

    return darts;
}


auto genDartsCircle()
{
    static std::mt19937                                     rng{ std::random_device{}()};
    static std::uniform_real_distribution<Gdiplus::REAL>    radius{0.0,1.0};
    static std::uniform_int                                 angle{0, 360};

    std::array<Gdiplus::PointF, Darts::numDarts>            darts{};

    for(auto &dart : darts)
    {
        auto distance = radius(rng);
        auto theta    = radians(angle(rng));

        dart= {static_cast<Gdiplus::REAL>(distance*cos(theta)), 
               static_cast<Gdiplus::REAL>(distance*sin(theta))};
    }

    return darts;
}



auto genDartsLowerCircle()
{
    static std::mt19937                                     rng{ std::random_device{}()};
    static std::uniform_real_distribution<Gdiplus::REAL>    radius{0.0,1.0};
    static std::uniform_int                                 angle{0, 180};

    std::array<Gdiplus::PointF, Darts::numDarts>            darts{};

    for(auto &dart : darts)
    {
        auto distance = radius(rng);
        auto theta    = radians(angle(rng));

        dart= {static_cast<Gdiplus::REAL>(distance*cos(theta)), 
               static_cast<Gdiplus::REAL>(distance*sin(theta))};
    }

    return darts;
}


auto genDartsRealistic()
{
    static std::mt19937                                     rng{ std::random_device{}()};
    static std::uniform_real_distribution<Gdiplus::REAL>    radius{0.0,1.0};
    static std::normal_distribution<double>                 angle{90, 60};          // mainly below the aim-point

    std::array<Gdiplus::PointF, Darts::numDarts>            darts{};

    for(auto &dart : darts)
    {
        auto distance = radius(rng);
        auto theta    = radians( angle(rng) );

        dart= {static_cast<Gdiplus::REAL>(distance*cos(theta)), 
               static_cast<Gdiplus::REAL>(distance*sin(theta))};
    }

    return darts;
}



std::array<Gdiplus::PointF, Darts::numDarts> Darts::darts{genDartsRealistic()};           // -1.0 -> 1.0


DartHit scoreFromPoint(BoardDimensions const &board, int x,int y)
{
    DartHit result{0,1};

    auto distance = std::hypot( x, y);

    if(distance > board.radius.outerDouble)
    {
    }
    else if(distance < board.radius.innerBullseye)
    {
        result.score=50;
    }
    else if(distance < board.radius.outerBullseye)
    {
        result.score=25;
    }
    else
    {
        if(   distance < board.radius.outerTriple
           && distance > board.radius.innerTriple)
        {
            result.multiplier=3;
        }
        else if(   distance < board.radius.outerDouble
                && distance > board.radius.innerDouble)
        {
            result.multiplier=2;
        }

        result.score=2;

        auto theta = static_cast<int>(degrees(std::atan2( y , x)));

        if(theta < 0)
        {
            theta = 360 + theta;
        }

        auto sector = (theta - Board::sector0Start) / Board::sectorWidth;

        sector = (sector+20) % 20;


        result.score = Board::sectorScore[sector];
    }


    return result;
}
