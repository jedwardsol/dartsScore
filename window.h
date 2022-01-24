#pragma once

#include "dimensions.h"

void createWindow();
void windowMessageLoop();

void paint(HWND h,  WPARAM w, LPARAM l);


struct DartHit
{
    int score;
    int multiplier;
};

DartHit scoreFromPoint(BoardDimensions const &board, int x,int y);




extern POINT               mousePosition;   // window coordinates
extern int                 accuracy;        // 2=high, 102 =low        
