#ifndef __JR_SEARCHHIST_H__
#define __JR_SEARCHHIST_H__

#include "hash.h"
#include "int64.h"

//Hash entry to keep the earlist occurence of a board state relative to
//the board state in the beginning of a turn.
class SearchHistEntry
{
    public:
    //Earliest ply this board state has occurred relative to the root node
    //of the tree, indexed by player color.
    unsigned char earliestOccur[MAX_COLORS];

    Int64 hash; //The full hash value
};

//Hash table to keep occurences of the same board state within the same turn.
//Logically within a player's turn, there is no need to search a node that
//is a board state that has already been searched elsewhere at a greater
//depth, as then it cannot possibly be better than the already searched line.
class SearchHistTable
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
    //Reset every entry to have no known occurences.
    //////////////////////////////////////////////////////////////////////////
    void reset()
    {
        
        for (int i = 0; i < hashes.getNumEntries(); i++)
        {
            hashes.getEntry(i).earliestOccur[GOLD]   = 255;
            hashes.getEntry(i).earliestOccur[SILVER] = 255;
            hashes.getEntry(i).hash = 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //Returns true if that board state has already occurred at an earlier
    //or equal ply than the input ply from the root for that player
    //////////////////////////////////////////////////////////////////////////
    bool hasOccurredAtPly(Int64 hash, unsigned char ply, unsigned char color)
    {   
        SearchHistEntry& hist = hashes.getEntry(hash & hashMask);
        return (hist.hash == hash &&  hist.earliestOccur[color] < ply);
    }      

    //////////////////////////////////////////////////////////////////////////
    //Attempts to write a hash entry for the input hash occuring at the
    //input ply in respect to the current reference state. If there is
    //already an entry, it is overwritten only if the input ply is earlier
    //than the current entry's ply
    //////////////////////////////////////////////////////////////////////////
    void setOccur(Int64 hash, unsigned char ply, unsigned char color)
    {
        SearchHistEntry& hist = hashes.getEntry(hash & hashMask);
        
        if (ply < hist.earliestOccur[color])
        {
            hist.hash  = hash;
            hist.earliestOccur[color] = ply;
        }
    }

    private:
    //Internal hash table to keep the entries
    HashTable<SearchHistEntry> hashes;

    //Mask to limit bits on accessing hashes
    Int64 hashMask;
};

#endif
