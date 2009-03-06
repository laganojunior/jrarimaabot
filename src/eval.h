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

    void scoreCombos(StepCombo combos[], int num, unsigned char color); 

    //array used to keep track of whenever a piece is moved from one
    //place to another in a certain ply, if the move causes a beta cut-off.
    //used for move scoring, as it might that in similar plies, similar
    //moves will also be moves that terminate that tree.
    unsigned char killermove[NUM_SQUARES][NUM_SQUARES][MAX_COLORS];
};

#endif
