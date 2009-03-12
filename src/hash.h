#ifndef __JR_HASH_H__
#define __JR_HASH_H__

//hash table data structures to keep data for transpositions

#include "defines.h"
#include "int64.h"
#include <assert.h>
#include <string.h>
#include <vector>

using namespace std;

//Hash entry used to keep scores and best successor on board positions
//data is packed into a 64 bit integer as follows:
//bit 0     : set to 1 if this hash has been already been filled with 
//            score data
//bit 1-2   : says wheter the score kept is exact, a lower bound, or
//            an upper bound
//bit 3-18  : 16 bits to store a score
//bit 19-23 : the depth this board was evaluated to
//bit 24-25 : type of move stored
//bit 26-31 : index for source square for first moving piece
//bit 32-37 : index for destination square for first moving piece
//bit 38-43 : index for source square for second moving piece, note that
//            the destination from the second piece must be the source square
//            of the first piece
class ScoreEntry
{

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
    //returns the move type stored
    //////////////////////////////////////////////////////////////////////////
    unsigned char getMoveType()
    {
        return (data >> 24) & 0x3;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the index of the source square the first piece is moving from
    //////////////////////////////////////////////////////////////////////////
    unsigned char getFrom1()
    {
        return (data >> 26) & 0x3F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the index of the destination square the first piece is moving 
    //to
    //////////////////////////////////////////////////////////////////////////
    unsigned char getTo1()
    {
        return (data >> 32) & 0x3F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the index of the source square the second piece is moving from
    //////////////////////////////////////////////////////////////////////////
    unsigned char getFrom2()
    {
        return (data >> 38) & 0x3F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the complete hash key
    //////////////////////////////////////////////////////////////////////////
    Int64 getHash()
    {
        return hash;
    }

    //////////////////////////////////////////////////////////////////////////
    //Sets the data in this entry to some specified values. Note that the
    //extra bits are given already shifted into place
    //////////////////////////////////////////////////////////////////////////
    void set(bool filled, unsigned char scoreType, short score, 
             unsigned char depth, unsigned char moveType, unsigned char from1,
             unsigned char to1, unsigned char from2, Int64 hash)
    {
        data = (filled & 0x1) 
             | (((Int64)scoreType & 0x3) << 1)
             | (((Int64)score & 0xFFFF) << 3)
             | (((Int64)depth & 0x1F) << 19)
             | (((Int64)moveType & 0x3) << 24)
             | (((Int64)from1 & 0x3F) << 26)
             | (((Int64)to1 & 0x3F) << 32)
             | (((Int64)from2 & 0x3F) << 38);

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

//Hash entry to see how many times a position occurred at the end of a turn
//Used for enforcing 3-repeat rule
class GameHistEntry
{
    public:
    //////////////////////////////////////////////////////////////////////////
    //Resets the entry to nothing.
    //////////////////////////////////////////////////////////////////////////
    void reset()
    {
        numOccur = 0;
    }


    char numOccur; // number of times this position has occurred
    
};

//Hash entry to see if a position has already occurred down the current 
//search path. Used to avoid repititions
class SearchHistEntry
{
    public:
    //////////////////////////////////////////////////////////////////////////
    //Resets the entry to nothing.
    //////////////////////////////////////////////////////////////////////////
    void reset()
    {
        occured = false;
    }


    char occured; 
    
};

//Generic Hash table template class
template <class T> //Note, T should have a reset() functions
class HashTable
{
    public:
	//////////////////////////////////////////////////////////////////////////
	//initializes the hashtable with the specified number of entries, and 
	//zeroes out the entries
	//////////////////////////////////////////////////////////////////////////
    void init(Int64 numEntries)
	{
		entries.resize(numEntries);
        for (int i = 0; i < numEntries; i++)
            entries[i].reset();
	}

	//////////////////////////////////////////////////////////////////////////
	//returns a reference to the entry from the given hash key
	//////////////////////////////////////////////////////////////////////////
	T& getEntry(Int64 key)
	{
		assert (key>=0 && key < entries.size());

		return entries[key];
	}

    //////////////////////////////////////////////////////////////////////////
	//Sets the entry for the given hash key
	//////////////////////////////////////////////////////////////////////////
	void setEntry(Int64 key, T& entry)
	{
		assert (key>=0 && key < entries.size());
		entries[key] = entry;
	}

    //////////////////////////////////////////////////////////////////////////
    //Returns the number of entries
    //////////////////////////////////////////////////////////////////////////
    unsigned int getNumEntries()
    {
        return entries.size();
    }

    private:

    vector<T> entries;       //array of entries in the hashtable
    Int64 numEntries;  //number of entries
};    

#endif
