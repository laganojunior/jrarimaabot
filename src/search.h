#ifndef __JR_SEARCH_H__
#define __JR_SEARCH_H__

//game tree search manager. Has algorithms for different types of searches

#include "board.h"
#include "step.h"
#include "hash.h"
#include <string>
#include <vector>

//search mode types
#define SEARCH_NONE      0
#define SEARCH_ALPHABETA 1

using namespace std;

class Search //Note: This class is not copyable, due to HashTable.
{
    public:
    Search(int numScoreHashBits);
    ~Search();

    StepCombo searchRootAlphaBeta(Board& board, int depth);
    short searchNodeAlphaBeta(Board& board, int depth, short alpha,  
                              short beta, vector<string>& nodePV, 
                              Board& refer);

    void addScoreEntry(Board& board, unsigned char scoreType, short score,
                       unsigned char bestMoveIndex, unsigned int depth);
    bool getScoreEntry(Board& board, ScoreEntry& entry, 
                       unsigned int depth);

    string getStatString();
    string getShortStatString();

    int lastSearchMode; //the last search algorithm used in searching, used
                        //when displaying statistics as certain stats don't
                        //make sense for some algorithms

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
};

#endif
