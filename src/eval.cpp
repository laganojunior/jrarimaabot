#include "eval.h"
#include "board.h"
#include "int64.h"

//////////////////////////////////////////////////////////////////////////////
//Clear all stored data from a previous search such as history score data.
//////////////////////////////////////////////////////////////////////////////
void Eval :: reset()
{
    histTable.reset();
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
    int material = __builtin_popcountll(board.pieces[GOLD][ELEPHANT]) * 1200
                 + __builtin_popcountll(board.pieces[GOLD][CAMEL]) * 700 
                 + __builtin_popcountll(board.pieces[GOLD][HORSE]) * 500 
                 + __builtin_popcountll(board.pieces[GOLD][DOG]) * 300 
                 + __builtin_popcountll(board.pieces[GOLD][CAT]) * 200
                 + __builtin_popcountll(board.pieces[GOLD][RABBIT]) * 100
                 - __builtin_popcountll(board.pieces[SILVER][ELEPHANT]) * 1200
                 - __builtin_popcountll(board.pieces[SILVER][CAMEL]) * 700 
                 - __builtin_popcountll(board.pieces[SILVER][HORSE]) * 500 
                 - __builtin_popcountll(board.pieces[SILVER][DOG]) * 300 
                 - __builtin_popcountll(board.pieces[SILVER][CAT]) * 200
                 - __builtin_popcountll(board.pieces[SILVER][RABBIT]) * 100;

    //rabbit advancement
    int advance = __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(0)) * 25000
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(1)) * 50
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(2)) * 40
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(3)) * 20
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(4)) * 15
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(5)) * 10
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(6)) * 5
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(7)) * 20
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(7)) * 25000
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
void Eval :: scoreCombos(list<StepCombo>& combos, unsigned char color)
{
    for (list<StepCombo> :: iterator i = combos.begin(); 
         i != combos.end(); ++i)
    {
        i->score = 0;

        //give moves some score based on captures, note that pieces are
        //numerically ordered with ELEPHANT being 0, and RABBIT being 5, so
        //to give a sensical score in respect to type, value should reversed
        //in respect to these numbers
        if (i->piece1IsCaptured())
        {
            unsigned char piece = i->getPiece1();
            if (colorOfPiece(piece) != color)
                i->score += 100 * (MAX_TYPES - typeOfPiece(piece));
            else
                i->score -= 100 * (MAX_TYPES - typeOfPiece(piece));
        }
        
        if (i->piece2IsCaptured())
        {
            unsigned char piece = i->getPiece2();
            if (colorOfPiece(piece) != color)
                i->score += 100 * (MAX_TYPES - typeOfPiece(piece));
            else
                i->score -= 100 * (MAX_TYPES - typeOfPiece(piece));
        }

        //give moves some score based on push/pulling moves
        if (i->stepCost > 1 && i->getFrom1() != ILLEGAL_SQUARE)
            i->score += 25;

        //give moves a boost according to their history score
        i->score += histTable.getScore(i->getRawMove(), color);
    }
}
