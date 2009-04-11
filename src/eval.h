#ifndef __JR_EVAL_H__
#define __JR_EVAL_H__

#include "board.h"
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
    }
    void reset();

    short evalBoard(Board& board, unsigned char color); 
    bool isWin(Board& board, unsigned char color);

    void scoreCombos(vector<StepCombo>& combos, unsigned char color);

    void loadWeights(string filename);
    void saveWeights(string filename);

    HistoryScoreTable histTable;

    //evaluation weights////////////////////////

    //static material weights for types and number of that type on the 
    //board
    short materialWeights[MAX_TYPES][8];

    //static position weights for square, piece pairs
    short posWeights[MAX_COLORS][MAX_TYPES][NUM_SQUARES];
};

#endif
