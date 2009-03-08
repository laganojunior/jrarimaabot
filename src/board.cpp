#include "board.h"
#include "square.h"
#include "piece.h"
#include "error.h"
#include "int64.h"
#include "step.h"
#include "hash.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <sstream>
#include <vector>
#include <cstdlib>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//Basic constructor, basically initializes an empty board
//////////////////////////////////////////////////////////////////////////////
Board :: Board()
{
    reset();
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor, for now it doesn't need to do anything
//////////////////////////////////////////////////////////////////////////////
Board :: ~Board()
{

}

//////////////////////////////////////////////////////////////////////////////
//Resets the board to a new game position
//////////////////////////////////////////////////////////////////////////////
void Board :: reset()
{
    //zero out the bitboards, could use memset for this, but for now I'll
    //be more explicit
    for (int color = 0; color < MAX_COLORS; color++)
    {
        for (int piece = 0; piece < MAX_TYPES; piece++)
        {
            pieces[color][piece] = 0;
        }
    }

    sideToMove = GOLD;
    stepsLeft = 4;
    turnNumber = 1;

    hash = 0;
    hashPiecesOnly = 0;
}

//////////////////////////////////////////////////////////////////////////////
//Load a position file onto this board object. Throws an Error object
//if the given file could not be opened or the position file has an invalid 
//syntax.
//////////////////////////////////////////////////////////////////////////////
void Board :: loadPositionFile(string filename)
{

    ifstream in(filename.c_str());

    if (!in.is_open())
    {
        Error error;
        error << "From Board :: loadPositionFile(string)\n"
              << "Could not open file: "
              << filename << "\n";
        throw error;
    }

    string line;
    stringstream lineStream;
    string word;

    //read the first line, which should contain the turn number, the color
    //to move, and steps made this turn if any
    getline(in,line);
    lineStream.str(line);
    lineStream >> word;
    
    stringstream wordStream(word);
    if (!isdigit(word[0]))
    {
        Error error;
        error << "From Board :: loadPositionFile(string)\n"
              << "Expected turn number as first part in first line\n"
              << "Got: " << word << '\n';
        throw error;
    }
    wordStream >> turnNumber;

    unsigned char colorChar;
    wordStream >> colorChar;
    if (colorChar != 'w' && colorChar != 'b' && colorChar != 's' 
     && colorChar != 'g')
    {
        Error error;
        error << "From Board :: loadPositionFile(string)\n"
              << "Expected color as second part in first line\n"
              << "Got: " << colorChar << '\n';
        throw error;
    }

    if (colorChar == 'w' || colorChar == 'g')
        sideToMove = GOLD;
    else
        sideToMove = SILVER;

    //read off the steps on the first line if any    
    StepCombo initSteps;
    string rest;
    getline(lineStream, rest);
    initSteps.fromString(rest);


    //skip the next line
    getline(in,line);
    
    //zero out the bitboards before reading the pieces
    for (int i = 0; i < MAX_COLORS; i++)
    {
        for (int j = 0; j < MAX_TYPES; j++)
        {
            pieces[i][j] = 0;
        }
    }

    hash = 0;
    hashPiecesOnly = 0;

    //read the next 8 lines to read the board
    for (int i = 0; i < 8; i++)
    {
        getline(in,line);
        
        for (int j = 0; j < 8; j++)
        {
            char pieceChar = line[3 + 2 * j]; //skip first 3 characters
                                              //and then read every other
                                              //character afterward

            //check for traps or empty squares
            if (pieceChar == 'X' || pieceChar == 'x' || pieceChar == ' ')
                continue;
            
            unsigned char piece = pieceFromChar(pieceChar);
            unsigned char color = colorOfPiece(piece);
            unsigned char type  = typeOfPiece(piece);
            writePieceOnBoard(j + i * 8, color, type);
        }
    }

    hash ^= hashTurnParts[sideToMove];
    hash ^= hashStepsLeftParts[4];

    //attempt to play the stored steps
    stepsLeft = 4;
    playCombo(initSteps);

    in.close();
}

//////////////////////////////////////////////////////////////////////////////
//Sets the hash parts for use in creating hashes. Note this should be called
//before the board is set in any way for hashes to be consistent
//////////////////////////////////////////////////////////////////////////////
void Board :: setHashes(Int64 pieceParts[][MAX_TYPES][NUM_SQUARES], 
                        Int64 turnParts[], Int64 stepsLeftParts[])
{
    memcpy(hashPieceParts,pieceParts,sizeof(Int64) * MAX_COLORS * MAX_TYPES
                                                   * NUM_SQUARES);
    memcpy(hashTurnParts, turnParts, sizeof(Int64) * MAX_COLORS);
    memcpy(hashStepsLeftParts, stepsLeftParts, sizeof (Int64) * 5);
}

//////////////////////////////////////////////////////////////////////////////
//Checks if a piece on the square specified by index is frozen. Returns
//true if the piece is placed on that square would be frozen (that piece need 
//not be the actual piece on that square) and false otherwise
//////////////////////////////////////////////////////////////////////////////
bool Board :: isFrozen(unsigned char index, unsigned char piece)
{
    return (getAllPiecesThatOutrank(typeOfPiece(piece))
          & getAllPiecesOfColor(oppColorOf(colorOfPiece(piece)))
          & getNeighbors(index)) && (!hasFriends(index,piece));
    
}

//////////////////////////////////////////////////////////////////////////////
//Checks if a piece on the square specified by index has friendly pieces 
//neighboring it. Returns true if the piece is placed on that square would be 
//have friendly neighbors (that piece need not be the actual piece on that 
//square) and false otherwise
//////////////////////////////////////////////////////////////////////////////
bool Board :: hasFriends(unsigned char index, unsigned char piece)
{   
        
    return (bool) (getAllPiecesOfColor(colorOfPiece(piece)) 
                    & getNeighbors(index));
}

//////////////////////////////////////////////////////////////////////////////
//returns the piece at a specified index. returns NO_PIECE if there is no
//piece at that index
//////////////////////////////////////////////////////////////////////////////
unsigned char Board :: getPieceAt(unsigned char index)
{   
    Int64 b = Int64FromIndex(index);
    for (int color = 0; color < MAX_COLORS; color ++)
    {
        for (int type = 0; type < MAX_TYPES; type ++)
        {
            if (pieces[color][type] & b)
            {
                return genPiece(color,type);
            }
        }
    }

    return NO_PIECE;
}

//////////////////////////////////////////////////////////////////////////////
//returns the bitboard that represents all pieces on the board
//////////////////////////////////////////////////////////////////////////////
Int64 Board :: getAllPieces()
{
    Int64 all = 0;
    for (int color = 0; color < MAX_COLORS; color ++)
    {
        for (int type = 0; type < MAX_TYPES; type ++)
        {
            all |= pieces[color][type];
        }
    }
    return all;
}

//////////////////////////////////////////////////////////////////////////////
//returns the bitboard that represents all the pieces of the given color
//////////////////////////////////////////////////////////////////////////////
Int64 Board :: getAllPiecesOfColor(unsigned char color)
{
    Int64 allColor = 0;
    for (int type = 0; type < MAX_TYPES; type ++)
    {
        allColor |= pieces[color][type];
    }
    return allColor;   
}

//////////////////////////////////////////////////////////////////////////////
//returns the bitboard that represents all the pieces that outrank a given
//piece type
//////////////////////////////////////////////////////////////////////////////
Int64 Board :: getAllPiecesThatOutrank(unsigned char type)
{
    //Note outranking pieces have numeric values in piece.h LESS than the 
    //piece type specified
    Int64 outrank = 0;
    for (int i = 0; i < type; ++i)
    {
        outrank |= pieces[GOLD][i] | pieces[SILVER][i];
    }
    return outrank;
}

//////////////////////////////////////////////////////////////////////////////
//returns the bitboard that represents all pieces of rank lower than the 
//given piece type
//////////////////////////////////////////////////////////////////////////////
Int64 Board :: getAllPiecesLower(unsigned char type)
{
    Int64 lower = 0;
    for (int i = type + 1; i < MAX_TYPES; ++i)
    {
        lower |= pieces[GOLD][i] | pieces[SILVER][i];
    }

    return lower;
}

//////////////////////////////////////////////////////////////////////////////
//returns true if and only if the pieces are in the same positions as the
//in the board specified
//////////////////////////////////////////////////////////////////////////////
bool Board :: samePieces(Board& comp)
{
    for (int color = 0; color < MAX_COLORS; color++)
    {
        for (int type = 0; type < MAX_TYPES; type++)
        {
            if (comp.pieces[color][type] != pieces[color][type])
                return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//writes the piece defined by the inputs type and color to the index 
//specified. This function exists as it might be the case that it will be
//desired to update several bitboards at once (i.e. along with the piece 
//bitboard, maybe an all pieces bitboard or a color bitboard should be updated
//).
//////////////////////////////////////////////////////////////////////////////
void Board :: writePieceOnBoard(unsigned char index, unsigned char color,
                                 unsigned char type)
{
    pieces[color][type] |= Int64FromIndex(index);
    
    //update the hashes
    hash ^= hashPieceParts[color][type][index];
    hashPiecesOnly ^= hashPieceParts[color][type][index];
}

//////////////////////////////////////////////////////////////////////////////
//Remove a piece from the board. This function may update many bitboards at
//once
//////////////////////////////////////////////////////////////////////////////
void Board :: removePieceFromBoard(unsigned char index, unsigned char color,
                                   unsigned char type)
{
    pieces[color][type] ^= Int64FromIndex(index);

    hash ^= hashPieceParts[color][type][index];
    hashPiecesOnly ^= hashPieceParts[color][type][index];
}

//////////////////////////////////////////////////////////////////////////////
//Plays a step on this board. This function ASSUMES the step is legal.
//////////////////////////////////////////////////////////////////////////////
void Board :: playStep(Step step)
{
    hash ^= hashStepsLeftParts[stepsLeft];
    if (step.isPass())
    {

        --stepsLeft;
    }
    else if (step.isCapture())
    {
        //remove the piece from the board
        removePieceFromBoard(step.getFrom(),colorOfPiece(step.getPiece()),
                                            typeOfPiece(step.getPiece()));
    }
    else
    {
        //move the piece
        removePieceFromBoard(step.getFrom(),colorOfPiece(step.getPiece()),
                                            typeOfPiece(step.getPiece()));
        writePieceOnBoard(step.getTo(),colorOfPiece(step.getPiece()),
                                       typeOfPiece(step.getPiece()));
        
        --stepsLeft;
    }
    hash ^= hashStepsLeftParts[stepsLeft];
}

//////////////////////////////////////////////////////////////////////////////
//Undoes the last step played on this board.
//////////////////////////////////////////////////////////////////////////////
void Board :: undoStep(Step step)
{
    hash ^= hashStepsLeftParts[stepsLeft];
    if (step.isPass())
    {
        ++stepsLeft;
    }
    else if (step.isCapture())
    {
        //add the piece back to the board
        writePieceOnBoard(step.getFrom(),colorOfPiece(step.getPiece()),
                                         typeOfPiece(step.getPiece()));
    }
    else
    {
        //move back the piece
        removePieceFromBoard(step.getTo(),colorOfPiece(step.getPiece()),
                                            typeOfPiece(step.getPiece()));
        writePieceOnBoard(step.getFrom(),colorOfPiece(step.getPiece()),
                                       typeOfPiece(step.getPiece()));
        
        ++stepsLeft;
    }
    hash ^= hashStepsLeftParts[stepsLeft];
}

//////////////////////////////////////////////////////////////////////////////
//plays a step combo on this board
//////////////////////////////////////////////////////////////////////////////
void Board :: playCombo(StepCombo& combo)
{
    for (int i = 0; i < combo.numSteps; ++i)
        playStep(combo.steps[i]);
}

//////////////////////////////////////////////////////////////////////////////
//Undoes a step combo
//////////////////////////////////////////////////////////////////////////////
void Board :: undoCombo(StepCombo& combo)
{
    for (int i = combo.numSteps - 1; i >= 0; --i)
        undoStep(combo.steps[i]);
} 

//////////////////////////////////////////////////////////////////////////////
//Gives the turn to the other player, setting the state for a fresh move
//////////////////////////////////////////////////////////////////////////////
void Board :: changeTurn()
{
    //update the hashes
    hash ^= hashTurnParts[sideToMove];
    hash ^= hashTurnParts[oppColorOf(sideToMove)];

    hash ^= hashStepsLeftParts[stepsLeft];
    hash ^= hashStepsLeftParts[4];

    //set state to new turn
    stepsLeft = 4;
    sideToMove = oppColorOf(sideToMove);
}
    
//////////////////////////////////////////////////////////////////////////////
//Undoes a turn change. Note that the old number of steps is needed to 
//undo the change, as the player may have passed with steps left.
//////////////////////////////////////////////////////////////////////////////
void Board :: unchangeTurn(unsigned int oldStepsLeft)
{
    //set state to new turn
    stepsLeft = oldStepsLeft;
    sideToMove = oppColorOf(sideToMove);

    //update the hashes
    hash ^= hashTurnParts[sideToMove];
    hash ^= hashTurnParts[oppColorOf(sideToMove)];
    hash ^= hashStepsLeftParts[stepsLeft];
    hash ^= hashStepsLeftParts[oldStepsLeft];
}

//////////////////////////////////////////////////////////////////////////////
//Generates some set of steps combos that can played on the current board
//state. Note this will not generate all possible combos (that's a lot of 
//combos), but rather all 1-steps and push/pull combos, and perhaps some 
//interesting other multi-step combos. Returns the number of moves generated.
//
//Note the array passed must be of large size, as this function cannot
//and will not check if it passed the limit.
//////////////////////////////////////////////////////////////////////////////
unsigned int Board :: genMoves(StepCombo combos[])
{
    if (stepsLeft < 1) //no steps left to move
        return 0;

    Int64 allPieces = getAllPieces();

    int numCombos = 0;
    Step step;

    //array to check if the piece is moving from their position would result
    //in a capture. Its really used as a boolean, but zeroing out boolean
    //arrays is ambiguous sometimes as booleans are really stored as bytes.
    unsigned char capturesChecked[64];

    //some arrays that keep the pre-calculated data on moving leading to
    //captures. indexed by the square moving from.
    bool canKillItself[64];            //wheter a piece moving to a trap
                                       //would kill itself

    bool leadsToCapture[64];           //wheter a piece moving would lead
                                       //to a capture of another piece

    unsigned char pieceCaptured[64];   //what the other piece captured is

    unsigned char captureSquare[64];   //where the capture square is.

    //zero out these arrays
    memset(capturesChecked,0,sizeof(unsigned char) * 64);
    memset(canKillItself,0,sizeof(unsigned char) * 64);
    memset(leadsToCapture,0,sizeof(unsigned char) * 64);

    //Generate all 1 step combos, then extend to pulls if possible, and 
    //explore push options.

    //keep track of pieces that outrank this piece in order to check if the
    //piece is frozen. Note since the iteration of type starts with the
    //elephant, it starts with no pieces, but as the iteration goes down
    //piece rank, the outranking pieces are then added
    Int64 outrankingPieces = 0;

    Int64 friends = getAllPiecesOfColor(sideToMove);

    //iterate through all the piece types except the rabbit which is treated
    //differently as it can't move back and can't possibly push or pull.
    for (int type = 0; type < RABBIT; ++type)
    {
        Int64 bitboard = pieces[sideToMove][type];
        
        int from;
        //iterate through all pieces of that type
        while ((from = bitScanForward(bitboard)) != NO_BIT_FOUND)
        {
            bitboard ^= Int64FromIndex(from); //erase bit

            unsigned char piece = genPiece(sideToMove,type);

            //check if the piece that is frozen by making sure no outranking
            //pieces are near and there are friends nearby
            if (!(friends & getNeighbors(from)) && 
                (outrankingPieces & getNeighbors(from)))
                continue;

            //check if moving the piece leads to a capture. This is only
            //possible if the piece is next to a trap and kills itself by
            //moving to a trap, or leaves a friend alone on a trap.
            if (Int64 trapsNear = getTraps() & getNeighbors(from))
            {

                //remove piece and see if it would leave a friendly piece
                //alone on that trap.

                unsigned char killSquare = bitScanForward(trapsNear);

                removePieceFromBoard(from,sideToMove,type);
    
                //if friendly piece is lonely on trap, then piece can kill
                //itself. Also it would kill any friendly piece there by 
                //moving
                if (!hasFriends(killSquare,piece))
                {
                    unsigned char capturedPiece = getPieceAt(killSquare);

                    if (capturedPiece != NO_PIECE && 
                        colorOfPiece(capturedPiece) == sideToMove)
                    {
                        //piece on trap will get killed if this piece is moved
                        //so set moves from here to capture this piece. 
                        leadsToCapture[from] = true;
                        pieceCaptured[from] = capturedPiece;
                        captureSquare[from] = killSquare;
                    }
                    else 
                    {
                        //piece will kill itself if it moves onto trap
                        canKillItself[from] = true;
                        captureSquare[from] = killSquare;
                    }    
                }

                //write piece back

                writePieceOnBoard(from,sideToMove,type);
            }
            
            //get enemy lower pieces in range, and see if moving them
            //would capture enemy pieces, this is only possible if those
            //pieces are near traps
            Int64 lowNeighbors;
            if (stepsLeft > 1)
            {

                //get all lower enemies
                lowNeighbors = getAllPiecesLower(type) & getNeighbors(from) 
                             & getAllPiecesOfColor(oppColorOf(sideToMove));

                //get all of those lower enemies near a trap
                Int64 lowNeighborsNearTraps = lowNeighbors 
                                            & getTrapNeighbors();
    
                //cycle through all such enemies
                int nearTrap;
                while ((nearTrap = bitScanForward(lowNeighborsNearTraps))
                        != NO_BIT_FOUND)
                {
                    //remove the bit
                    lowNeighborsNearTraps ^= Int64FromIndex(nearTrap);

                    //check if this square is already checked and set it so
                    //if not.
                    if (capturesChecked[nearTrap])
                        continue;
        
                    capturesChecked[nearTrap] = 1;

                    //get the piece
                    unsigned char enemyNear = getPieceAt(nearTrap);
                    
                    //get the trap that the enemy is near
                    int trapSquareNear = bitScanForward(getNeighbors(nearTrap) 
                                                      & getTraps());

                    //temporarily remove piece
                    removePieceFromBoard(nearTrap,oppColorOf(sideToMove)
                                                 ,typeOfPiece(enemyNear));

                    //check if it would make that trap have no friends
                    if (!hasFriends(trapSquareNear,enemyNear))
                    {
                        unsigned char capturedPiece;

                        //check if friendly piece is on the trap. Note that
                        //the push/puller can be on the trap square, so if
                        //an enemy piece is there, still consider this
                        //piece eligible to kill itself

                        capturedPiece = getPieceAt(trapSquareNear);

                        if (capturedPiece != NO_PIECE && 
                            colorOfPiece(capturedPiece) == 
                            oppColorOf(sideToMove))
                        {
                            //trap square is occupied by friendly piece, 
                            //and that piece could get
                            //captured if this is moved
                            leadsToCapture[nearTrap] = true;
                            pieceCaptured[nearTrap] = capturedPiece;
                            captureSquare[nearTrap] = trapSquareNear;
                        }
                        else
                        {
                            //piece is captured if it gets moved onto the
                            //trap
                            canKillItself[nearTrap] = true;
                            captureSquare[nearTrap] = trapSquareNear;
                        }
                    }

                    //write the piece back
                    writePieceOnBoard(nearTrap,oppColorOf(sideToMove)
                                                 ,typeOfPiece(enemyNear));
                        
                }
            }

            //get all the empty neighbors and try to move to them.
            Int64 freeNeighbors = getNeighbors(from) & ~allPieces;

            int to;
            Step step;

            //generate 1 step combos and extend those to pulls
            while ((to = bitScanForward(freeNeighbors)) != NO_BIT_FOUND)
            {
                //erase this bit from the bitboard.
                freeNeighbors ^= Int64FromIndex(to);

                //write the 1-step///////////////////////////
                combos[numCombos].reset();
    
                //write the moving step
                step.genMove(piece,from,to);
                combos[numCombos].addStep(step);

                //write capture steps if necessary
                if (canKillItself[from] && to == captureSquare[from])
                {
                    step.genCapture(piece, captureSquare[from]);
                    combos[numCombos].addStep(step);

                    //note that piece is captured for scoring purposes
                    combos[numCombos].hasFriendlyCapture = true;
                    combos[numCombos].friendlyCaptureType = type;                    
                }
                else if (leadsToCapture[from])
                {
                    step.genCapture(pieceCaptured[from], captureSquare[from]);
                    combos[numCombos].addStep(step);

                    //note that piece is captured for scoring purposes
                    combos[numCombos].hasFriendlyCapture = true;
                    combos[numCombos].friendlyCaptureType 
                                       = typeOfPiece(pieceCaptured[from]);
                }
                else
                    combos[numCombos].hasFriendlyCapture = false;
                
                StepCombo prefix = combos[numCombos]; //keep this combo 
                                                      //to prefix pulls.

                ++numCombos;//note that piece is captured for scoring purposes

                //write pulls for every lower neighbor. Keep a copy of the
                //bitboard as it is used again for pushes.
                if (stepsLeft > 1)
                {
                    Int64 tempLowNeighbors = lowNeighbors;
                    int lowerSquare;
                    while ((lowerSquare = bitScanForward(tempLowNeighbors)) 
                            != NO_BIT_FOUND)
                    {   
                        tempLowNeighbors ^= Int64FromIndex(lowerSquare);

                        unsigned char lowerPiece = getPieceAt(lowerSquare);

                        //copy the prefix to start the pull
                        combos[numCombos] = prefix;
                    
                        //add the pulling move, note the lower piece moves 
                        //onto the square this moves from.
                        step.genMove(lowerPiece,lowerSquare,from);
                        combos[numCombos].addStep(step);
        
                        //add necessary captures

                        if (canKillItself[lowerSquare] &&
                            from == captureSquare[lowerSquare])
                        {
                            step.genCapture(lowerPiece, 
                                            captureSquare[lowerSquare]);

                            combos[numCombos].addStep(step);

                            //note that piece is captured for scoring purposes
                            combos[numCombos].hasEnemyCapture = true;
                            combos[numCombos].enemyCaptureType 
                                              = typeOfPiece(lowerPiece); 
                        }
                        else if (leadsToCapture[lowerSquare])
                        {
                            step.genCapture(pieceCaptured[lowerSquare], 
                                            captureSquare[lowerSquare]);

                            combos[numCombos].addStep(step);

                            //note that piece is captured for scoring purposes
                            combos[numCombos].hasEnemyCapture = true;
                            combos[numCombos].enemyCaptureType 
                                    = typeOfPiece(pieceCaptured[lowerSquare]); 
                        }
                        else
                            combos[numCombos].hasEnemyCapture = false;
                    
                        numCombos++;
                    }
                }   
            }

            //now generate pushes
            if (stepsLeft > 1)
            {
                //iterate through all lower pieces
                int lowerSquare;
                while ((lowerSquare = bitScanForward(lowNeighbors)) 
                        != NO_BIT_FOUND)
                {
                    lowNeighbors ^= Int64FromIndex(lowerSquare);

                    unsigned char lowerPiece = getPieceAt(lowerSquare);

                    //iterate through all squares this lower piece can be
                    //pushed to.

                    Int64 freePushSquares = getNeighbors(lowerSquare)  
                                          & ~allPieces;

                    int pushSquare;
                    while ((pushSquare = bitScanForward(freePushSquares))
                           != NO_BIT_FOUND)
                    {
                        freePushSquares ^= Int64FromIndex(pushSquare);

                        combos[numCombos].reset();

                        //add the pushing step
                        step.genMove(lowerPiece,lowerSquare,pushSquare);
                        combos[numCombos].addStep(step);

                        //add necessary captures
                        
                        if (canKillItself[lowerSquare] &&
                            pushSquare == captureSquare[lowerSquare])
                        {
                            step.genCapture(lowerPiece, 
                                            captureSquare[lowerSquare]);

                            combos[numCombos].addStep(step);

                            //note that piece is captured for scoring purposes
                            combos[numCombos].hasEnemyCapture = true;
                            combos[numCombos].enemyCaptureType 
                                              = typeOfPiece(lowerPiece); 
                              
                        }
                        else if (leadsToCapture[lowerSquare])
                        {
                            step.genCapture(pieceCaptured[lowerSquare], 
                                            captureSquare[lowerSquare]);

                            combos[numCombos].addStep(step);

                            //note that piece is captured for scoring purposes
                            combos[numCombos].hasEnemyCapture = true;
                            combos[numCombos].enemyCaptureType 
                                    = typeOfPiece(pieceCaptured[lowerSquare]); 
                        }
                        else
                            combos[numCombos].hasEnemyCapture = false;                            
                        

                        //complete the push by moving onto the square the
                        //enemy piece was pushed from
                        step.genMove(piece,from,lowerSquare);
                        combos[numCombos].addStep(step);

                        //write capture steps if necessary
                        if (canKillItself[from] 
                            && lowerSquare == captureSquare[from])
                        {
                            step.genCapture(piece, captureSquare[from]);
                            combos[numCombos].addStep(step);
    
                            //note that piece is captured for scoring purposes
                            combos[numCombos].hasFriendlyCapture = true;
                            combos[numCombos].friendlyCaptureType = type; 
                        }
                        else if (leadsToCapture[from])
                        {
                            step.genCapture(pieceCaptured[from], 
                            captureSquare[from]);
                            combos[numCombos].addStep(step);

                            //note that piece is captured for scoring purposes
                            combos[numCombos].hasFriendlyCapture = true;
                            combos[numCombos].friendlyCaptureType 
                                       = typeOfPiece(pieceCaptured[from]);
                        }
                        else
                            combos[numCombos].hasFriendlyCapture = false;

                        ++numCombos;
                    }
                }
            }
        }

        //before moving on to the next lower type, add the next lower 
        //outranking piece type
        outrankingPieces |= pieces[oppColorOf(sideToMove)][type];
    }

    //Generate Rabbit Moves separately, do not check for push/pulls
    Int64 rabbits = pieces[sideToMove][RABBIT];
    int from;
    unsigned char piece = genPiece(sideToMove,RABBIT);

    while ((from = bitScanForward(rabbits)) != NO_BIT_FOUND)
    {
        //remove the bit
        rabbits ^= Int64FromIndex(from);
    
        unsigned char piece = genPiece(sideToMove,RABBIT);
        //check if the piece that is frozen by making sure no outranking
        //pieces are near and there are friends nearby
        if (!(friends & getNeighbors(from)) && 
            (outrankingPieces & getNeighbors(from)))
            continue;

        //check if moving the piece leads to a capture. This is only
        //possible if the piece is next to a trap and kills itself by
        //moving to a trap, or leaves a friend alone on a trap.
        if (Int64 trapsNear = getTraps() & getNeighbors(from))
        {
            //remove piece and see if it would leave a friendly piece
            //alone on that trap.

            unsigned char killSquare = bitScanForward(trapsNear);

            removePieceFromBoard(from,sideToMove,RABBIT);

            //if friendly piece is lonely on trap, then piece can kill
            //itself. Also it would kill any friendly piece there by 
            //moving
            if (!hasFriends(killSquare,piece))
            {
                
                unsigned char capturedPiece;

                if ((capturedPiece = getPieceAt(killSquare)) != NO_PIECE)
                {
                    //piece on trap will get killed if this piece is moved
                    //so set moves from here to capture this piece. 
                    if (colorOfPiece(capturedPiece) == sideToMove)
                    {
                        leadsToCapture[from] = true;
                        pieceCaptured[from] = capturedPiece;
                        captureSquare[from] = killSquare;
                    }
                }
                else 
                {
                    //piece will kill itself if it moves onto trap
                    canKillItself[from] = true;
                    captureSquare[from] = killSquare;
                }    
            }

            //write piece back

            writePieceOnBoard(from,sideToMove,RABBIT);
        }

        //get all the empty neighbors and try to move to them. Note that the
        //neighbors for rabbits are different than regular pieces, and 
        //depends on the color
        Int64 freeNeighbors;
        if (sideToMove == GOLD)
            freeNeighbors = getNeighborsUp(from) & ~allPieces;
        else
            freeNeighbors = getNeighborsDown(from) & ~allPieces;

        int to;
        Step step;

        //generate all 1-steps
        while ((to = bitScanForward(freeNeighbors)) != NO_BIT_FOUND)
        {
            //erase this bit from the bitboard.
            freeNeighbors ^= Int64FromIndex(to);

            combos[numCombos].reset();

            //write the moving step
            step.genMove(piece,from,to);
            combos[numCombos].addStep(step);

            //write capture steps if necessary
            if (canKillItself[from] && to == captureSquare[from])
            {
                step.genCapture(piece, captureSquare[from]);
                combos[numCombos].addStep(step);

                //note that piece is captured for scoring purposes
                combos[numCombos].hasFriendlyCapture = true;
                combos[numCombos].friendlyCaptureType = RABBIT;   
            }
            else if (leadsToCapture[from])
            {
                step.genCapture(pieceCaptured[from], captureSquare[from]);
                combos[numCombos].addStep(step);

                //note that piece is captured for scoring purposes
                combos[numCombos].hasFriendlyCapture = true;
                combos[numCombos].friendlyCaptureType = 
                                             typeOfPiece(pieceCaptured[from]);   
            }
            else
                combos[numCombos].hasFriendlyCapture = false;                

            ++numCombos;
        }
    }

    return numCombos;
}

//////////////////////////////////////////////////////////////////////////////
//Attempt to generate the 1-step move using the from and to square specified. 
//If it is a legal move, then it is written onto the combo given and true is
//return. If it is not, false is returned.
//////////////////////////////////////////////////////////////////////////////
bool Board :: gen1Step(StepCombo& combo, unsigned char from, unsigned char to)
{
    //check if there are steps available 
    if (stepsLeft < 1)
        return false;

    //Check if there is a piece there on the current side to move
    unsigned char piece = getPieceAt(from);

    if (piece == NO_PIECE || colorOfPiece(piece) != sideToMove)
        return false;

    //Check if this piece is frozen
    if (isFrozen(from, piece))
        return false;

    //Check if the destination square is occupied
    if (getAllPieces() & Int64FromIndex(to))
        return false;

    //At this point, it should be a legal move, so generate it
    combo.reset();
    Step step;
    step.genMove(piece,from,to);
    combo.addStep(step);

    //check if the move leads to capture
    Step captureStep;
    if (moveLeadsToCapture(step,captureStep))
        combo.addStep(captureStep);

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//Attempt to generate the 2 step move, specified by the 2 from/to pairs in
//order. If it can be done, the move is written onto the combo given, and
//true is returned. If not, false is returned
//////////////////////////////////////////////////////////////////////////////
bool Board :: gen2Step(StepCombo& combo, unsigned char from1, 
                       unsigned char to1, unsigned char from2)
{   
    //Check if 2 steps can be done
    if (stepsLeft < 2)
        return false;

    //Check if there are pieces on the two squares
    unsigned char piece1 = getPieceAt(from1);
    unsigned char piece2 = getPieceAt(from1);

    if (piece1 == NO_PIECE || piece2 == NO_PIECE)
        return false;

    combo.reset();

    //Branch off wheter this is a push or pull
    if (colorOfPiece(piece1) == sideToMove)
    {
        // first piece is from the player to move, this is a pulled

        // check if the other piece is of the opponent and can be pulled.
        // Note that higher ranking pieces have lower numerical type values
        if (colorOfPiece(piece2) == sideToMove ||
            typeOfPiece(piece2) <= typeOfPiece(piece1))
            return false;
    }
    else
    {
        // this is a push

        // check if the other piece is of the side to move and can push
        // the first piece
        // Note that higher ranking pieces have lower numerical type values
        if (colorOfPiece(piece2) != sideToMove ||
            typeOfPiece(piece2) >= typeOfPiece(piece1))
            return false;
    }   


    // Generate the pushing move, and check if that leads to any captures
    Step step, captureStep;
    step.genMove(piece1, from1, to1);
    combo.addStep(step);

    if (moveLeadsToCapture(step, captureStep))
        combo.addStep(captureStep);

    // Generate the step to finish the pull, and check for captures
    step.genMove(piece2, from2, from1);
    combo.addStep(step);

    if (moveLeadsToCapture(step, captureStep))
        combo.addStep(captureStep); 

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//Check if a step would lead to a capture immediately after being played.
//If it does, the capture step is written on the variable given,
//and true is returned. If not, false is returned.
//////////////////////////////////////////////////////////////////////////////
bool Board :: moveLeadsToCapture(Step& step, Step& captureStep)
{
    unsigned char from = step.getFrom();
    unsigned char to = step.getTo();
    unsigned char piece = step.getPiece();
    unsigned char color = colorOfPiece(piece);

    bool capture = false;
    //If it leads to immediate capture, it must be that piece is moving from
    //near a trap
    if (Int64 trapsNear = getTraps() & getNeighbors(from))
    {
        //temporarily move the piece
        removePieceFromBoard(from, color, typeOfPiece(piece));
        writePieceOnBoard(to, color, typeOfPiece(piece));

        //check if the trap square is now without any friends
        unsigned char trap = bitScanForward(trapsNear);
        unsigned char trappedPiece = getPieceAt(trap);
        if (trappedPiece != NO_PIECE && 
            colorOfPiece(trappedPiece) == color
            && !hasFriends(trap, trappedPiece))
        {
            //write the capture
            captureStep.genCapture(trappedPiece, trap);
            capture = true;
        }

        //move back the piece
        removePieceFromBoard(to, sideToMove, typeOfPiece(piece));
        writePieceOnBoard(from, sideToMove, typeOfPiece(piece));
    }

    return capture;
}


//////////////////////////////////////////////////////////////////////////////
//Prints the board onto the stream designated by out
//////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& out, Board b)
{
    //print out the turn number and side to move
    out << b.turnNumber << charFromColor(b.sideToMove) << endl;

    char pieceArray[64]; //displayable array that the pieces will eventually
                         //be placed onto
    
    //clear out the array, using ' ' as empty square
    for (int i = 0; i < 64; i ++)
        pieceArray[i] = ' ';

    //now scan the bitboards, placing the appropriate piece on the array
    //if bits are found
    for (int color = 0; color < MAX_COLORS; color ++)
    {
        for (int type = 0; type < MAX_TYPES; type ++)
        {
            Int64 bitboard = b.pieces[color][type];
            int index;
            while ((index = bitScanForward(bitboard)) != -1)
            {
                //write the character onto the array
                pieceArray[index] = charFromPiece(genPiece(color,type));
                
                //delete the bit found from the bitboard
                bitboard ^= Int64FromIndex(index);
            }
        }
    }
    
    out <<  " +-----------------+\n";

    //display the array obtained
    for (int row = 0; row < 8; row++)
    {
        out<< 8 - row << "| ";
        for (int column = 0; column < 8; column ++)
        {
            out << pieceArray[column + row * 8] << ' ';
        }
        
        out << '|' << endl;
    }

    out << " +-----------------+\n";
    out << "   a b c d e f g h\n";

    return out;
}                    
