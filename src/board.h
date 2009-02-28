#ifndef __JR__BOARD_H__
#define __JR__BOARD_H__
//The class that is the board representation

#include "int64.h"
#include "step.h"
#include "piece.h"
#include "square.h"
#include "hash.h"
#include <string>
#include <vector>

using namespace std;

class Board
{
    public:
    Board();
    ~Board();
    
    //functions///////////////////////////////////////////////////////////////

    void loadPositionFile(string filename);
    void setHashes(Int64 pieceParts[][MAX_TYPES][NUM_SQUARES], 
                   Int64 turnParts[]);

    bool isFrozen(unsigned char index, unsigned char piece);
    bool hasFriends(unsigned char index, unsigned char piece);

    unsigned char getPieceAt(unsigned char index);
    Int64 getAllPieces();
    Int64 getAllPiecesOfColor(unsigned char color);
    Int64 getAllPiecesThatOutrank(unsigned char type);
    Int64 getAllPiecesLower(unsigned char type);

    bool samePieces(Board& comp);

    void writePieceOnBoard(unsigned char index, unsigned char color,
                           unsigned char type);
    void removePieceFromBoard(unsigned char index, unsigned char color,
                              unsigned char type);

    void playStep(Step step);
    void undoStep(Step step);
    void playCombo(StepCombo& combo);
    void undoCombo(StepCombo& combo);

    void changeTurn();
    void unchangeTurn(unsigned int oldStepsLeft);

    unsigned int genMoves(StepCombo combos[]);

    short eval(unsigned char color);
    bool isWin(unsigned char color);

    //variables///////////////////////////////////////////////////////////////

    //bitboards to store positions of the pieces. first index is color, second 
    //index is piece type
    Int64 pieces[MAX_COLORS][MAX_TYPES];

    Int64 hashPieceParts[MAX_COLORS][MAX_TYPES][NUM_SQUARES];
    Int64 hashTurnParts[MAX_COLORS];

    //hashes
    Int64 hash; //key that is used to access the entry
                       

    unsigned char sideToMove; //current player to move
    unsigned int turnNumber;
    unsigned int stepsLeft;   //number of steps left in this move 
                              //available
};

//display operator
ostream& operator<<(ostream& out, Board b);
#endif
