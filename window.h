#pragma once

#include <vector>
#include "dimensions.h"

void createWindow();
void windowMessageLoop();

void paint(HWND h,  WPARAM w, LPARAM l);


struct DartHit
{
    int score;
    int multiplier;
};

DartHit scoreFromPoint(BoardDimensions const &board, int x,int y);     // board coordinates




extern POINT                        mousePosition;   // client coordinates
extern int                          accuracy;        // 2=high, 102 =low        
extern POINT                        bestPoint;
