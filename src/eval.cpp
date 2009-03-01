#include "eval.h"
#include "board.h"
#include "int64.h"

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
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(1)) * 100
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(2)) * 10
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(3)) * 9
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(4)) * 8
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(5)) * 7
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(6)) * 6
                + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getRow(7)) * 50
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(7)) * 30000
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(6)) * 100
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(5)) * 10
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(4)) * 9
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(3)) * 8
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(2)) * 7
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(1)) * 6
                - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getRow(0)) * 50;

    //keeps center control with the strong board.pieces
    int strongCenter = __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(0)) * 50
                     + __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(1)) * 40
                     + __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(2)) * 30
                     + __builtin_popcountll(board.pieces[GOLD][ELEPHANT] & getCenterRing(3)) * 20
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(0)) * 30
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(1)) * 20
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(2)) * -50
                     + __builtin_popcountll(board.pieces[GOLD][CAMEL] & getCenterRing(3)) * -100
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(0)) * 40
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(1)) * 30
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(2)) * 20
                     + __builtin_popcountll(board.pieces[GOLD][HORSE] & getCenterRing(3)) * 30
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(0)) * 50
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(1)) * 40
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(2)) * 30
                     - __builtin_popcountll(board.pieces[SILVER][ELEPHANT] & getCenterRing(3)) * 20
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(0)) * 40
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(1)) * 20
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(2)) * -50
                     - __builtin_popcountll(board.pieces[SILVER][CAMEL] & getCenterRing(3)) * -100
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(0)) * 40
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(1)) * 30
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(2)) * 20
                     - __builtin_popcountll(board.pieces[SILVER][HORSE] & getCenterRing(3)) * 30;

    //keep rabbits away from center 
    int rabbitCenterAvoid = __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(0)) * -100
                          + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(1)) * -75
                          + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(2)) * -25
                          + __builtin_popcountll(board.pieces[GOLD][RABBIT] & getCenterRing(3)) * 25;
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(0)) * -100
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(1)) * -75
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(2)) * -25
                          - __builtin_popcountll(board.pieces[SILVER][RABBIT] & getCenterRing(3)) * 25;

    //keep pieces away from traps
    int trapAvoid = __builtin_popcountll(board.pieces[GOLD][CAMEL] & (getTraps() | getTrapNeighbors()))    * -300
                  + __builtin_popcountll(board.pieces[GOLD][HORSE] & (getTraps() | getTrapNeighbors()))    * -100
                  + __builtin_popcountll(board.pieces[GOLD][DOG] & (getTraps() | getTrapNeighbors()))      * -50
                  + __builtin_popcountll(board.pieces[GOLD][CAT] & (getTraps() | getTrapNeighbors()))      * -25
                  + __builtin_popcountll(board.pieces[GOLD][RABBIT] & (getTraps() | getTrapNeighbors()))   * -50
                  - __builtin_popcountll(board.pieces[SILVER][CAMEL] & (getTraps() | getTrapNeighbors()))  * -300 
                  - __builtin_popcountll(board.pieces[SILVER][HORSE] & (getTraps() | getTrapNeighbors()))  * -100
                  - __builtin_popcountll(board.pieces[SILVER][DOG] & (getTraps() | getTrapNeighbors()))    * -50
                  - __builtin_popcountll(board.pieces[SILVER][CAT] & (getTraps() | getTrapNeighbors()))    * -25
                  - __builtin_popcountll(board.pieces[SILVER][RABBIT] & (getTraps() | getTrapNeighbors())) * -50;

    //guard local traps with weaker pieces
    int trapGuard = __builtin_popcountll(board.pieces[GOLD][DOG] & (getTrapNeighbors()) & 
                      (getRow(5) | getRow(6) | getRow(7) )) * 100
                  + __builtin_popcountll(board.pieces[GOLD][CAT] & (getTrapNeighbors()) & 
                      (getRow(5) | getRow(6) | getRow(7) )) * 75
                  - __builtin_popcountll(board.pieces[SILVER][DOG] & (getTrapNeighbors()) & 
                      (getRow(1) | getRow(2) | getRow(3) )) * 100
                  - __builtin_popcountll(board.pieces[SILVER][CAT] & (getTrapNeighbors()) & 
                      (getRow(1) | getRow(2) | getRow(3) )) * 70;

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
