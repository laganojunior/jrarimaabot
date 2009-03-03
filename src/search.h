#ifndef __JR_SEARCH_H__
#define __JR_SEARCH_H__

//game tree search manager

#include "board.h"
#include "step.h"
#include "hash.h"
#include "eval.h"
#include <string>
#include <vector>

//some limiting constants
#define SEARCH_MAX_DEPTH 20
#define SEARCH_MAX_COMBOS_PER_PLY 120

//hash bit constants
#define SEARCH_GAME_HIST_HASH_BITS 10

using namespace std;

class Search //Note: This class is not copyable, due to HashTable.
{
    public:
    Search(int numScoreHashBits);
    ~Search();

    StepCombo searchRoot(Board& board, int depth);
    short searchNode(Board& board, int depth, short alpha,  
                              short beta, vector<string>& nodePV, 
                              Board& refer);

    void addScoreEntry(Board& board, unsigned char scoreType, short score,
                       unsigned char bestMoveIndex, unsigned int depth);
    bool getScoreEntry(Board& board, ScoreEntry& entry, 
                       unsigned int depth);

    void incrementGameHistory(Board& board);
    void decrementGameHistory(Board& board);

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
    HashTable<ScoreEntry> scorehashes;
    Int64 scoreHashMask;
    Int64 scoreExtraHashMask;

    //a hash table to keep data on which positions have occurred at the
    //end of turns, used to make sure positions aren't repeated 3 times
    //which is a loss.
    HashTable<GameHistEntry> gameHist;
    Int64 gameHistHashMask;

    //keep pre-made arrays for storing moves at each ply, so that the
    //constructors/deconstructors aren't called so much
    StepCombo combos[SEARCH_MAX_DEPTH][SEARCH_MAX_COMBOS_PER_PLY];  
    unsigned int numCombos[SEARCH_MAX_DEPTH];

    //Eval instance to score stuff
    Eval eval;
};

#endif   
