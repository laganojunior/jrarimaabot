#ifndef __JR_EVAL_H__
#define __JR_EVAL_H__

#include "board.h"
#include "defines.h"
#include "step.h"
#include "hash.h"
#include <vector>

//functions and structures used for scoring heurisitics

using namespace std;

class Eval
{
    public:
    Eval()
    {
        reset();
    }
    void reset();

    short evalBoard(Board& board, unsigned char color); 
    bool isWin(Board& board, unsigned char color);

    void scoreCombos(vector<StepCombo>& combos, int num, unsigned char color);
};

#endif
