#ifndef __JR_RAWMOVE_H__
#define __JR_RAWMOVE_H__

//Raw Move structure, which just has data on source squares and destination
//squares and context-independent from any board configuration. Used for
//transposition table and killer move table.
class RawMove
{
    public:
    RawMove() {}
    ~RawMove() {}
    RawMove(unsigned char numSteps, unsigned char from1, unsigned char to1,
            unsigned char from2)
    {
        this->numSteps = numSteps;
        this->from1    = from1;
        this->to1      = to1;
        this->from2    = from2;
    }

    bool operator==(const RawMove comp)
    {
        return numSteps == comp.numSteps && from1 == comp.from1
            && to1 == comp.to1 && from2 == comp.from2;
    }

    unsigned char numSteps; //Number of steps this move takes
    unsigned char from1;    //the source square the first piece moves from
    unsigned char to1;      //the destination square the first piece moves to
    unsigned char from2;    //the source square the second piece moves from
                            //if needed. Note it is implied the second piece
                            //moves to the square that first piece moved from.
                            //That is, 2nd piece moves from2 -> from1
};

#endif
