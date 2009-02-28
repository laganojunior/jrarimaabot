#include "error.h"
#include "search.h"
#include "board.h"
#include "step.h"
#include "piece.h"
#include "square.h"
#include <fstream>
#include <time.h>
#include <string.h>
#include <sstream>
#include <vector>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//Constructor. Basically set last search mode to none and initialize the 
//hash tables to the number of bits
//////////////////////////////////////////////////////////////////////////////
Search :: Search(int scoreHashBits)
{
    lastSearchMode = SEARCH_NONE;

    scorehashes.init(Int64FromIndex(scoreHashBits));
    scoreHashMask = 0;
    scoreExtraHashMask = 0;

    //create masks. Note that masks for hash keys are done from the least
    //signficant bit, but masks for extra bits are done the other way
    for (int i = 0; i < scoreHashBits; i++)
        scoreHashMask |= Int64FromIndex(i);

    for (int i = 0; i < SCORE_ENTRY_EXTRA_BITS; i++)
    {
        scoreExtraHashMask |= Int64FromIndex(63 - i);
    }
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor, does nothing
//////////////////////////////////////////////////////////////////////////////
Search :: ~Search()
{

}

//////////////////////////////////////////////////////////////////////////////
//Resets all relevant search stats and starts a search on the given board
//as the root node to the given depth using a alphabeta search and returns
//the best combo for the player to move.
//////////////////////////////////////////////////////////////////////////////
StepCombo Search :: searchRootAlphaBeta(Board& board, int depth)
{
    //reset search statistics
    lastSearchMode = SEARCH_ALPHABETA;
    numTerminalNodes = 0;
    numTotalNodes = 0;
    millis = 0;
    totalNodesPerSec = 0;
    score = -30000;
    pv.resize(1);
    maxDepth = depth;
    hashHits = 0;
    collisions = 0;

    //start timing now
    time_t reftime = clock();

    StepCombo combos[120];
    int num = board.genMoves(combos);
        
    if (num == 0 ) //no moves available? 
    {
        return StepCombo(); //give an empty step combo
    }

    Board refer = board;    
    for (int i = 0; i < num; ++i)
    {
        board.playCombo(combos[i]);  

        vector<string> nodePV;

        short nodeScore = searchNodeAlphaBeta(board, 
                                              depth - combos[i].stepCost, 
                                              score, 30000, nodePV, refer);

        board.undoCombo(combos[i]);

        if (nodeScore > score)
        {
            score = nodeScore;
            pv.resize(0);
            pv.insert(pv.begin(), combos[i].toString());
            pv.insert(pv.begin()+1, nodePV.begin(), nodePV.end());
        }  
    }

    //stop timing now
    time_t stoptime = clock();

    millis = (stoptime - reftime) * 1000 / CLOCKS_PER_SEC;
    totalNodesPerSec = (float)numTotalNodes / (float)millis * 1000;
    
    //extract current turn from pv
    int firstOppTurn = 0;
    StepCombo PVToPlay;
    for (int i = 0; i < pv.size(); ++i)
    {
        //stop when the step cost becomes 4, as that has to be the end
        //of the player's turn. 
        if (PVToPlay.stepCost == 4)
            break;

        StepCombo steps;
        steps.fromString(pv[i]);
    
        PVToPlay.addCombo(steps);
    }

    return PVToPlay;
}

//////////////////////////////////////////////////////////////////////////////
//runs an alpha beta search on the given board to the given depth and returns
//the solved score of this node and writes the principal variation from this
//node to nodePV. The refer board is the state of the board at the beginning
//of the turn, so don't go down paths that repeat that state
//////////////////////////////////////////////////////////////////////////////
short Search :: searchNodeAlphaBeta(Board& board, int depth, short alpha, 
                                    short beta, vector<string>& nodePV,
                                    Board& refer)
{   

    ++numTotalNodes; //count the node as explored

    if (board.isWin(board.sideToMove)) //check if this is a winning position
    {
        ++numTerminalNodes; //node is terminal

        return 30000; //return positive infinity.
    }

    if (depth <= 0) //terminal node due to depth
    {
        ++numTerminalNodes; //this node is terminal

        return board.eval(board.sideToMove);
    }

    //check if there is a hash position of at least this depth
    int bestIndexFromHash = 0;
    ScoreEntry thisEntry;
    if (getScoreEntry(board,thisEntry))
    {
        //exact bounds
        if (thisEntry.getScoreType() == SCORE_ENTRY_EXACT)                            
        {
            nodePV.resize(1);
            nodePV[0] = "<HT>";
            hashHits++;
            ++numTerminalNodes;
            return thisEntry.getScore();
        }

        //upper bound cutoff
        if (thisEntry.getScoreType() == SCORE_ENTRY_UPPER
            && thisEntry.getScore() <= alpha)
        {
            nodePV.resize(1);
            nodePV[0] = "<HT>";
            hashHits++;
            ++numTerminalNodes;
            return alpha;
        }

        //lower bound cutoff
        if (thisEntry.getScoreType() == SCORE_ENTRY_LOWER
            && thisEntry.getScore() >= beta)
        {
            nodePV.resize(1);
            nodePV[0] = "<HT>";
            hashHits++;
            ++numTerminalNodes;
            return beta;
        }

        //get best move from hash
        bestIndexFromHash = thisEntry.getMoveIndex();
    }

    StepCombo combos[120];
    int num = board.genMoves(combos);

    //check for immobility, which is a loss
    if (num == 0) 
    {
        ++numTerminalNodes; //node is terminal
        return -30000;
    }

    short oldAlpha = alpha;
    int bestIndex = 0;

    //play each step, and explore each subtree
    for (int i = 0; i < num; ++i) 
    {
        board.playCombo(combos[i]);  

        //check if the board is not back to the reference state
        if (board.samePieces(refer))
        {
            board.undoCombo(combos[i]);
            continue;
        }
        
        vector<string> thisPV;
        short nodeScore;
        
        //branch off wheter or not the turn has to be passed or not
        if (board.stepsLeft != 0)
        {
            //steps remaining, keep searching within this player's turn
            nodeScore = searchNodeAlphaBeta(board, depth - combos[i].stepCost, 
                                              alpha, beta, thisPV, refer);
        }
        else
        {
            //turn change is forced.

            //change state to fresh opponent turn
            unsigned char oldnumsteps = board.stepsLeft;
            board.changeTurn();

            //set new refer state to this one
            Board newRefer = board;

            //search through opponent's turn
            nodeScore = -searchNodeAlphaBeta(board, depth - combos[i].stepCost
                                            ,-beta, -alpha, thisPV, newRefer);

            //revert state back
            board.unchangeTurn(oldnumsteps);
        }

        board.undoCombo(combos[i]);       

        if (nodeScore > alpha)
        {
            alpha = nodeScore;

            nodePV.resize(0);
            nodePV.insert(nodePV.begin(), combos[i].toString());
            nodePV.insert(nodePV.begin() + 1, thisPV.begin(), 
                          thisPV.end());   

            bestIndex = i;

            if (alpha >= beta) //beta cutoff
            {   
                //Store the hash for this position and note a beta
                //cutoff, that is: note that beta is a lower bound
                addScoreEntry(board, SCORE_ENTRY_LOWER, beta, i, depth); 
				ScoreEntry entry;
                return beta;
            }
        }  
    }

    if (oldAlpha == alpha)
    {
        //if alpha remains unchanged, then it might be that the true value
        //is actually under alpha, so alpha is a upperbound

        addScoreEntry(board, SCORE_ENTRY_UPPER, alpha, bestIndex, depth);
		ScoreEntry entry;
    }   
    else
    {
        //alpha is actually an exact value of the score
        addScoreEntry(board, SCORE_ENTRY_EXACT, alpha, bestIndex, depth);
		ScoreEntry entry;
    }
        
    return alpha;
}

//////////////////////////////////////////////////////////////////////////////
//Attempts to write a entry into the score hash table. Only overwrite the 
//current entry if the depth is at least as much as the one already in the
//table
//////////////////////////////////////////////////////////////////////////////
void Search :: addScoreEntry(Board& board, unsigned char scoreType,
                            short score,
                            unsigned char bestMoveIndex, unsigned int depth)
{
    ScoreEntry& entry = scorehashes.getEntry(board.hash & scoreHashMask);

    if (!entry.isFilled() || entry.getDepth() <= depth)
    {
        //check for a collision
        if (entry.isFilled() && 
            entry.getExtra() != (board.hash & scoreExtraHashMask))  
        {
            collisions++; 
        }

        entry.set(true, scoreType, score, bestMoveIndex, 
                  depth, board.hash & scoreExtraHashMask); 
    }
}

//////////////////////////////////////////////////////////////////////////////
//Attempts to get a board's entry from the score hash table. If the board's
//entry is found in the hashtable, it is written in the given entry reference
//and this function returns true. If not, false is returned
//////////////////////////////////////////////////////////////////////////////
bool Search :: getScoreEntry(Board& board, ScoreEntry& entry)
{
    //get the entry from the array
    ScoreEntry thisEntry = scorehashes.getEntry(board.hash & scoreHashMask);
    
    //check if the extra bits match
    if (thisEntry.isFilled() && 
        thisEntry.getExtra() == (board.hash & scoreExtraHashMask))
    {
        entry = thisEntry;
        return true;    
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
//returns a string that describes the statistics of the previous search
//////////////////////////////////////////////////////////////////////////////
string Search :: getStatString()
{
    stringstream out;

    switch (lastSearchMode)
    {
        case SEARCH_ALPHABETA:
        {
            out << "Depth: " << maxDepth << endl
                << "Terminal Nodes: " << numTerminalNodes << endl
                << "Total Nodes: " <<  numTotalNodes << endl
                << "Time Taken: " << millis << "ms\n"
                << "Nodes/Sec: " << totalNodesPerSec << endl
                << "Score: " << score << endl
                << "Hash Table Hits: " << hashHits << endl
                << "Hash Table Collisions: " << collisions << endl;

            out << "PV:";
            for (int i = 0; i < pv.size(); ++i)
                out << " " << pv[i];
            out << endl;

        }break;

        case SEARCH_NONE:
        {
            out << "No Search Done\n";  
        }break;
    }

    return out.str();
}
