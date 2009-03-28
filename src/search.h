#ifndef __JR_SEARCH_H__
#define __JR_SEARCH_H__

//game tree search manager

#include "board.h"
#include "step.h"
#include "gamehist.h"
#include "hash.h"
#include "eval.h"
#include "killer.h"
#include "transposition.h"
#include <string>
#include <vector>

//some limiting constants
#define SEARCH_MAX_COMBOS_PER_PLY 120

//hash bit constants
#define SEARCH_SEARCH_HIST_HASH_BITS 20

using namespace std;

class Search
{
    public:
    Search(int numScoreHashBits);
    ~Search();

    StepCombo iterativeDeepen(Board& board, int maxDepth, ostream& log);
    void searchRoot(Board& board, int depth);
    short searchNode(Board& board, int depth, int ply, short alpha,  
                              short beta, vector<string>& nodePV);
    short doMoveAndSearch(Board& board, int depth, int ply, short alpha,  
                          short beta, vector<string>& nodePV,
                          StepCombo& combo);

    void loadMoveFile(string filename, Board board);  

    void addSearchHistory(Board& board);
    void removeSearchHistory(Board& board);
    bool  hasOccured(Board& board);

    StepCombo getNextBestComboAndRemove(vector<StepCombo>& list, 
                                        unsigned int num);

    unsigned int numTerminalNodes; //number of terminal nodes explored
    unsigned int numTotalNodes;    //number of all nodes explored
    unsigned int totalNodesPerSec; //rate at which nodes are explored per 
                                   //second
    unsigned int hashHits; //number of hits on the hash table for scoring
                           //purposes
    short score;  //the solved (in respect to the game tree) of the current
                  //position

    vector<string> pv; //the principal variation

    //a hash table to keep transposition data.
    TranspositionTable transTable;

    //a hash table to keep data on which positions have occurred at the
    //beginning of turns, used to make sure positions aren't repeated 3 times
    //which is a loss.
    GameHistTable gameHistTable;

    //a hash table to keep data on which positions are in this search path
    //down the tree. This is used to eliminate moves that just lead to
    //repeating positions
    HashTable<SearchHistEntry> searchHist;
    Int64 searchHistHashMask;

    //Killer move table
    KillerMoveTable killerTable;

    //keep global arrays for move generation, so that the step combo 
    //constructors are not called so much. The outer vector is indexed by
    //ply. The inner vector keeps the combos for that ply
    vector<vector<StepCombo> > combos;

    //Eval instance to score stuff
    Eval eval;
};

#endif   
