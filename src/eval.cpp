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
    short materialScore =    materialWeights[ELEPHANT][numBits(board.pieces[GOLD][ELEPHANT])]
                           + materialWeights[CAMEL][numBits(board.pieces[GOLD][CAMEL])]
                           + materialWeights[HORSE][numBits(board.pieces[GOLD][HORSE])]
                           + materialWeights[DOG][numBits(board.pieces[GOLD][DOG])]
                           + materialWeights[CAT][numBits(board.pieces[GOLD][CAT])]
                           + materialWeights[RABBIT][numBits(board.pieces[GOLD][RABBIT])]
                           - materialWeights[ELEPHANT][numBits(board.pieces[SILVER][ELEPHANT])]
                           - materialWeights[CAMEL][numBits(board.pieces[SILVER][CAMEL])]
                           - materialWeights[HORSE][numBits(board.pieces[SILVER][HORSE])]
                           - materialWeights[DOG][numBits(board.pieces[SILVER][DOG])]
                           - materialWeights[CAT][numBits(board.pieces[SILVER][CAT])]
                           - materialWeights[RABBIT][numBits(board.pieces[SILVER][RABBIT])];

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

    //scores for frozen pieces
    short frozenScore = 0;

    Int64 strongGold = board.pieces[GOLD][ELEPHANT];
    Int64 strongSilver = board.pieces[SILVER][ELEPHANT];

    Int64 notNearGold   = ~near(board.getAllPiecesOfColor(GOLD));
    Int64 notNearSilver = ~near(board.getAllPiecesOfColor(SILVER));

    for (int type = CAMEL; type < MAX_TYPES; type++)
    {
        frozenScore += frozenWeights[type - 1]
                                    [numBits(board.pieces[GOLD][type] 
                                    & near(strongSilver) & notNearGold)]
                     - frozenWeights[type - 1]
                                    [numBits(board.pieces[SILVER][type] 
                                    & near(strongGold) & notNearSilver)];

        //Update the strong pieces for the next type iteration
        strongGold   |= board.pieces[GOLD][type];
        strongSilver |= board.pieces[SILVER][type];

    }

    score = materialScore + positionScore + frozenScore;

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
void Eval :: scoreCombos(vector<StepCombo>& combos, unsigned char color)
{
    for (int i = 0; i < combos.size(); ++i)
    {
        combos[i].score = 0;

        //give moves some score based on captures, note that pieces are
        //numerically ordered with ELEPHANT being 0, and RABBIT being 5, so
        //to give a sensical score in respect to type, value should reversed
        //in respect to these numbers
        if (combos[i].piece1IsCaptured())
        {
            unsigned char piece = combos[i].getPiece1();
            if (colorOfPiece(piece) != color)
                combos[i].score += 100 * (MAX_TYPES - typeOfPiece(piece));
            else
                combos[i].score -= 100 * (MAX_TYPES - typeOfPiece(piece));
        }
        
        if (combos[i].piece2IsCaptured())
        {
            unsigned char piece = combos[i].getPiece2();
            if (colorOfPiece(piece) != color)
                combos[i].score += 100 * (MAX_TYPES - typeOfPiece(piece));
            else
                combos[i].score -= 100 * (MAX_TYPES - typeOfPiece(piece));
        }

        //give moves some score based on push/pulling moves
        if (combos[i].stepCost > 1 && combos[i].getFrom1() != ILLEGAL_SQUARE)
            combos[i].score += 25;

        //give moves a boost according to their history score
        combos[i].score += histTable.getScore(combos[i].getRawMove(), color);
    }
}

//////////////////////////////////////////////////////////////////////////////
//Load weights from the file specified
//////////////////////////////////////////////////////////////////////////////
void Eval :: loadWeights(string filename)
{
    ifstream fin(filename.c_str());

    if (!fin.is_open())
    {
        Error error;
        error << "From Eval :: loadWeights\n";
        error << "Couldn't open file " << filename << "\n";
        throw error;
    }

    //Material Weights////////////////////////////////////////////////////////
    for (int type = 0; type < MAX_TYPES; type++)
    {   
        int maxNum;

        switch (type)
        {
            case ELEPHANT:
            case CAMEL:
            {
                maxNum = 1;
            }break;

            case HORSE:
            case DOG:
            case CAT:
            {
                maxNum = 2;
            }break;

            case RABBIT:
            {
                maxNum = 8;
            }break;
        }

        string line;
        getline(fin, line);
        stringstream lineStream(line);

        materialWeights[type][0] = 0;
        for (int i = 0; i < maxNum; i++)
        {
            lineStream >> materialWeights[type][i + 1];
        }
    }
 
    //Read the extra separator line
    string line;
    getline(fin, line);

    //Position Weights////////////////////////////////////////////////////////
    
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

    //Frozen Penalties////////////////////////////////////////////////////////
    //Note that elephants are not included in this array
    for (int type = 0; type < MAX_TYPES - 1; type++)
    {   
        int maxNum;

        switch (type + 1) //increase type by 1, as type 0 is the elephant
        {
            case CAMEL:
            {
                maxNum = 1;
            }break;

            case HORSE:
            case DOG:
            case CAT:
            {
                maxNum = 2;
            }break;

            case RABBIT:
            {
                maxNum = 8;
            }break;
        }

        string line;
        getline(fin, line);
        stringstream lineStream(line);

        frozenWeights[type][0] = 0;

        for (int i = 0; i < maxNum; i++)
        {
            lineStream >> frozenWeights[type][i+1];
        }
    }

    fin.close();
}

//////////////////////////////////////////////////////////////////////////////
//Save weights to the specified file
//////////////////////////////////////////////////////////////////////////////   
void Eval :: saveWeights(string filename)
{
    ofstream fout(filename.c_str());

    //Material Weights////////////////////////////////////////////////////////
    for (int type = 0; type < MAX_TYPES; type++)
    {   
        int maxNum;

        switch (type)
        {
            case ELEPHANT:
            case CAMEL:
            {
                maxNum = 1;
            }break;

            case HORSE:
            case DOG:
            case CAT:
            {
                maxNum = 2;
            }break;

            case RABBIT:
            {
                maxNum = 8;
            }break;
        }

        for (int i = 0; i < maxNum; i++)
        {
            fout << materialWeights[type][i+1] << " ";
        }
        fout << endl;
    }

    //Write an extra separator line
    fout << endl;

    //Position Weights////////////////////////////////////////////////////////
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

    //Frozen Penalties////////////////////////////////////////////////////////
    //Note that elephants are not included in this array
    for (int type = 0; type < MAX_TYPES - 1; type++)
    {   
        int maxNum;

        switch (type + 1) //increase type by 1, as type 0 is the elephant
        {
            case CAMEL:
            {
                maxNum = 1;
            }break;

            case HORSE:
            case DOG:
            case CAT:
            {
                maxNum = 2;
            }break;

            case RABBIT:
            {
                maxNum = 8;
            }break;
        }

        for (int i = 0; i < maxNum; i++)
        {
            fout << frozenWeights[type][i+1] << " ";
        }
        fout << endl;
    }

    fout.close(); 
}

