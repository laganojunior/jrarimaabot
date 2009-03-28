#ifndef __JR_KILLER_H__
#define __JR_KILLER_H__

#include "rawmove.h"
#include <vector>

using namespace std;

#define KILLERMOVETABLE_NUMKILLERSPERPLY 2

//A killer move entry, which is just a raw move combined with a score to
//represent the number of cutoffs this move produced.
struct KillerMove
{
    RawMove move;
    unsigned int score;
};

//A Table to keep killer moves, which are moves that caused cutoffs for that
//ply. Hopefully such moves are still good moves in other places in the
//search tree for that depth, so searching them first causes less nodes
//to be searched overall.
class KillerMoveTable
{
    public:
    void addKillerMove(unsigned int ply, RawMove killer);
    vector<RawMove> getKillerMoves(unsigned int ply);
    void resetKillerMoves(unsigned int ply);
    void reset();
        
    private:
    //a two dimension array to keep the killer moves. The outer vector
    //is indexed by ply, the inner vector is the moves for that ply.
    vector<vector<KillerMove> > killers;
};

//////////////////////////////////////////////////////////////////////////////
//A function to compare killer moves in order to sort them by score. Note
//that killer moves are sorted by descending score order, so move1 < move2
//iff move1's score > move2's score
//////////////////////////////////////////////////////////////////////////////
inline bool killerMoveCompare(const KillerMove& move1, 
                              const KillerMove& move2)
{
    return move1.score > move2.score;
}

#endif
