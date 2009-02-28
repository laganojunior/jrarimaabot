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
//hash table to the number of bits
//////////////////////////////////////////////////////////////////////////////
Search :: Search(int numHashBits) : hashes(numHashBits)
{
    lastSearchMode = SEARCH_NONE;
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
    HashTableEntry thisEntry = hashes.getEntry(board.hash);
    if (board.lock == thisEntry.getLock() && depth <= thisEntry.getDepth()
        && thisEntry.hasScores())
    {
        short upper = thisEntry.getUpperBound();
        short lower = thisEntry.getLowerBound();

        //exact bounds
        if (upper == lower)                            
        {
            nodePV.resize(1);
            nodePV[0] = "<HT>";
            hashHits++;
            ++numTerminalNodes;
            return lower;
        }

        //upper bound cutoff
        if (upper <= alpha)
        {
            nodePV.resize(1);
            nodePV[0] = "<HT>";
            hashHits++;
            ++numTerminalNodes;
            return alpha;
        }

        //lower bound cutoff
        if (lower >= beta)
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

            //check if board position is in the history, if so, do not 
            //consider passing the turn
            if (isInHistory(board))
            {
                board.undoCombo(combos[i]);  
                continue;
            }

            //update the history to include this board state
            addToHistory(board);

            //set new refer state to this one
            Board newRefer = board;

            //search through opponent's turn
            nodeScore = -searchNodeAlphaBeta(board, depth - combos[i].stepCost
                                            ,-beta, -alpha, thisPV, newRefer);

            //then remove this board from the history
            removeFromHistory(board);

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
                addScoreEntry(board, 30000, beta, i, depth); 
                return beta;
            }
        }  
    }

    if (oldAlpha == alpha)
    {
        //if alpha remains unchanged, then it might be that the true value
        //is actually under alpha, so alpha is a upperbound

        addScoreEntry(board, alpha, -30000, bestIndex, depth);
    }   
    else
    {
        //alpha is actually an exact value of the score
        addScoreEntry(board, alpha, alpha, bestIndex, depth);
    }
        
    return alpha;
}

//////////////////////////////////////////////////////////////////////////////
//adds a board to the history
//////////////////////////////////////////////////////////////////////////////
void Search :: addToHistory(Board& board)
{                    
    //get a reference to the board hash entry
    HashTableEntry& hist = hashes.getEntry(board.hash);

    //set this hash entry to be also a history entry and preserve score
    //data on it if it matches this position. If the lock doesn't match,
    //favor the history entry and overwrite the current hash.
    if (hist.getLock() != board.lock)
    {   
        hist.set(false, true, 30000, -30000, 0, 0, board.lock);
    }
    else
    {
        hist.set(hist.hasScores(), true, hist.getUpperBound(), 
                 hist.getLowerBound(), hist.getMoveIndex(),
 
                 hist.getDepth(), board.lock);
    }
}

//////////////////////////////////////////////////////////////////////////////
//remove the entry off the history
//////////////////////////////////////////////////////////////////////////////
void Search :: removeFromHistory(Board& board)
{
    //get a reference to the board hash entry
    HashTableEntry& hist = hashes.getEntry(board.hash);
    
    //check if the entry is a history entry, and remove that flag, but keep
    //score data if present
    if (hist.getLock() == board.lock)
    {
        hist.set(hist.hasScores(), false, hist.getUpperBound(), 
                 hist.getLowerBound(), hist.getMoveIndex(), 
                 hist.getDepth(), board.lock);
    }
}

//////////////////////////////////////////////////////////////////////////////
//returns true iff the board is in the given history
//////////////////////////////////////////////////////////////////////////////
bool Search :: isInHistory(Board& board)
{
    //get a reference to the board hash entry
    HashTableEntry& hist = hashes.getEntry(board.hash);

    return hist.getLock() == board.lock && hist.isHistory();
}

//////////////////////////////////////////////////////////////////////////////
//loads up the history using the moves stored in the move file given,
//and using the hash parts to create the hashes.
//////////////////////////////////////////////////////////////////////////////
void Search :: loadHistory(string filename, 
                           Int64 hashparts[][MAX_TYPES][NUM_SQUARES],
                           Int64 lockparts[][MAX_TYPES][NUM_SQUARES])
{
    ifstream file(filename.c_str());

    vector<Board> history; //stack of board states used only for possible
                           //takebacks

    Board board(1); //number of bits don't matter, as the hash parts are
                    //overwritten manually

    //copy hash parts to match hashes for the board that is actually used
    //in searching.
    memcpy(board.hashParts, hashparts,sizeof(Int64) * MAX_COLORS * MAX_TYPES
                                                    * NUM_SQUARES);
    memcpy(board.lockParts, lockparts,sizeof(Int64) * MAX_COLORS * MAX_TYPES
                                                    * NUM_SQUARES);

    history.push_back(board);

    while (!file.eof())
    {
        string line;
        stringstream lineStream;

        getline(file, line);
        lineStream.str(line);

        string word;
        lineStream >> word; //get first word in line which has turn number
        
        stringstream wordStream;
        wordStream.str(word);
            
        int turnNum = 0;
    
        wordStream >> turnNum; //get the turn number out of the word

        if (turnNum == 1) //initial setup line
        {
            //read the next 16 words, as they contain the placement of the
            //16 pieces of that player
            for (int j = 0; j < 16; ++j)
            {
                //check for end of line prematurely, this may occur if this
                //is the current turn to play. Just stop loading quietly
                if (lineStream.eof()) 
                {
                    return;
                }

                lineStream >> word;

                //check if moves were taken back, if so, just revert to the
                //last board state
                if (word == string("takeback")) 
                {
                    board = history.back();
                    history.pop_back();
                    break;
                }
            
                if (word.length() != 3)
                {
                    Error error;
                    error << "In Search :: LoadHistory(string)\n"
                          << "Got invalid word in position file\n"
                          << "Word: " << word << '\n'
                          << "In line: " << line << '\n';
                    throw error;
                }

                unsigned char piece = pieceFromChar(word[0]);
                stringstream squareString;
                squareString << word[1] << word[2];
                unsigned char square = squareFromString(squareString.str());

                board.writePieceOnBoard(square, colorOfPiece(piece), 
                                        typeOfPiece(piece));

            }

            history.push_back(board);
        }
        else //normal moves
        {
            while (!lineStream.eof())
            {
                lineStream >> word;

                if (word == string("\n"))
                    break;
                
                //check if moves were taken back, if so just pop the last 
                //board on the history back onto this one and remove the
                //current one from the hash history
                if (word == string("takeback")) 
                {
                    removeFromHistory(board);
                    board = history.back();
                    history.pop_back();
                    break;
                }
                
                Step step;
                step.fromString(word);
                
                board.playStep(step);
            }

            history.push_back(board);

            addToHistory(board);
        }
    }

    file.close();
}

//////////////////////////////////////////////////////////////////////////////
//Attempts to write a entry into the hash table for score purposes. Will 
//only overwrite an entry if the depth is at least the depth of the current
//entry and the current entry is not for history
//////////////////////////////////////////////////////////////////////////////
void Search :: addScoreEntry(Board& board, short upper, short lower,
                            unsigned char bestMoveIndex, unsigned int depth)
{
    HashTableEntry& entry = hashes.getEntry(board.hash);

    if (entry.getDepth() <= depth)
    {
        //check for a collision
        if (entry.getLock() != board.lock)  
        {
            if (entry.isHistory()) //do not overwrite history  
                return;
                    
            collisions++; 
        }

        entry.set(true, isInHistory(board), upper, lower, bestMoveIndex, 
                  depth, board.lock); 
    }
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
