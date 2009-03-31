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
    //Initialize hash tables
    transTable.setHashKeySize(scoreHashBits);
    gameHistTable.setHashKeySize(10);
    searchHist.init(Int64FromIndex(SEARCH_SEARCH_HIST_HASH_BITS));

    //create hash masks
    searchHistHashMask = 0;
    for (int i = 0; i < SEARCH_SEARCH_HIST_HASH_BITS; i++)
        searchHistHashMask |= Int64FromIndex(i);
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor, does nothing
//////////////////////////////////////////////////////////////////////////////
Search :: ~Search()
{

}

//////////////////////////////////////////////////////////////////////////////
//Resets all relevant search stats and does an iterative deepening search
//and returns the best move
//////////////////////////////////////////////////////////////////////////////
StepCombo Search :: iterativeDeepen(Board& board, int maxDepth, ostream& log)
{
    //reset search statistics
    numTerminalNodes = 0;
    numTotalNodes = 0;
    hashHits = 0;
    
    eval.reset();
    killerTable.reset();

    log << setw(6) << "Depth" << setw(6) << "Score" << setw(15) 
        << "Nodes" << setw(10) << "Time(ms)" << setw(10)
        << "Nodes/Sec" << " PV\n";
    //start timing now
    time_t reftime = clock();

    //Add this board to the search history
    addSearchHistory(board);

    vector<string> pv;

    for (int currDepth = 1; currDepth <= maxDepth; currDepth++)
    {
        StepCombo pass;
        pass.genPass(board.stepsLeft);
        pass.evalScore = eval.evalBoard(board, board.sideToMove);
        short score = searchNode(board, currDepth, 4 - board.stepsLeft,
                                 -30000, 30000, pv, true, pass, false);
        
        Int64 currMillis = (clock() - reftime) * 1000 / CLOCKS_PER_SEC + 1; 
        log << setw(6) << currDepth << setw(6) << score << setw(15) 
            << numTotalNodes << setw(10) << currMillis << setw(10)
            << (unsigned int)((float)numTotalNodes / currMillis * 1000);
        for (int i = 0; i < pv.size(); ++i)
            log << " " << pv[i];
        log << endl;

        log.flush();

        //If the score is so great, then it's probably a win, so don't
        //search any further
        if (score >= 25000)
            break;
    }

    removeSearchHistory(board);

    //extract current turn from pv
    StepCombo PVToPlay;
    for (int i = 0; i < pv.size(); ++i)
    {
        //stop when the step cost becomes the amount of steps left, as that
        //has to be the end of the player's turn. Also stop whenever 
        //something that is not a move is read.
        if (PVToPlay.stepCost == board.stepsLeft || pv[i] == string("<HT>"))
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
short Search :: searchNode(Board& board, int depth, int ply, short alpha, 
                           short beta, vector<string>& nodePV, bool isRoot
                           ,StepCombo& lastMove, bool genDependent)
{   
    ++numTotalNodes; //count the node as explored

    //check if this is a winning position
    if (eval.isWin(board, board.sideToMove)) 
    {
        ++numTerminalNodes; //node is terminal

        return beta; //return positive infinity.
    }

    if (depth <= 0) //terminal node due to depth
    {
        ++numTerminalNodes; //this node is terminal
        //Get the evaluated score, which is stored as the lastMove's score
        //But make sure it is within the alpha-beta window
        if (lastMove.evalScore < alpha)  
            return alpha;
        
        if (lastMove.evalScore > beta)
            return beta;

        return lastMove.evalScore;
    }

    //Check if the player has the last move, if the position is already
    //scored higher than beta, then assuming that this player can only help
    //the position by moving (he can also just pass, but this bot doesn't
    //handle passes), this position will lead to a beta cutoff no matter
    //what.
    if (depth <= board.stepsLeft && lastMove.evalScore >= beta)
        return beta;

    //make sure to just pass here if there are no steps left, this can only
    //occur if the root node started with no steps left.
    if (board.stepsLeft == 0)
    {
        StepCombo pass;
        pass.genPass(0);
        pass.evalScore = eval.evalBoard(board, board.sideToMove);
        return doMoveAndSearch(board, depth, ply, alpha, beta, nodePV, pass,
                               pass.evalScore);
    }

    //list of moves to try before generating all the moves
    vector<StepCombo> preGenSteps;

    //check if there is a hash position of at least this depth
    if (transTable.hasValidEntry(board.hash))
    {
        TranspositionEntry thisEntry = transTable.getEntry(board.hash);  

        //adjust the bounds with the bounds in the hash entry, if the depth 
        //matches

        //exact bounds
        if (depth <= thisEntry.getDepth())
        {
            short cutScore;
            if (thisEntry.getScoreType() == SCORE_ENTRY_EXACT)                            
            {       
                cutScore = thisEntry.getScore();
                alpha = cutScore;
                beta = cutScore;
                
                
            }

            //upper bound adjust
            if (thisEntry.getScoreType() == SCORE_ENTRY_UPPER)
            {
                beta = thisEntry.getScore();
                cutScore = alpha;
            }

            //lower bound adjust
            if (thisEntry.getScoreType() == SCORE_ENTRY_LOWER)
            {
                alpha = thisEntry.getScore();
                cutScore = beta;
            }

            //check if the adjustments made a cutoff, but only do it if this
            //is not the root node, as at the root, there should be at least
            //one move in the pv.
            if (alpha >= beta && !isRoot)
            {
                nodePV.resize(1);
                nodePV[0] = "<HT>";
                hashHits++;
                ++numTerminalNodes;
                return cutScore;
            }
        }

        //Get the move from the hash table, see if it can be played, and
        //play it now before generating all moves
        if (thisEntry.getNumSteps() == 1)
        {
            StepCombo hashBestCombo;
            if (board.gen1Step(hashBestCombo, thisEntry.getFrom1(),
                              thisEntry.getTo1()))
                preGenSteps.push_back(hashBestCombo);
        }
        else
        {
            StepCombo hashBestCombo;
            if (board.gen2Step(hashBestCombo, thisEntry.getFrom1(),
                              thisEntry.getTo1(), thisEntry.getFrom2()))
                preGenSteps.push_back(hashBestCombo);
        }
    }

    short oldAlpha = alpha;
    StepCombo bestCombo;


    //Get a list of killer moves to try
    vector<RawMove> killer = killerTable.getKillerMoves(ply);

    //Check if the killer moves are available to play here, and add
    //them to the pre gen list
    for (int i = 0; i < killer.size(); i++)
    {
        StepCombo killerCombo;

        if (killer[i].numSteps == 1)
        {
            if (board.gen1Step(killerCombo, killer[i].from1, killer[i].to1))
            {
                //Make sure the move isn't already in the pre gen list
                bool alreadyIn = false;
                for (int j = 0; j < preGenSteps.size(); j++)
                {
                    if (preGenSteps[j] == killerCombo)
                    {
                        alreadyIn = true;
                        break;
                    }   
                }

                //place the killer move onto the pre gen list
                if (!alreadyIn)
                    preGenSteps.push_back(killerCombo);
            }
        }
        else
        {
            if (board.gen2Step(killerCombo, killer[i].from1, killer[i].to1,
                                            killer[i].from2))
            {
                //Make sure the move isn't already in the pre gen list
                bool alreadyIn = false;
                for (int j = 0; j < preGenSteps.size(); j++)
                {
                    if (preGenSteps[j] == killerCombo)
                    {
                        alreadyIn = true;
                        break;
                    }   
                }

                //place the killer move onto the pre gen list
                if (!alreadyIn)
                    preGenSteps.push_back(killerCombo);
            }
        }
    }

    //if there are any pre-gen steps, explore them first
    if (preGenSteps.size() > 0) 
    {
        bestCombo = preGenSteps[0];

        for (int i = 0; i < preGenSteps.size(); i++)
        {
            StepCombo next = preGenSteps[i];

            short nodeScore = doMoveAndSearch(board, depth, ply, alpha, 
                                              beta, nodePV, next,
                                              lastMove.evalScore); 

            if (nodeScore > alpha) 
            {   
                bestCombo = next;
                alpha = nodeScore;
                if (nodeScore >= beta) //beta cutoff
                {   
                    //Store the hash for this position and note a beta
                    //cutoff, that is: note that beta is a lower bound
                    transTable.setEntry(board.hash, 
                            TRANSPOSITION_SCORETYPE_LOWER, beta, depth,
                            next.getRawMove());

                    //increase killer score
                    killerTable.addKillerMove(ply, next.getRawMove());

                    return beta;
                }
            }
        }       
    }

    //Check to make sure the combo arrays has entries up to this ply
    if ((int)combos.size() - 1 < (int)ply)
    {
        while (combos.size() != ply + 1)
        {
            vector<StepCombo> vec;
            combos.push_back(vec);
            combos[combos.size() - 1].resize(SEARCH_MAX_COMBOS_PER_PLY);
        }
    }

    //If no cutoff was caused by the pre generation steps, generate the
    //remaining list of moves to explore. This can be the full list or the
    //list of moves dependent on the last move.
    //
    //Generate dependent moves on these conditions:
    //1) The ply indicates that this is the second step in the player's move.
    //   Since usually the best move does not consist of 4 independent steps,
    //   it is same to assume that at least one step is dependent on a 
    //   previous one. Thus force the second step to be dependent on the first
    //2) The genDependent flag was passed true. This occurs if
    //   doMoveAndSearch detects that the last step did not make the position
    //   better. By assuming that there always exists a move that improves
    //   the current position, the only way that the prior step can lead to
    //   the best step combo is if it unlocks another step that improves the
    //   position.
    unsigned int numCombos; 
    if ((ply % 4 == 1 || genDependent) && lastMove.numSteps > 0)
    {
        numCombos = board.genDependentMoves(combos[ply], lastMove);
    }
    else
    {
        numCombos = board.genMoves(combos[ply]);
    }
        
    if (numCombos == 0 ) 
    {
        //loss by immobility
        return alpha;
    }

    //if there were no pre gen steps, set the first best move now
    if (preGenSteps.size() == 0)
    {
        bestCombo = combos[ply][0];
    }

    //score the combos for sorting
    eval.scoreCombos(combos[ply], numCombos, board.sideToMove); 

    //play each step, and explore each subtree
    for (int i = 0; i < numCombos; i++)
    {
        //get the next combo to look at.
        StepCombo next = getNextBestComboAndRemove(combos[ply], numCombos);

        //Check if this combo is in the pre gen list, if so, just don't
        //research it
        bool redo = false;
        for (int j = 0; j < preGenSteps.size(); j++)
        {
            if (next == preGenSteps[j])
            {
                redo = true;
                break;
            }
        }

        if (redo)
            continue;

        //explore the subtree for this move.

        short nodeScore = doMoveAndSearch(board, depth, ply, alpha, beta, 
                                          nodePV, next, lastMove.evalScore); 
            
        if (nodeScore > alpha) 
        {   
            bestCombo = next;
            alpha = nodeScore;
            if (nodeScore >= beta) //beta cutoff
            {   
                //Store the hash for this position and note a beta
                //cutoff, that is: note that beta is a lower bound
                transTable.setEntry(board.hash, 
                        TRANSPOSITION_SCORETYPE_LOWER, beta, depth,
                        next.getRawMove());


                //increase killer score
                killerTable.addKillerMove(ply, next.getRawMove());

                return beta;
            }
        }
    } 

    if (oldAlpha == alpha)
    {
        //if alpha remains unchanged, then it might be that the true value
        //is actually under alpha, so alpha is a upperbound

        transTable.setEntry(board.hash, TRANSPOSITION_SCORETYPE_UPPER, alpha, 
                            depth, bestCombo.getRawMove());
    }   
    else
    {
        //alpha is actually an exact value of the score
        transTable.setEntry(board.hash, TRANSPOSITION_SCORETYPE_EXACT, alpha, 
                            depth, bestCombo.getRawMove());
    }
        
    return alpha;
}

//////////////////////////////////////////////////////////////////////////////
//Play a move on this board, and searches the resulting child node. returns
//the updated alpha score.
//////////////////////////////////////////////////////////////////////////////
short Search :: doMoveAndSearch(Board& board, int depth, int ply, short alpha,  
                                short beta, vector<string>& nodePV,
                                StepCombo& combo, short lastScore)
{
    
    board.playCombo(combo);  

    //Check if the position has already occured in the search history.
    if (hasOccured(board))
    {
        board.undoCombo(combo);
        return alpha;
    }

    //add this new board to the search history
    addSearchHistory(board);
    
    vector<string> thisPV;
    short nodeScore;
    
    //branch off wheter or not the turn has to be passed or not
    if (board.stepsLeft != 0)
    {
        //Check if the move helped the position. If not, then dependent moves
        //must be generated next turn
        combo.evalScore = eval.evalBoard(board, board.sideToMove);

        bool genDependent;
        if (combo.evalScore <= lastScore)
            genDependent = true;
        else
            genDependent = false;

        //steps remaining, keep searching within this player's turn
        nodeScore = searchNode(board, 
                               depth - combo.stepCost,
                               ply + combo.stepCost,
                               alpha, beta, thisPV, false, combo,
                               genDependent);
    }
    else
    {
        //turn change is forced.

        //Check if the move helped the position. If not, then this move by
        //assumption cannot have been the best move to play
        combo.evalScore = eval.evalBoard(board, board.sideToMove);
        if (combo.evalScore <= lastScore)
        {
            removeSearchHistory(board);
            board.undoCombo(combo);            
            return alpha;
        }

        //change state to fresh opponent turn
        unsigned char oldnumsteps = board.stepsLeft;
        board.changeTurn();

        //Check if state is now one that has occurred in the history
        //2 times before (making this the third). If it is, disallow the
        //move
        if (gameHistTable.getNumOccur(board.hashPiecesOnly) >= 2)
        {   
            board.unchangeTurn(oldnumsteps);
            removeSearchHistory(board);
            board.undoCombo(combo);
            return alpha;
        }

        //Increment board state occurences
        gameHistTable.incrementOccur(board.hashPiecesOnly);

        //search through opponent's turn
        StepCombo pass;
        pass.genPass(board.stepsLeft);
        pass.evalScore = -combo.evalScore;
        nodeScore = -searchNode(board, 
                                depth - combo.stepCost,
                                ply + combo.stepCost,
                                -beta, -alpha, thisPV, false, pass,
                                false);

        //Decrement board state occurences
        gameHistTable.decrementOccur(board.hashPiecesOnly);

        //revert state back
        board.unchangeTurn(oldnumsteps);
    }

    //remove the board from the history
    removeSearchHistory(board);

    board.undoCombo(combo);       

    if (nodeScore > alpha)
    {
        alpha = nodeScore;

        nodePV.resize(0);
        nodePV.insert(nodePV.begin(),
                      combo.toString());
        nodePV.insert(nodePV.begin() + 1, thisPV.begin(), 
                      thisPV.end());   

        if (alpha >= beta) //beta cutoff
        {       
            return beta;
        }
    }  
    
    return alpha;
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
                
                if (lineStream.bad())
                {
                    quit = 1;
                    okay = false;
                    break;
                }

                //Check if a move is being taken back instead
                if (word == string("takeback"))
                {
                    //remove an occurence of the current board to the history
                    gameHistTable.decrementOccur(board.hashPiecesOnly);

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
                gameHistTable.incrementOccur(board.hashPiecesOnly);
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
                gameHistTable.decrementOccur(board.hashPiecesOnly);

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
            gameHistTable.incrementOccur(board.hashPiecesOnly);
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
//Returns the next best combo from the list
//according to the scores for each one and sets the score for that one 
//sufficiently low to remove it from consideration next time this is called
//////////////////////////////////////////////////////////////////////////////
StepCombo Search :: getNextBestComboAndRemove(vector<StepCombo>& list, 
                                              unsigned int num)
{
    short bestScore = list[0].score;   
    unsigned int bestIndex = 0;

    //search for the best score
    for (int i = 1; i < num; ++i)
    {
        if (list[i].score > bestScore)
        {
            bestScore = list[i].score;
            bestIndex = i;
        }
    }

    //remove this combo from future consideration by setting its score
    //very low
    list[bestIndex].score = -30000;
    
    return list[bestIndex];
}
