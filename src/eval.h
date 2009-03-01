#ifndef __JR_EVAL_H__
#define __JR_EVAL_H__

#include "board.h"
#include "step.h"

//functions and structures used for scoring heurisitics

class Eval
{
    public:

    short evalBoard(Board& board, unsigned char color);
    bool isWin(Board& board, unsigned char color);

    void scoreCombos(StepCombo combos[], int num, unsigned char color,
                     int bestIndex); 
};

#endif
