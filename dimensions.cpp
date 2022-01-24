
#include "dimensions.h"

BoardDimensions boardDimensions(HWND h)
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



RadiusDimensions radiusDimensions(BoardDimensions  const &board, int sectorNumber)
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



Gdiplus::PointF sectorTextLocation(BoardDimensions  const &board, int sectorNumber)
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
