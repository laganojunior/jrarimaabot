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

    void increaseHistoryScore(unsigned char from1, unsigned char to1,
                              unsigned char from2, unsigned char color,
                              unsigned char depth);
    unsigned short getHistoryScore(unsigned char from1, unsigned char to1,
                                  unsigned char from2, unsigned char color);

    //array used to keep counts of when a certain move turned out to be
    //the best move for that player. It is indexed from1, to1, from2, color
    unsigned short historyScore[NUM_SQUARES+1][NUM_SQUARES+1]
                               [NUM_SQUARES+1][MAX_COLORS];
};

#endif
