#include "piece.h"
#include "error.h"
#include "square.h"
#include "step.h"
#include <assert.h>
#include <ctype.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//initialize a move given a string that describes the move. Throws an Error
//if the string is an invalid format, or the step is obviously illegal even
//without any knowledge about other pieces (e.g. a piece is moving off the
//board or a piece is being captured on a non-capture square
//////////////////////////////////////////////////////////////////////////////
void Step :: fromString(string str)
{
    if (str.length() != 4  || !isalpha(str[0]) || !isalpha(str[1]) 
    || !isdigit(str[2]) || !isalpha(str[3]))
    {
        Error error;
        error << "From Step :: fromString(string str)\n";
        error << "Invalid Format!\n";
        error << "Got: " << str << '\n';
        
        throw error;
    }
    
    //read the piece type and color
    unsigned char piece = pieceFromChar(str[0]);
    
    //read the position moving from
    stringstream squareStringStream;
    squareStringStream << str[1] << str[2];
    unsigned int from = squareFromString(squareStringStream.str());

    //read the last character for move type
    bool goingOffBoard = false;
    bool illegalCapture = false;
    bool illegalChar = false;
    int row = from / 8;
    int col = from % 8;
    int to;
    switch (str[3])
    {
        case 'n':
        {
            if (row == 0)
                goingOffBoard = true;
            
            to = from - 8;            
        }break;
        case 's':
        {
            if (row == 7)
                goingOffBoard = true;
            
            to = from + 8;
        }break;
        case 'w':
        {
            if (col == 0)
                goingOffBoard = true;
            
            to = from - 1;
        }break;
        case 'e':
        {
            if (col == 7)
                goingOffBoard = true;
            
            to = from + 1;
        }break;
        case 'x':
        {
            if (!isCaptureSquare(from))
                illegalCapture = true;
            
            to = -1;
            
        }break;  
            
        default:
        {
            illegalChar = true;  
        }       
    }

    //Something went wrong, throw an exception
    if (goingOffBoard || illegalCapture || illegalChar)
    {
        Error error;
        if (goingOffBoard)
        {
            error << "From Step :: fromString(string str)\n"
                  << "Illegal Move. Piece moves off board\n"
                  << "Got " << str << '\n';
        }
        else if (illegalCapture)
        {
            error << "From Step :: fromString(string str)\n"
                  << "Invalid capture from non-trap square\n"
                  << "Got " << str << '\n';

        }
        else
        {
            error << "From Step :: fromString(string str)\n"
                  << "Invalid move type character\n"
                  << "Got " << str[3] << " from " << str << '\n';
        }

        throw error;
    }

    //generate the step
    if (to == -1)
        genCapture(piece,from);
    else
        genMove(piece,from,to);
} 

//////////////////////////////////////////////////////////////////////////////
//returns the string representation of this step
//////////////////////////////////////////////////////////////////////////////
string Step :: toString()
{

    char moveType = '?'; //If this character remains unchanged, something is
                         //wrong

    unsigned char to = getTo();
    unsigned char from = getFrom();

    if (to - from == 1)
        moveType = 'e';

    if (to - from == -1)
        moveType = 'w';

    if (to - from == -8)
        moveType = 'n';

    if (to - from == 8)
        moveType = 's';

    if (isCapture())
        moveType = 'x';

    stringstream stepStream;
    stepStream << charFromPiece(getPiece()) << stringFromSquare(from)
               << moveType;
    
    return stepStream.str();
}

//////////////////////////////////////////////////////////////////////////////
//Constructor. Sets the combo to be the empty combo
//////////////////////////////////////////////////////////////////////////////
StepCombo :: StepCombo()
{
    reset();
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor. Does nothing.
//////////////////////////////////////////////////////////////////////////////
StepCombo :: ~StepCombo()
{

}

//////////////////////////////////////////////////////////////////////////////
//returns a string representing this combo
//////////////////////////////////////////////////////////////////////////////
string StepCombo :: toString()
{
    stringstream stream;
    for (int i = 0; i < numSteps - 1; ++i)
    {
        stream << steps[i].toString() << " ";
    }

    stream << steps[numSteps - 1].toString();
    return stream.str();
}

//////////////////////////////////////////////////////////////////////////////
//sets this step combo using the given string, which should be a string
//of whitespace separated steps
//////////////////////////////////////////////////////////////////////////////
void StepCombo :: fromString(string s)
{
    stringstream stepStream;
    stepStream.str(s);

    reset();
    while (stepStream.good())
    {
        string word;
        stepStream >> word;

        if (stepStream.fail())
            break;

        Step step;
        step.fromString(word);
        addStep(step);
    }
}

//////////////////////////////////////////////////////////////////////////////
//adds a step to this combo.
//////////////////////////////////////////////////////////////////////////////
void StepCombo :: addStep(Step step)
{   
    if (numSteps >= MAX_STEPS_IN_COMBO)
    {
        cerr << toString() << endl;
    }

    assert(numSteps < MAX_STEPS_IN_COMBO);

    steps[numSteps] = step;
    ++ numSteps;

    if (!step.isCapture())
        ++ stepCost;
}

//////////////////////////////////////////////////////////////////////////////
//adds a combo of steps to this combo
//////////////////////////////////////////////////////////////////////////////
void StepCombo :: addCombo(StepCombo& combo)
{
    assert(numSteps + combo.numSteps <= MAX_STEPS_IN_COMBO);

    for (int i = 0; i < combo.numSteps; i++)
    {
        steps[numSteps + i] = combo.steps[i];
    }
    
    numSteps += combo.numSteps;
    stepCost += combo.stepCost;
}

//////////////////////////////////////////////////////////////////////////////
//Return the source square that the first moving piece is moving from. Returns
//ILLEGAL_SQUARE if there is no such square
//////////////////////////////////////////////////////////////////////////////
unsigned char StepCombo :: getFrom1()
{
    if (stepCost < 1)
        return ILLEGAL_SQUARE;

    return steps[0].getFrom();
}

//////////////////////////////////////////////////////////////////////////////
//Return the destination square that the first piece is moving to. Returns
//ILLEGAL_SQUARE if there is no such square
//////////////////////////////////////////////////////////////////////////////
unsigned char StepCombo :: getTo1()
{
    if (stepCost < 1)
        return ILLEGAL_SQUARE;
    
    return steps[0].getTo();
}

//////////////////////////////////////////////////////////////////////////////
//Return the source square that the second piece is moving from. Note that
//there is no getTo2() function as it is assumed that the second piece is
//moving to the square that the first piece moved from
//////////////////////////////////////////////////////////////////////////////
unsigned char StepCombo :: getFrom2()
{
    if (stepCost < 2)
        return ILLEGAL_SQUARE;

    //note that second step can denote a capture, so the actual
    //second moving step can be afterward
    unsigned int secondMoveIndex = 1;

    if (steps[1].isCapture())
        secondMoveIndex = 2;

    return steps[secondMoveIndex].getFrom();
}

//////////////////////////////////////////////////////////////////////////////
//Returns a raw move structure depicting up to the first 2 moves of this
//combo
//////////////////////////////////////////////////////////////////////////////
RawMove StepCombo :: getRawMove()
{
    if (stepCost < 2)
        return RawMove(1, getFrom1(), getTo1(), ILLEGAL_SQUARE);
    else
        return RawMove(2, getFrom1(), getTo1(), getFrom2());
}

//////////////////////////////////////////////////////////////////////////////
//resets this combo to a blank combo
//////////////////////////////////////////////////////////////////////////////
void StepCombo :: reset()
{
    numSteps = 0;
    stepCost = 0;
    score = 0;
    hasFriendlyCapture = false;
    friendlyCaptureType = false;
    hasEnemyCapture = false;
    enemyCaptureType = false;
}
    
