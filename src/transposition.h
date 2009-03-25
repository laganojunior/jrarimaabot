#ifndef __JR_TRANSPOSITION_H__
#define __JR_TRANSPOSITION_H__

#include "hash.h"
#include "rawmove.h"
#include "int64.h"

#define TRANSPOSITION_SCORETYPE_EXACT 0
#define TRANSPOSITION_SCORETYPE_UPPER 1
#define TRANSPOSITION_SCORETYPE_LOWER 2

//A single entry in the transposition table
class TranspositionEntry
{
    //Hash entry used to keep scores and best successor on board positions
    //data is packed into a 64 bit integer as follows:
    //bit 0     : set to 1 if this hash has been already been filled with 
    //            score data
    //bit 1-2   : says wheter the score kept is exact, a lower bound, or
    //            an upper bound
    //bit 3-18  : 16 bits to store a score
    //bit 19-23 : the depth this board was evaluated to
    //bit 24-25 : type of move stored
    //bit 26-32 : index for source square for first moving piece
    //bit 33-39 : index for destination square for first moving piece
    //bit 40-46 : index for source square for second moving piece, note that
    //            the destination from the second piece must be the source 
    //            square of the first piece

    public:
    //inline functions////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    //returns true if this entry has score data, which implies it should have
    //some sort of data on a move also
    //////////////////////////////////////////////////////////////////////////
    bool isFilled()
    {
        return data & 0x1;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns wheter the score is a lower bound, upper bound, or exact score
    //////////////////////////////////////////////////////////////////////////
    unsigned char getScoreType()
    {
        return (data >> 1) & 0x3;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the score kept in this hash entry
    //////////////////////////////////////////////////////////////////////////
    short getScore()
    {
        return (data >> 3) & 0xFFFF;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the depth the position was evaluated to
    //////////////////////////////////////////////////////////////////////////
    unsigned char getDepth()
    {
        return (data >> 19) & 0x1F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the number of steps for the move stored
    //////////////////////////////////////////////////////////////////////////
    unsigned char getNumSteps()
    {
        return (data >> 24) & 0x3;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the index of the source square the first piece is moving from
    //////////////////////////////////////////////////////////////////////////
    unsigned char getFrom1()
    {
        return (data >> 26) & 0x7F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the index of the destination square the first piece is moving 
    //to
    //////////////////////////////////////////////////////////////////////////
    unsigned char getTo1()
    {
        return (data >> 33) & 0x7F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the index of the source square the second piece is moving from
    //////////////////////////////////////////////////////////////////////////
    unsigned char getFrom2()
    {
        return (data >> 40) & 0x7F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the complete hash key
    //////////////////////////////////////////////////////////////////////////
    Int64 getHash()
    {
        return hash;
    }

    //////////////////////////////////////////////////////////////////////////
    //Returns a raw move structure for the move stored.
    //////////////////////////////////////////////////////////////////////////
    RawMove getRawMove()
    {
        return RawMove(getNumSteps(), getFrom1(), getTo1(), getFrom2());
    }

    //////////////////////////////////////////////////////////////////////////
    //Sets the data in this entry to some specified values. Note that the
    //extra bits are given already shifted into place
    //////////////////////////////////////////////////////////////////////////
    void set(bool filled, unsigned char scoreType, short score, 
             unsigned char depth, unsigned char numSteps, unsigned char from1,
             unsigned char to1, unsigned char from2, Int64 hash)
    {
        data = (filled & 0x1) 
             | (((Int64)scoreType & 0x3) << 1)
             | (((Int64)score & 0xFFFF) << 3)
             | (((Int64)depth & 0x1F) << 19)
             | (((Int64)numSteps & 0x3) << 24)
             | (((Int64)from1 & 0x3F) << 26)
             | (((Int64)to1 & 0x7F) << 33)
             | (((Int64)from2 & 0x7F) << 40);

        this->hash = hash;
    }

    //////////////////////////////////////////////////////////////////////////
    //Resets the entry to nothing.
    //////////////////////////////////////////////////////////////////////////
    void reset()
    {
        data = 0;
        hash = 0;
    }

    private:
    Int64 data;
    Int64 hash; // the complete hash key
};

//Transposition table to store bounds on scores and best known successor
//from stored positions
class TranspositionTable
{
    public:
    //////////////////////////////////////////////////////////////////////////
    //sets the table size to handle all keys that have the specified number of 
    //bits. That is the table is set to 2 ^ (numbits) entries. Also sets the
    //hash mask to enforce that keys are limited to that number of bits
    //////////////////////////////////////////////////////////////////////////
    void setHashKeySize(unsigned int numBits)
    {
        hashes.init(Int64FromIndex(numBits)); 
        hashMask = Int64LowerBitsFilled(numBits);        
    }

    //////////////////////////////////////////////////////////////////////////
    //Returns wheter the entry for that hash key is valid for that key
    //////////////////////////////////////////////////////////////////////////
    bool hasValidEntry(Int64 hash)
    {
        return hashes.getEntry(hash & hashMask).isFilled() &&
               hashes.getEntry(hash & hashMask).getHash() == hash;
    }

    //////////////////////////////////////////////////////////////////////////
    //Gets the entry for that hash key. Note that this will always return
    //an entry, even if that entry isn't filled in or appropriate for use.
    //To check if it is, use hasValidEntry()
    //////////////////////////////////////////////////////////////////////////
    TranspositionEntry& getEntry(Int64 hash)
    {
        return hashes.getEntry(hash & hashMask);
    }

    //////////////////////////////////////////////////////////////////////////
    //Sets the entry for that hash key. If another hash was already there,
    //this replaces it only if the new depth is at least the depth already
    //stored
    //////////////////////////////////////////////////////////////////////////
    void setEntry(Int64 hash, unsigned char scoreType, short score, 
                  unsigned char depth, RawMove bestMove)
    {
        TranspositionEntry& entry = hashes.getEntry(hash & hashMask);
        if (!entry.isFilled() || depth >= entry.getDepth())
        {
            entry.set(true, scoreType, score, depth, bestMove.numSteps,
                      bestMove.from1, bestMove.to1, bestMove.from2, hash);
        }
    }
    
    private:
    //Internal hash table to keep the entries
    HashTable<TranspositionEntry> hashes;

    //Mask to limit bits on accessing hashes
    Int64 hashMask;
};

#endif


