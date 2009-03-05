#include "error.h"
#include "search.h"
#include "board.h"
#include "step.h"
#include "piece.h"
#include "square.h"
#include "eval.h"
#include <fstream>
#include <time.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//Constructor. Basically set last search mode to none and initialize the 
//hash tables to the number of bits
//////////////////////////////////////////////////////////////////////////////
Search :: Search(int scoreHashBits)
{

    scorehashes.init(Int64FromIndex(scoreHashBits));
    gameHist.init(Int64FromIndex(SEARCH_GAME_HIST_HASH_BITS));
    searchHist.init(Int64FromIndex(SEARCH_SEARCH_HIST_HASH_BITS));

    //create masks. Note that masks for hash keys are done from the least
    //signficant bit, but masks for extra bits are done the other way
    scoreHashMask = 0;
    scoreExtraHashMask = 0;

    for (int i = 0; i < scoreHashBits; i++)
        scoreHashMask |= Int64FromIndex(i);

    for (int i = 0; i < SCORE_ENTRY_EXTRA_BITS; i++)
    {
        scoreExtraHashMask |= Int64FromIndex(63 - i);
    }

    gameHistHashMask = 0;

    for (int i = 0; i < SEARCH_GAME_HIST_HASH_BITS; i++)
        gameHistHashMask |= Int64FromIndex(i);

    for (int i = 0; i < SEARCH_SEARCH_HIST_HASH_BITS; i++)
        searchHistHashMask |= Int64FromIndex(i);

    //set killer moves to zero array
    memset(eval.killermove, 0, sizeof(unsigned char) * NUM_SQUARES * 
                               NUM_SQUARES * MAX_COLORS);
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor, does nothing
//////////////////////////////////////////////////////////////////////////////
Search :: ~Search()
{

}

//////////////////////////////////////////////////////////////////////////////
//Resets all relevant search stats and starts a search on the given board
//as the root node to the given depth using a search and returns
//the best combo for the player to move.
//////////////////////////////////////////////////////////////////////////////
StepCombo Search :: searchRoot(Board& board, int depth)
{
    //reset search statistics
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

    numCombos[0] = board.genMoves(combos[0]);
        
    if (numCombos[0] == 0 ) //no moves available? 
    {
        return StepCombo(); //give an empty step combo
    }

    //check if there is a hash position of at least this depth, but only
    //to get the best move from the hash.
    int bestIndexFromHash = -1;
    ScoreEntry thisEntry;   
    if (getScoreEntry(board,thisEntry, depth))
    {
        //get best move from hash
        bestIndexFromHash = thisEntry.getMoveIndex();
    }

    //Add this board to the search history
    addSearchHistory(board);

    //give the combos some score for move ordering
    eval.scoreCombos(combos[0], numCombos[0], board.sideToMove, 
                     bestIndexFromHash);

    for (int i = 0; i < numCombos[0]; ++i)
    {
        //get the next best combo to look at
        unsigned int nextIndex = getNextBestCombo(0);

        board.playCombo(combos[0][nextIndex]);  

        //Check if the position has already occured in the search history.
        //This can happen in the root if the first move is a complete pass
        if (hasOccured(board))
        {
            board.undoCombo(combos[0][nextIndex]);
            continue;
        }

        //add this new board to the search history
        addSearchHistory(board);

        vector<string> nodePV;

        short nodeScore = searchNode(board, 
                                     depth - combos[0][nextIndex].stepCost, 
                                     score, 30000, nodePV);

        //remove the board from the history
        removeSearchHistory(board);

        board.undoCombo(combos[0][nextIndex]);

        if (nodeScore > score)
        {
            score = nodeScore;
            pv.resize(0);
            pv.insert(pv.begin(), combos[0][nextIndex].toString());
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
        //of the player's turn. Also stop whenever something that is not
        //a move is read.
        if (PVToPlay.stepCost == 4 || pv[i] == string("<HT>"))
            break;

        StepCombo steps;
        steps.fromString(pv[i]);
    
        PVToPlay.addCombo(steps);
    }

    return PVToPlay;
}

//////////////////////////////////////////////////////////////////////////////
//runs a search on the given board to the given depth and returns
//the solved score of this node and writes the principal variation from this
//node to nodePV. The refer board is the state of the board at the beginning
//of the turn, so don't go down paths that repeat that state
//////////////////////////////////////////////////////////////////////////////
short Search :: searchNode(Board& board, int depth, short alpha, 
                           short beta, vector<string>& nodePV)
{   

    ++numTotalNodes; //count the node as explored

    //check if this is a winning position
    if (eval.isWin(board, board.sideToMove)) 
    {
        ++numTerminalNodes; //node is terminal

        return 30000; //return positive infinity.
    }

    if (depth <= 0) //terminal node due to depth
    {
        ++numTerminalNodes; //this node is terminal

        return eval.evalBoard(board, board.sideToMove);
    }

    //check if there is a hash position of at least this depth
    int bestIndexFromHash = -1;
    ScoreEntry thisEntry;   
    if (getScoreEntry(board,thisEntry, depth))
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

    //set a variable to measure the ply, which should increase the farther
    //the search is into the tree.
    unsigned int ply = maxDepth - depth;

    numCombos[ply] = board.genMoves(combos[ply]);

    //check for immobility, which is a loss
    if (numCombos[ply] == 0) 
    {
        ++numTerminalNodes; //node is terminal
        return -30000;
    }

    short oldAlpha = alpha;
    int bestIndex = 0;

    //give the combos some score for move ordering
    eval.scoreCombos(combos[ply], numCombos[ply], board.sideToMove, 
                     bestIndexFromHash);

    //play each step, and explore each subtree
    for (int i = 0; i < numCombos[ply]; ++i) 
    {
        //get the next best combo to look at
        unsigned int nextIndex = getNextBestCombo(ply);

        board.playCombo(combos[ply][nextIndex]);  

        //Check if the position has already occured in the search history.
        if (hasOccured(board))
        {
            board.undoCombo(combos[ply][nextIndex]);
            continue;
        }

        //add this new board to the search history
        addSearchHistory(board);
        
        vector<string> thisPV;
        short nodeScore;
        
        //branch off wheter or not the turn has to be passed or not
        if (board.stepsLeft != 0)
        {
            //steps remaining, keep searching within this player's turn
            nodeScore = searchNode(board, 
                                   depth - combos[ply][nextIndex].stepCost, 
                                   alpha, beta, thisPV);
        }
        else
        {
            //turn change is forced.

            //change state to fresh opponent turn
            unsigned char oldnumsteps = board.stepsLeft;
            board.changeTurn();

            //Check if state is now one that has occurred in the history
            //2 times before (making this the third). If it is, disallow the
            //move
            if (getGameHistoryOccurences(board) >= 2)
            {   
                board.unchangeTurn(oldnumsteps);
                board.undoCombo(combos[ply][nextIndex]);
                continue;
            }

            //Increment board state occurences
            incrementGameHistory(board);

            //search through opponent's turn
            nodeScore = -searchNode(board, 
                                    depth - combos[ply][nextIndex].stepCost,
                                    -beta, -alpha, thisPV);

            //Decrement board state occurences
            decrementGameHistory(board);

            //revert state back
            board.unchangeTurn(oldnumsteps);
        }

        //remove the board from the history
        removeSearchHistory(board);

        board.undoCombo(combos[ply][nextIndex]);       

        if (nodeScore > alpha)
        {
            alpha = nodeScore;

            nodePV.resize(0);
            nodePV.insert(nodePV.begin(),
                         combos[ply][nextIndex].toString());
            nodePV.insert(nodePV.begin() + 1, thisPV.begin(), 
                          thisPV.end());   

            bestIndex = i;

            if (alpha >= beta) //beta cutoff
            {   
                //Store the hash for this position and note a beta
                //cutoff, that is: note that beta is a lower bound
                addScoreEntry(board, SCORE_ENTRY_LOWER, beta, i, depth); 

                //give the killer score for this type of move an increase
                //depending on the depth to go of this search.
                if (eval.killermove[combos[ply][nextIndex].steps[0].getFrom()]
                                   [combos[ply][nextIndex].steps[0].getTo()]
                                   [board.sideToMove] + depth <= 255)

                    eval.killermove[combos[ply][nextIndex].steps[0].getFrom()]
                                   [combos[ply][nextIndex].steps[0].getTo()]
                                   [board.sideToMove] += depth;
                    
                return beta;
            }
        }  
    }

    if (oldAlpha == alpha)
    {
        //if alpha remains unchanged, then it might be that the true value
        //is actually under alpha, so alpha is a upperbound

        addScoreEntry(board, SCORE_ENTRY_UPPER, alpha, bestIndex, depth);
    }   
    else
    {
        //alpha is actually an exact value of the score
        addScoreEntry(board, SCORE_ENTRY_EXACT, alpha, bestIndex, depth);
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
bool Search :: getScoreEntry(Board& board, ScoreEntry& entry,
                             unsigned int depth)
{
    //get the entry from the array
    ScoreEntry thisEntry = scorehashes.getEntry(board.hash & scoreHashMask);
    
    //check if the extra bits match and the depth is at least the one wanted
    if (thisEntry.isFilled() && 
        thisEntry.getExtra() == (board.hash & scoreExtraHashMask) && 
        thisEntry.getDepth() >= depth)
    {
        entry = thisEntry;
        return true;    
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
//Increase the count in the game history for the given board position.
//////////////////////////////////////////////////////////////////////////////
void Search :: incrementGameHistory(Board& board)
{
    GameHistEntry& hist = gameHist.getEntry(board.hashPiecesOnly
                                            & gameHistHashMask);
    if (hist.numOccur < 127)
        hist.numOccur++;
}

//////////////////////////////////////////////////////////////////////////////
//Decrease the count in the game history for the given board position
//////////////////////////////////////////////////////////////////////////////
void Search :: decrementGameHistory(Board& board)
{
    GameHistEntry& hist = gameHist.getEntry(board.hashPiecesOnly 
                                            & gameHistHashMask);
    if (hist.numOccur > 0)
        hist.numOccur--;
}

//////////////////////////////////////////////////////////////////////////////
//returns the count in the game history for the given board position
//////////////////////////////////////////////////////////////////////////////
int  Search :: getGameHistoryOccurences(Board& board)
{
    GameHistEntry& hist = gameHist.getEntry(board.hashPiecesOnly
                                            & gameHistHashMask);
    return hist.numOccur;
}

//////////////////////////////////////////////////////////////////////////////
//Loads a move file and places all the moves at the beginning of the turn 
//in the history, using the board given as a reference for hashes. It is 
//important that the board given is the board used to do searches (or at least
//a copy of that board with the same hash parts) as the
//data is only good if the hashes match up.
//////////////////////////////////////////////////////////////////////////////
void Search :: loadMoveFile(string filename, Board board)
{

    board.reset();
    
    //keep a stack of boards so that takebacks can easily be done
    vector<Board> boardHist;
    boardHist.push_back(board);


    ifstream in(filename.c_str());
    
    if (!in.is_open())
    {
        Error error;
        error << "From Search :: loadMoveFile(string, board)\n"
              << "Could not open file: "
              << filename << "\n";
        throw error;
    }

    //go through all lines, until the file ends, or quit is signaled
    bool quit = false;
    while (!in.eof() || quit)
    {
        string line;

        //read the line
        getline(in, line);
        stringstream lineStream(line);

        string word;

        //grab the first word, which tells what the the turn number is.
        lineStream >> word; 

        if (word == string(""))
            break;

        stringstream wordStream(word);
        if (!isdigit(word[0]))
        {
            Error error;
            error << "From Search:: loadMoveFile(string, board)\n"
                  << "Expected turn number as first part in first line\n"
                  << "Got: " << word << '\n';
            throw error;
        }

        int turnNumber;
        wordStream >> turnNumber;

        if (turnNumber == 1)
        {
            //first turn has pieces being played onto the board, read the
            //next 16 words which should say where the pieces go
            
            bool okay = true;
            for (int i = 0; i < 16; i++)
            {
                lineStream >> word;
                
                if (lineStream.bad() || lineStream.eof())
                {
                    quit = 1;
                    okay = false;
                    break;
                }

                //Check if a move is being taken back instead
                if (word == string("takeback"))
                {
                    //remove an occurence of the current board to the history
                    decrementGameHistory(board);

                    //pop the last board on the stack back onto the current
                    //state
                    board = boardHist.back();
                    boardHist.pop_back();

                    //flag that it is not okay to write this board state
                    //to history
                    okay = false;
                    break;
                }

                if (word.length() != 3)
                {
                    Error error;
                    error << "From Search:: loadMoveFile(string, board)\n";
                    error << "Invalid Format for first piece placement\n";
                    error << "Got: " << word << '\n';
                    
                    throw error;
                }

                //read the piece
                unsigned char piece = pieceFromChar(word[0]);
                     
                //read the location
                stringstream squareStringStream;    
                squareStringStream << word[1] << word[2];
                unsigned char square = 
                                 squareFromString(squareStringStream.str());

                //write the piece onto the location
                board.writePieceOnBoard(square, colorOfPiece(piece),
                                                typeOfPiece(piece));
                    
            }

            if (okay)
            {
                //Add the current states to the history and the board stack
                incrementGameHistory(board);
                boardHist.push_back(board);
            }
        }
        else
        {
            //turn should have a list of steps to play
            StepCombo steps;
    
            //get the rest of the line
            string rest;
            getline(lineStream, rest);

            if (rest == string("takeback"))
            {
                //remove an occurence of the current board to the history
                decrementGameHistory(board);

                //pop the last board on the stack back onto the current
                //state
                board = boardHist.back();
                boardHist.pop_back();

                continue;
            }

            if (rest == string(""))
                break;

            //create the steps from the line
            steps.fromString(rest);

            //play the steps on the board, then give the turn away
            board.playCombo(steps);
            board.changeTurn();
        
            //Add the current states to the history and the board stack
            incrementGameHistory(board);
            boardHist.push_back(board);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//Add a board to the search history
//////////////////////////////////////////////////////////////////////////////
void Search :: addSearchHistory(Board& board)
{
    SearchHistEntry& hist = searchHist.getEntry(board.hashPiecesOnly
                                               & searchHistHashMask);
    hist.occured = true;
}

//////////////////////////////////////////////////////////////////////////////
//Remove a board from the search history
//////////////////////////////////////////////////////////////////////////////
void Search :: removeSearchHistory(Board& board)
{
    SearchHistEntry& hist = searchHist.getEntry(board.hashPiecesOnly
                                               & searchHistHashMask);
    hist.occured = false;
}

//////////////////////////////////////////////////////////////////////////////
//returns wheter or not a board is present in the search history
//////////////////////////////////////////////////////////////////////////////
bool Search :: hasOccured(Board& board)
{
    SearchHistEntry& hist = searchHist.getEntry(board.hashPiecesOnly
                                               & searchHistHashMask);
    return hist.occured;
}
        

//////////////////////////////////////////////////////////////////////////////
//Gets the index of next best combo from the combos array at the given ply 
//according to the scores for each one and sets the score for that one 
//sufficiently low to remove it from consideration next time this is called
//////////////////////////////////////////////////////////////////////////////
unsigned int Search :: getNextBestCombo(unsigned int ply)
{
    short bestScore = combos[ply][0].score;   
    unsigned int bestIndex = 0;

    //search for the best score
    for (int i = 1; i < numCombos[ply]; ++i)
    {
        if (combos[ply][i].score > bestScore)
        {
            bestScore = combos[ply][i].score;
            bestIndex = i;
        }
    }

    //remove this combo from future consideration by setting its score
    //very low
    combos[ply][bestIndex].score = -30000;
    
    return bestIndex;
}

//////////////////////////////////////////////////////////////////////////////
//returns a string that describes the statistics of the previous search
//////////////////////////////////////////////////////////////////////////////
string Search :: getStatString()
{
    stringstream out;

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

    return out.str();
}

//////////////////////////////////////////////////////////////////////////////
//Returns a shorter version of the statisitic string which should fit in
//one line. It will only tell the depth, score, nodes searched, and the pv
//////////////////////////////////////////////////////////////////////////////
string Search :: getShortStatString()
{
    stringstream out;

    out << setw(6) << maxDepth << setw(6) << score << setw(11) 
        << numTotalNodes << setw(10) << millis;
    for (int i = 0; i < pv.size(); ++i)
        out << " " << pv[i];

    return out.str();
}

