#ifndef __JR_EVAL_H__
#define __JR_EVAL_H__

#include "board.h"
#include "step.h"
#include "hash.h"
#include <vector>

//functions and structures used for scoring heurisitics

#define SEARCH_MAX_DEPTH 20

#define EVAL_KILLER_MOVES_PER_PLY 2

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
            from1 = combo.steps[0].getFrom();
            to1 = combo.steps[0].getTo();
            return *this;
        }
        
        if (combo.stepCost == 2)
        {
            movetype = SCORE_MOVE_2STEP;
            from1 = combo.steps[0].getFrom();
            to1 = combo.steps[0].getTo();

            //note that second step can denote a capture, so the actual
            //second moving step can be afterward
            unsigned int secondMoveIndex = 1;

            if (combo.steps[1].isCapture())
                secondMoveIndex = 2;

            from2 = combo.steps[secondMoveIndex].getFrom();
            return *this;
        }
    }

    bool operator==(StepCombo& combo)
    {
        if (combo.stepCost == 1 && movetype == SCORE_MOVE_1STEP
            && from1 == combo.steps[0].getFrom() 
            && to1 == combo.steps[0].getTo())
        {
            return true;
        }

        if (combo.stepCost == 2 && movetype == SCORE_MOVE_2STEP
            && from1 == combo.steps[0].getFrom() 
            && to1 == combo.steps[0].getTo())
        {
            //note that second step can denote a capture, so the actual
            //second moving step can be afterward
            unsigned int secondMoveIndex = 1;

            if (combo.steps[1].isCapture())
                secondMoveIndex = 2;

            if (from2 == combo.steps[secondMoveIndex].getFrom())
                return true;
        }

        return false;
    }
};

class Eval
{
    public:

    short evalBoard(Board& board, unsigned char color);
    bool isWin(Board& board, unsigned char color);

    void scoreCombos(StepCombo combos[], int num, unsigned char color); 

    vector<KillerMove>& getKillerMoves(unsigned int ply);
    void  addKillerMove(StepCombo& combo, unsigned int ply);        

    //array used to keep track of moves that tended to create cutoffs
    //at certain plys. Hopefully, the same moves will create cutoffs
    //at other nodes at similar depths
    vector<KillerMove> killermoves[SEARCH_MAX_DEPTH];

    
};

#endif
