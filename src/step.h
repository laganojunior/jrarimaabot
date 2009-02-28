#ifndef __JR_STEP_H__
#define __JR_STEP_H__

#include "piece.h"
#include <string>

#define MAX_STEPS_IN_COMBO 20

using namespace std;

//representation of a single step.
class Step
{
    public:
    void fromString(string str);
    string toString();

    //////////////////////////////////////////////////////////////////////////
    //returns the piece
    //////////////////////////////////////////////////////////////////////////
    unsigned char getPiece()
    {
        return data & 0XF; //first 4 bits
    }


    //////////////////////////////////////////////////////////////////////////
    //returns the square piece is moving from
    //////////////////////////////////////////////////////////////////////////
    unsigned char getFrom()
    {
        return (data >> 4) & 0x3F; //the 6 bits after the first 4
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the destination square. Note that this is the same as the from
    //square if this is a capture
    //////////////////////////////////////////////////////////////////////////
    unsigned char getTo()
    {
        return (data >> 10) & 0x3F; //the last 6 bits
    }


    //////////////////////////////////////////////////////////////////////////
    //returns true if and only if this is a capture move
    //////////////////////////////////////////////////////////////////////////
    bool isCapture()
    {
        return getFrom() == getTo() && getPiece() != NO_PIECE;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns true if and only if this is a pass
    //////////////////////////////////////////////////////////////////////////
    bool isPass()
    {
        return getPiece() == NO_PIECE;
    }

    //////////////////////////////////////////////////////////////////////////
    //sets this step to be a movement between two squares
    //////////////////////////////////////////////////////////////////////////
    void genMove(unsigned char piece,unsigned char from,unsigned char to)
    {
        data = piece | (from << 4) | (to << 10);
    }

    //////////////////////////////////////////////////////////////////////////
    //sets this step to be a capture.
    //////////////////////////////////////////////////////////////////////////
    void genCapture(unsigned char piece,unsigned char at)
    {
        data = piece | (at << 4) | (at << 10);
    }

    //////////////////////////////////////////////////////////////////////////
    //sets this step to be a pass, which is done by setting the piece moving
    //to no piece.
    //////////////////////////////////////////////////////////////////////////
    void genPass()
    {
        data = NO_PIECE | (1 << 4) | (2 << 10);
    }

    //all the data is packed into a 16-bit integer
    //bits 0 - 3:   describes the piece
    //bits 4 - 9:   describes the source square
    //bits 10 - 15: describes the destination square, set to be the same
    //              as the source square to designate a capture
    unsigned int data; 

    
};

//representation of a combo of steps. Used for sets of steps that cannot be
//broken down into its parts (such as pushes and pulls), and perhaps useful
//macro movements of pieces
class StepCombo
{
    public:
    StepCombo();
    ~StepCombo();

    string toString();
    void fromString(string s);

    void addStep(Step step);
    void addCombo(StepCombo& combo);

    void reset();
    
    Step steps[MAX_STEPS_IN_COMBO]; //the set of steps in this combo. 
                                    //the number was purposefully chosen to
                                    //be large.

    unsigned short numSteps; //the number of steps in the steps array

    unsigned short stepCost; //number of actual steps that this combo takes,
                             //note that captures do not count.
};

#endif
