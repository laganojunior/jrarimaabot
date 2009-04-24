#ifndef __JR_EVALHASH_H__
#define __JR_EVALHASH_H__

#include "hash.h"
#include "int64.h"

//Hash entry to keep static evaluation scores
class EvalHashEntry
{
    public:
    short score;  //static score for the board position scored in the
                  //perspective of gold

    unsigned char useful; //a count of how many times this entry was used
                          //minus the amount of times it wasn't used. The
                          //entry is overwritten only if the useful count
                          //goes to 0

    Int64 hash; //The full hash value
};

//Table to keep static evaluation data
class EvalHashTable
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
    //Reset every entry
    //////////////////////////////////////////////////////////////////////////
    void reset()
    {
        for (int i = 0; i < hashes.getNumEntries(); i++)
        {
            hashes.getEntry(i).score = 0;
            hashes.getEntry(i).hash = 0;
            hashes.getEntry(i).useful = 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //Attempts to get the entry for the input hash. If an entry is found
    //that matches the full hash, true is returned, the corresponding
    //entry is written onto the input reference and its useful count is
    //incremented. If not, false is returned.
    //////////////////////////////////////////////////////////////////////////
    bool getEntry(Int64 hash, EvalHashEntry& out)
    {
        EvalHashEntry& entry = hashes.getEntry(hash & hashMask);
        if (entry.hash == hash)
        {
            out = entry;
            if (entry.useful  != 255)
                ++entry.useful;
            return true;
        }
        else
        {
            return false;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //Attempts to write a hash entry for the input hash, score, and color.
    //If there is already an entry, that entry's useful count is decremented
    //and will be overwritten if the useful count goes to 0.
    //////////////////////////////////////////////////////////////////////////
    void setEntry(Int64 hash, short score)
    {
        EvalHashEntry& entry = hashes.getEntry(hash & hashMask);

        if (entry.hash != hash)
        {
            if (entry.useful != 0)
            {
                --entry.useful;
                if (entry.useful == 0)
                {
                entry.hash = hash;
                entry.score = score;
                entry.useful = 1;
                }
            }
        }

        entry.hash = hash;
        entry.score = score;
        entry.useful = 1;
    }

    private:
    //Internal hash table to keep the entries
    HashTable<EvalHashEntry> hashes;

    //Mask to limit bits on accessing hashes
    Int64 hashMask;
};

#endif
