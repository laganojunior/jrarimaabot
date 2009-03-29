#ifndef __JR_STEP_H__
#define __JR_STEP_H__

#include "piece.h"
#include "defines.h"
#include "rawmove.h"
#include <string>
#include <vector>

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

    unsigned char getFrom1();
    unsigned char getTo1();
    unsigned char getFrom2();

    RawMove getRawMove();

    void reset();
    
    void genPass(int stepsLeft)
    {
        steps.resize(0);
        stepCost = stepsLeft;
    }
    
    bool isPass()
    {
        return steps.empty() && stepCost > 0;
    }

    bool operator==(StepCombo& comp)
    {
        if (steps.size() != comp.steps.size())
            return false;

        for (int i = 0; i < steps.size(); i++)
        {
            if (comp.steps[i].data != steps[i].data)
                return false;  
        }
        
        return true;
    }     
    
    vector<Step> steps; //the set of steps in this combo, note that the array
                        //can be larger than the actual number of steps stored
                        //Use the variable numSteps to find the actual number.

    unsigned int numSteps;   //number of total steps, including captures.
    
    unsigned short stepCost; //number of actual steps that this combo takes,
                             //note that captures do not count.

    short score; //a heurisitic score used for move ordering. Has nothing
                 //to do with scores used in scoring boards and solving the
                 //game tree.
};

#endif
