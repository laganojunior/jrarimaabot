#include "killer.h"
#include "rawmove.h"
#include <vector>
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//Add a killer move for a specified depth or increase the score for that move
//if already present. If the table is already full, then the one with the 
//lowest score is replaced.
//////////////////////////////////////////////////////////////////////////////
void KillerMoveTable :: addKillerMove(unsigned int ply, RawMove killer)
{
    //Check to make sure the killer move table has entries up to that ply
    if ((int)killers.size() - 1 < (int)ply)
    {
        while (killers.size() != ply + 1)
        {
            vector<KillerMove> vec;
            killers.push_back(vec);
        }
    }

    //Search if the entry is already present, and just add points to it
    bool found = false;
    int lowestIndex = 0;
    for (int i = 0; i < killers[ply].size(); i++)
    {
        if (killers[ply][i].score < killers[ply][lowestIndex].score)
        {
            lowestIndex = i;
        }

        if (killers[ply][i].move == killer)
        {
            killers[ply][i].score += 1;
            found = true;
            break;
        }
    }

    //if it wasn't found, then add it. If the array is already full, then
    //just overwrite the one with the lowest score
    if (!found)
    {
        if (killers[ply].size() < KILLERMOVETABLE_NUMKILLERSPERPLY)
        {
            KillerMove newKiller;
            newKiller.move = killer;   
            newKiller.score = 1;
            killers[ply].push_back(newKiller);
        }
        else
        {
            killers[ply][lowestIndex].move = killer;
            killers[ply][lowestIndex].score = 1;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//Return a list of killer moves for this depth sorted by descending cutoff
//score
//////////////////////////////////////////////////////////////////////////////
vector<RawMove> KillerMoveTable :: getKillerMoves(unsigned int ply)
{
    //Check to make sure the killer move table has entries up to that ply
    if ((int)killers.size() - 1 < (int)ply)
    {
        while (killers.size() != ply + 1)
        {       
            vector<KillerMove> vec;
            killers.push_back(vec);
        }
    }

    vector<RawMove> killerMoves;
    
    //get the list of killer moves without the score
    for (int i = 0; i < killers[ply].size(); i++)
    {
        killerMoves.push_back(killers[ply][i].move);
    }

    return killerMoves;
}

//////////////////////////////////////////////////////////////////////////////
//Reset the list of killer moves for that ply to an empty list
//////////////////////////////////////////////////////////////////////////////
void KillerMoveTable :: resetKillerMoves(unsigned int ply)
{
    killers[ply].resize(0);
}

//////////////////////////////////////////////////////////////////////////////
//Resets the list of killer moves for every ply to be empty
//////////////////////////////////////////////////////////////////////////////
void KillerMoveTable :: reset()
{
    killers.resize(0);
}
