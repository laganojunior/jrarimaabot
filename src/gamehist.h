#ifndef __JR_GAMEHIST_H__
#define __JR_GAMEHIST_H__

#include "hash.h"
#include "int64.h"

//Hash entry to see how many times a position occurred at the end of a turn
//Used for enforcing 3-repeat rule
class GameHistEntry
{
    public:
    unsigned char numOccur; // number of times this position has occurred
    
};

//Table to keep game history table
class GameHistTable
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
    //Reset every entry to have no occurences
    //////////////////////////////////////////////////////////////////////////
    void reset()
    {
        
        for (int i = 0; i < hashes.getNumEntries(); i++)
        {
            hashes.getEntry(i).numOccur = 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //Increase the number of occurences for that hash
    //////////////////////////////////////////////////////////////////////////
    void incrementOccur(Int64 hash)
    {
        if (hashes.getEntry(hash & hashMask).numOccur < 255)
            hashes.getEntry(hash & hashMask).numOccur++;
    }

    //////////////////////////////////////////////////////////////////////////
    //Decrease the number of occurences for that hash
    //////////////////////////////////////////////////////////////////////////
    void decrementOccur(Int64 hash)
    {
        if (hashes.getEntry(hash & hashMask).numOccur > 0)
            hashes.getEntry(hash & hashMask).numOccur--;
    }

    //////////////////////////////////////////////////////////////////////////
    //Return the number of occurences for that hash
    //////////////////////////////////////////////////////////////////////////
    unsigned char getNumOccur(Int64 hash)
    {
        return hashes.getEntry(hash & hashMask).numOccur;
    }

    private:
    //Internal hash table to keep the entries
    HashTable<GameHistEntry> hashes;

    //Mask to limit bits on accessing hashes
    Int64 hashMask;
};

#endif
