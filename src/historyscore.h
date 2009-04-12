#ifndef __JR_HISTORYSCORE_H__
#define __JR_HISTORYSCORE_H__

#include "square.h"
#include "piece.h"
#include "rawmove.h"

//A table to keep track of when moves are the best known move.
//Used for move ordering.
class HistoryScoreTable
{
    public:
    void reset();
    void scaleDown();
    unsigned short getScore(RawMove move, unsigned char color);
    void increaseScore(RawMove move, unsigned char color, int depth);
    
    private:
    //The actual score table structure. It is indexed as follows:
    //from1, to1, from2, side to move.
    unsigned short scores[NUM_SQUARES+1][NUM_SQUARES+1][NUM_SQUARES+1]
                         [MAX_COLORS]; 
};

#endif 
