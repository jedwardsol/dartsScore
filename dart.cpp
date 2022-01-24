
#include <cmath>
#include <numbers>

#include "print.h"
#include "dimensions.h"
#include "window.h"




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

        auto angle    = static_cast<int>(degrees(std::atan2( y , x)));

        if(angle < 0)
        {
            angle = 360 + angle;
        }

        auto sector = (angle - Board::sector0Start) / Board::sectorWidth;

        sector = (sector+20) % 20;


        result.score = Board::sectorScore[sector];
    }


    return result;
}
