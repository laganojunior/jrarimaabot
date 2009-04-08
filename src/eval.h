#ifndef __JR_EVAL_H__
#define __JR_EVAL_H__

#include "board.h"
#include "defines.h"
#include "step.h"
#include "hash.h"
#include "historyscore.h"
#include <list>

//functions and structures used for scoring heurisitics

using namespace std;

class Eval
{
    public:
    Eval()
    {
        reset();
        loadWeights();
    }
    void reset();

    short evalBoard(Board& board, unsigned char color); 
    bool isWin(Board& board, unsigned char color);

    void scoreCombos(list<StepCombo>& combos, unsigned char color);

    void loadWeights();
    void saveWeights();
    void loadPositionWeights(string filename);
    void savePositionWeights(string filename);

    HistoryScoreTable histTable;

    //evaluation weights////////////////////////

    //static position weights for square, piece pairs
    short posWeights[MAX_COLORS][MAX_TYPES][NUM_SQUARES];
};

#endif
