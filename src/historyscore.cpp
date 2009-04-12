#include "historyscore.h"
#include "rawmove.h"

//////////////////////////////////////////////////////////////////////////////
//Resets all score entries to 0.
//////////////////////////////////////////////////////////////////////////////
void HistoryScoreTable :: reset()
{
    for (int f1 = 0; f1 < NUM_SQUARES + 1; f1 ++)
    {
        for (int t1 = 0; t1 < NUM_SQUARES + 1; t1 ++)        
        {
            for (int f2 = 0; f2 < NUM_SQUARES + 1; f2 ++)
            {
                for (int c = 0; c < MAX_COLORS; c++)
                {
                    scores[f1][t1][f2][c] = 0;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//Scales down all the entries by a factor of 2^7.
//////////////////////////////////////////////////////////////////////////////
void HistoryScoreTable :: scaleDown()
{
    for (int f1 = 0; f1 < NUM_SQUARES + 1; f1 ++)
    {
        for (int t1 = 0; t1 < NUM_SQUARES + 1; t1 ++)        
        {
            for (int f2 = 0; f2 < NUM_SQUARES + 1; f2 ++)
            {
                for (int c = 0; c < MAX_COLORS; c++)
                {
                    scores[f1][t1][f2][c] >>= 7;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//Returns the score for that move and for that color to move
//////////////////////////////////////////////////////////////////////////////
unsigned short HistoryScoreTable :: getScore(RawMove move, 
                                             unsigned char color)
{
    return scores[move.from1][move.to1][move.from2][color];
}

//////////////////////////////////////////////////////////////////////////////
//Increases the score for that move and that color to move by an amount 
//dependent on the depth of the node (currently depth ^ 3)
//////////////////////////////////////////////////////////////////////////////
void HistoryScoreTable :: increaseScore(RawMove move, unsigned char color, 
                                        int depth)
{
    //Check if the score is really high, and if so, scale down the scores
    if (scores[move.from1][move.to1][move.from2][color] > 20000)
        scaleDown();

    //Increase the score
    scores[move.from1][move.to1][move.from2][color] += depth * depth * depth;
}
