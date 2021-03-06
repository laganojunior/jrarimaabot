#ifndef __JR_SEARCH_H__
#define __JR_SEARCH_H__

//game tree search manager

#include "board.h"
#include "step.h"
#include "gamehist.h"
#include "searchhist.h"
#include "hash.h"
#include "eval.h"
#include "transposition.h"
#include <string>
#include <vector>

//some limiting constants
#define SEARCH_MAX_COMBOS_PER_PLY 120

//hash bit constants
#define GAME_HIST_HASH_BITS   15

using namespace std;

class Search
{
    public:
    Search(Int64 hashTableBytes);
    ~Search();

    StepCombo iterativeDeepen(Board& board, int maxDepth, ostream& log);
    short searchNode(Board& board, int depth, int ply, short alpha,  
                              short beta, vector<string>& nodePV, 
                              StepCombo& lastMove, bool genDependent,
                              Int64 turnRefer);
    short doMoveAndSearch(Board& board, int depth, int ply, short alpha,  
                          short beta, vector<string>& nodePV,
                          StepCombo& combo, short nodeScore, Int64 turnRefer);

    void loadMoveFile(string filename, Board board);  

    unsigned int numTerminalNodes; //number of terminal nodes explored
    unsigned int numTotalNodes;    //number of all nodes explored
    unsigned int totalNodesPerSec; //rate at which nodes are explored per 
                                   //second
    unsigned int hashHits; //number of hits on the hash table for scoring
                           //purposes

    //a hash table to keep transposition data.
    TranspositionTable transTable;

    //a hash table to keep data on which positions have occurred at the
    //beginning of turns, used to make sure positions aren't repeated 3 times
    //which is a loss.
    GameHistTable gameHistTable;

    //a hash table to keep data on which positions have already occured at 
    //an earlier ply in the same turn in order to not repeat nodes.
    SearchHistTable searchHistTable;

    //keep global arrays for move generation, so that the step combo 
    //constructors are not called so much. The outer vector is indexed by
    //ply. The inner vector keeps the combos for that ply
    vector<vector<StepCombo> > combos;

    //Eval instance to score stuff
    Eval eval;
};

#endif   
