#include "eval.h"
#include "board.h"
#include "int64.h"

//////////////////////////////////////////////////////////////////////////////
//Clear all stored data from a previous search such as killer move data
//and history score data
//////////////////////////////////////////////////////////////////////////////
void Eval :: reset()
{
    //reset history scores
    memset(historyScore, 0, sizeof(unsigned short) * (NUM_SQUARES+1)
                          * (NUM_SQUARES+1) * (NUM_SQUARES+1) * MAX_COLORS);
}

//////////////////////////////////////////////////////////////////////////////
//Returns a static evaluation score of the board in the
//perspective of the player given
//////////////////////////////////////////////////////////////////////////////
short Eval :: evalBoard(Board& board, unsigned char color)
{
    //do scores assuming GOLD's perspective. if it is SILVER that is really
    //desired, then just negate at the end as this is a zero sum game

    int score;
    
    //material
    int material = __builtin_popcountll(board.pieces[GOLD][ELEPHANT]) * 2000
                 + __builtin_popcountll(board.pieces[GOLD][CAMEL]) * 700 
                 + __builtin_popcountll(board.pieces[GOLD][HORSE]) * 500 
                 + __builtin_popcountll(board.pieces[GOLD][DOG]) * 300 
                 + __builtin_popcountll(board.pieces[GOLD][CAT]) * 200
                 + __builtin_popcountll(board.pieces[GOLD][RABBIT]) * 100
                 - __builtin_popcountll(board.pieces[SILVER][ELEPHANT]) * 2000
                 - __builtin_popcountll(board.pieces[SILVER][CAMEL]) * 700 
                 - __builtin_popcountll(board.pieces[SILVER][HORSE]) * 500 
                 - __builtin_popcountll(board.pieces[SILVER][DOG]) * 300 
                 - __builtin_popcountll(board.pieces[SILVER][CAT]) * 200
                 - __builtin_popcountll(board.pieces[SILVER][RABBIT]) * 100;

    //rabbit advancement
    int advance = __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(0)) * 30000
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(1)) * 50
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(2)) * 40
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(3)) * 20
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(4)) * 15
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(5)) * 10
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(6)) * 5
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(7)) * 20
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(7)) * 30000
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(6)) * 50
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(5)) * 40
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(4)) * 20
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(3)) * 15
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(2)) * 10
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(1)) * 5
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(0)) * 20;

    //keeps center control with the strong board.pieces
    int strongCenter = __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(0)) * 25
                     + __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(1)) * 15
                     + __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(2)) * 10
                     + __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(3)) * 0
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(0)) * 10
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(1)) * 10
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(2)) * 0
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(3)) * -20
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(0)) * 5
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(1)) * 4
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(2)) * 3
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(3)) * 3
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(0)) * 25
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(1)) * 15
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(2)) * 10
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(3)) * 0
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(0)) * 10
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(1)) * 10
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(2)) * 0
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(3)) * -20
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(0)) * 5
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(1)) * 4
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(2)) * 3
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(3)) * 3;

    //keep rabbits away from center 
    int rabbitCenterAvoid = __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(0)) * -10
                          + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(1)) * -5
                          + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(2)) * -3
                          + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(3)) * 10
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(0)) * -10
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(1)) * -5
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(2)) * -3
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(3)) * 10;

    //keep pieces away from traps
    int trapAvoid = __builtin_popcountll(board.pieces[GOLD][CAMEL] & (getTraps() | getTrapNeighbors()))    * -20
                  + __builtin_popcountll(board.pieces[GOLD][HORSE] & (getTraps() | getTrapNeighbors()))    * -10
                  + __builtin_popcountll(board.pieces[GOLD][DOG] & (getTraps() | getTrapNeighbors()))      * -5
                  + __builtin_popcountll(board.pieces[GOLD][CAT] & (getTraps() | getTrapNeighbors()))      * -3
                  + __builtin_popcountll(board.pieces[GOLD][RABBIT] & (getTraps() | getTrapNeighbors()))   * -5
                  - __builtin_popcountll(board.pieces[SILVER][CAMEL] & (getTraps() | getTrapNeighbors()))  * -20 
                  - __builtin_popcountll(board.pieces[SILVER][HORSE] & (getTraps() | getTrapNeighbors()))  * -10
                  - __builtin_popcountll(board.pieces[SILVER][DOG] & (getTraps() | getTrapNeighbors()))    * -5
                  - __builtin_popcountll(board.pieces[SILVER][CAT] & (getTraps() | getTrapNeighbors()))    * -3
                  - __builtin_popcountll(board.pieces[SILVER][RABBIT] & (getTraps() | getTrapNeighbors())) * -5;

    //guard local traps with weaker pieces
    int trapGuard = __builtin_popcountll(board.pieces[GOLD][DOG] & (getTrapNeighbors()) & 
                      (getRow(5) | getRow(6) | getRow(7) )) * 20
                  + __builtin_popcountll(board.pieces[GOLD][CAT] & (getTrapNeighbors()) & 
                      (getRow(5) | getRow(6) | getRow(7) )) * 10
                  - __builtin_popcountll(board.pieces[SILVER][DOG] & (getTrapNeighbors()) & 
                      (getRow(1) | getRow(2) | getRow(3) )) * 20
                  - __builtin_popcountll(board.pieces[SILVER][CAT] & (getTrapNeighbors()) & 
                      (getRow(1) | getRow(2) | getRow(3) )) * 10;

    score = material + advance + strongCenter + rabbitCenterAvoid + trapAvoid + trapGuard;

    if (color == GOLD)
        return score;
    else
        return -score;
}

//////////////////////////////////////////////////////////////////////////////
//returns true if the position is a win for the color specified.
//////////////////////////////////////////////////////////////////////////////
bool Eval :: isWin(Board& board, unsigned char color)
{
    if (color == GOLD)
        return board.pieces[GOLD][RABBIT] & getRow(0);
    else
        return board.pieces[SILVER][RABBIT] & getRow(7);
}

//////////////////////////////////////////////////////////////////////////////
//gives the combo in the array some heurisitic scores to see which order the
//moves should be considered in.
//////////////////////////////////////////////////////////////////////////////
void Eval :: scoreCombos(vector<StepCombo>& combos, int num, 
                         unsigned char color)
{
    for (int i = 0; i < num; ++i)
    {
        combos[i].score = 0;

        //give moves some score based on captures, note that pieces are
        //numerically ordered with ELEPHANT being 0, and RABBIT being 5, so
        //to give a sensical score in respect to type, value should reversed
        //in respect to these numbers

        if (combos[i].hasFriendlyCapture)
        {
            combos[i].score -= 1000 * 
                               (MAX_TYPES - combos[i].friendlyCaptureType);
        } 
        
        if (combos[i].hasEnemyCapture)
        {
            combos[i].score += 1000 * 
                               (MAX_TYPES - combos[i].enemyCaptureType);
        }

        //give moves some score based on history heuristic
        combos[i].score += getHistoryScore(combos[i].getFrom1(),
                                                combos[i].getTo1(),
                                                combos[i].getFrom2(),
                                                color);

        //give moves some score based on push/pulling moves
        if (combos[i].stepCost > 1 && combos[i].getFrom1() != ILLEGAL_SQUARE)
            combos[i].score += 250;
    }
}

//////////////////////////////////////////////////////////////////////////////
//Increase the history score of a certain move that was best at a certain 
//depth
//////////////////////////////////////////////////////////////////////////////
void Eval :: increaseHistoryScore(unsigned char from1, unsigned char to1,
                                  unsigned char from2, unsigned char color,
                                  unsigned char depth)
{
    //Check if there would be a overflow over the maximum if the history
    //score is increased for this node before adding to the score
    if ((int)historyScore[from1][to1][from2][color] 
        + (int)depth < 65535)
    {
        //Increase the score for this move depending on its depth
        historyScore[from1][to1][from2][color] += depth;
    }
    else
        historyScore[from1][to1][from2][color] = 65535;
}

//////////////////////////////////////////////////////////////////////////////
//Get the history score for a particular move and player
//////////////////////////////////////////////////////////////////////////////
unsigned short Eval :: getHistoryScore(unsigned char from1, unsigned char to1,
             unsigned char from2, unsigned char color)
{
    return historyScore[from1][to1][from2][color];
}
