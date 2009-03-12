#ifndef __JR_EVAL_H__
#define __JR_EVAL_H__

#include "board.h"
#include "defines.h"
#include "step.h"
#include "hash.h"
#include <vector>

//functions and structures used for scoring heurisitics

using namespace std;

class KillerMove
{
    public:
    unsigned char movetype;
    unsigned char from1;
    unsigned char from2;
    unsigned char to1;
    unsigned int score;

    KillerMove& operator=(StepCombo& combo)
    {
        if (combo.stepCost == 1)
        {
            movetype = SCORE_MOVE_1STEP;
            from1 = combo.getFrom1();
            to1 = combo.getTo1();
            return *this;
        }
        
        if (combo.stepCost == 2)
        {
            movetype = SCORE_MOVE_2STEP;
            from1 = combo.getFrom1();
            to1   = combo.getTo1();
            from2 = combo.getFrom2();
            return *this;
        }
    }

    bool operator==(StepCombo& combo)
    {
        if (combo.stepCost == 1 && movetype == SCORE_MOVE_1STEP
            && from1 == combo.getFrom1()
            && to1 == combo.getTo1())
        {
            return true;
        }

        if (combo.stepCost == 2 && movetype == SCORE_MOVE_2STEP
            && from1 == combo.getFrom1()
            && to1   == combo.getTo1()
            && from2 == combo.getFrom2())
        {
            return true;
        }

        return false;
    }
};

class Eval
{
    public:
    void reset();

    short evalBoard(Board& board, unsigned char color); 
    bool isWin(Board& board, unsigned char color);

    void scoreCombos(StepCombo combos[], int num, unsigned char color); 

    vector<KillerMove>& getKillerMoves(unsigned int ply);
    void  addKillerMove(StepCombo& combo, unsigned int ply);        

    void increaseHistoryScore(unsigned char from1, unsigned char to1,
                              unsigned char from2, unsigned char color,
                              unsigned char depth);
    unsigned char getHistoryScore(unsigned char from1, unsigned char to1,
                                  unsigned char from2, unsigned char color);
	
    //array used to keep track of moves that tended to create cutoffs
    //at certain plys. Hopefully, the same moves will create cutoffs
    //at other nodes at similar depths
    vector<KillerMove> killermoves[SEARCH_MAX_DEPTH];

    //array used to keep counts of when a certain move turned out to be
    //the best move for that player. It is indexed from1, to1, from2, color
    unsigned short historyScore[NUM_SQUARES+1][NUM_SQUARES+1]
                               [NUM_SQUARES+1][MAX_COLORS];
};

#endif
