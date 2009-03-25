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
#define SEARCH_MAX_DEPTH 20
#define SEARCH_MAX_COMBOS_PER_PLY 120

//hash bit constants
#define SEARCH_GAME_HIST_HASH_BITS 20
#define SEARCH_SEARCH_HIST_HASH_BITS 20

using namespace std;

class Search
{
    public:
    Search(int numScoreHashBits);
    ~Search();

    StepCombo searchRoot(Board& board, int depth);
    short searchNode(Board& board, int depth, short alpha,  
                              short beta, vector<string>& nodePV);
    short doMoveAndSearch(Board& board, int depth, short alpha,  
                          short beta, vector<string>& nodePV,
                          StepCombo& combo);

    void loadMoveFile(string filename, Board board);  

    void addSearchHistory(Board& board);
    void removeSearchHistory(Board& board);
    bool  hasOccured(Board& board);

    unsigned int getNextBestCombo(unsigned int ply);

    string getStatString();
    string getShortStatString();

    unsigned int numTerminalNodes; //number of terminal nodes explored
    unsigned int numTotalNodes;    //number of all nodes explored
    unsigned int millis;           //milliseconds taken to do the search
    unsigned int totalNodesPerSec; //rate at which nodes are explored per 
                                   //second
    unsigned int maxDepth; //depth the search was performed to
    unsigned int hashHits; //number of hits on the hash table for scoring
                           //purposes
    unsigned int collisions; //number of times different positions were 
                             //detected to have the same hash for scores
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

    //keep pre-made arrays for storing moves at each ply, so that the
    //constructors/deconstructors aren't called so much
    StepCombo combos[SEARCH_MAX_DEPTH][SEARCH_MAX_COMBOS_PER_PLY];  
    unsigned int numCombos[SEARCH_MAX_DEPTH];

    //Eval instance to score stuff
    Eval eval;
};

#endif   
