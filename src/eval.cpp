#include "eval.h"
#include "board.h"
#include "int64.h"
#include "error.h"
#include <fstream>
#include <list>
#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//Clear all stored data from a previous search such as history score data.
//////////////////////////////////////////////////////////////////////////////
void Eval :: reset()
{
    histTable.reset();
}

//////////////////////////////////////////////////////////////////////////////
//Returns a static evaluation score of the board in the
//perspective of the player given
//////////////////////////////////////////////////////////////////////////////
short Eval :: evalBoard(Board& board, unsigned char color)
{
    //do scores assuming GOLD's perspective. if it is SILVER that is really
    //desired, then just negate at the end as this is a zero sum game

    short score;
    
    //material
    short materialScore =    numBits(board.pieces[GOLD][ELEPHANT]) * 1200
                           + numBits(board.pieces[GOLD][CAMEL]) * 700 
                           + numBits(board.pieces[GOLD][HORSE]) * 500 
                           + numBits(board.pieces[GOLD][DOG]) * 300 
                           + numBits(board.pieces[GOLD][CAT]) * 200
                           + numBits(board.pieces[GOLD][RABBIT]) * 100
                           - numBits(board.pieces[SILVER][ELEPHANT]) * 1200
                           - numBits(board.pieces[SILVER][CAMEL]) * 700 
                           - numBits(board.pieces[SILVER][HORSE]) * 500 
                           - numBits(board.pieces[SILVER][DOG]) * 300 
                           - numBits(board.pieces[SILVER][CAT]) * 200
                           - numBits(board.pieces[SILVER][RABBIT]) * 100;

    //scores for static positions
    short positionScore = 0;

    for (int pieceColor = 0; pieceColor < MAX_COLORS; ++pieceColor)
    {
        for (int type = 0; type < MAX_TYPES; ++type)
        {
            Int64 b = board.pieces[pieceColor][type];
            int pos;

            //Go through all squares where this particular piece is
            while ((pos = bitScanForward(b)) != NO_BIT_FOUND)
            {
                b ^= Int64FromIndex(pos);
                
                //modify the score accordingly
                if (pieceColor == GOLD)
                    positionScore += posWeights[GOLD][type][pos];
                else
                    positionScore -= posWeights[SILVER][type][pos];
            }
        }
    }

    score = materialScore + positionScore;

    if (color == GOLD)
        return score;
    else
        return -score;
}

//////////////////////////////////////////////////////////////////////////////
//returns true if the position is a win for the color specified.
//////////////////////////////////////////////////////////////////////////////
bool Eval :: isWin(Board& board, unsigned char color)
{
    if (color == GOLD)
        return board.pieces[GOLD][RABBIT] & getRow(0);
    else
        return board.pieces[SILVER][RABBIT] & getRow(7);
}

//////////////////////////////////////////////////////////////////////////////
//gives the combo in the array some heurisitic scores to see which order the
//moves should be considered in.
//////////////////////////////////////////////////////////////////////////////
void Eval :: scoreCombos(list<StepCombo>& combos, unsigned char color)
{
    for (list<StepCombo> :: iterator i = combos.begin(); 
         i != combos.end(); ++i)
    {
        i->score = 0;

        //give moves some score based on captures, note that pieces are
        //numerically ordered with ELEPHANT being 0, and RABBIT being 5, so
        //to give a sensical score in respect to type, value should reversed
        //in respect to these numbers
        if (i->piece1IsCaptured())
        {
            unsigned char piece = i->getPiece1();
            if (colorOfPiece(piece) != color)
                i->score += 100 * (MAX_TYPES - typeOfPiece(piece));
            else
                i->score -= 100 * (MAX_TYPES - typeOfPiece(piece));
        }
        
        if (i->piece2IsCaptured())
        {
            unsigned char piece = i->getPiece2();
            if (colorOfPiece(piece) != color)
                i->score += 100 * (MAX_TYPES - typeOfPiece(piece));
            else
                i->score -= 100 * (MAX_TYPES - typeOfPiece(piece));
        }

        //give moves some score based on push/pulling moves
        if (i->stepCost > 1 && i->getFrom1() != ILLEGAL_SQUARE)
            i->score += 25;

        //give moves a boost according to their history score
        i->score += histTable.getScore(i->getRawMove(), color);
    }
}

//////////////////////////////////////////////////////////////////////////////
//Loads all weight files
//////////////////////////////////////////////////////////////////////////////
void Eval :: loadWeights()
{
    loadPositionWeights("evalWeights/positionWeights.txt");
}

//////////////////////////////////////////////////////////////////////////////
//Load position weights from the specified file
//////////////////////////////////////////////////////////////////////////////
void Eval :: loadPositionWeights(string filename)
{
    ifstream fin(filename.c_str());

    if (!fin.is_open())
    {
        Error error;
        error << "From Eval :: loadPositionWeights\n";
        error << "Couldn't open file " << filename << "\n";
        throw error;
    }

    //Load the weights from the file, which are all in gold's perspective,
    //and only describe the left hand side of the board.
    for (int type = 0; type < MAX_TYPES; type++)
    {
        //Read in the values
        for (int row = 0; row < 8; row++)
        {
            string line;
            getline(fin, line);
            stringstream lineStream(line);
            
            for (int col = 0; col < 4; col++)
            {
                short weight;
                lineStream >> weight;

                //write two values for gold, one for each side of the board.
                posWeights[GOLD][type][row * 8 + col] = weight;
                posWeights[GOLD][type][row * 8 + 7 - col] = weight;

                //write two values for silver, which are just mirrored 
                //horizontally from gold's positions
                posWeights[SILVER][type][(7 - row) * 8 + col] = weight;
                posWeights[SILVER][type][(7 - row) * 8 + 7 - col] = weight;
            }
        }
        
        //Read the extra separator line
        string line;
        getline(fin, line);
    }

    fin.close();
}

//////////////////////////////////////////////////////////////////////////////
//Save position weights to the specified file
//////////////////////////////////////////////////////////////////////////////   
void Eval :: savePositionWeights(string filename)
{
    ofstream fout(filename.c_str());

    for (int type = 0; type < MAX_TYPES; type++)
    {               
        for (int row = 0; row < 8; row ++)
        {
            for (int col = 0; col < 4; col++)
            {
                fout << posWeights[GOLD][type][row * 8 + col] << " ";
            }
            
            fout << endl;
        }

        //Write an extra separator line
        fout << endl;
    }
    fout.close(); 
}

