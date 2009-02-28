#ifndef __JR_HASH_H__
#define __JR_HASH_H__

//hash table data structures to keep data for transpositions

#define SCORE_ENTRY_EXTRA_BITS 32

#define SCORE_ENTRY_EXACT 0
#define SCORE_ENTRY_UPPER 1
#define SCORE_ENTRY_LOWER 2

#include "int64.h"
#include <assert.h>
#include <string.h>


class ScoreEntry
{
    //data is packed into a 64 bit integer as follows:
    //bit 0     : set to 1 if this hash has been already been filled with 
    //            score data
    //bit 1-2   : says wheter the score kept is exact, a lower bound, or
    //            an upper bound
    //bit 3-18  : 16 bits to store a score
    //bit 19-26 : index of the combo in the array returned by genMoves() that
    //            either is the best move from this node, or caused a beta
    //            cutoff
    //bit 27-31 : the depth this board was evaluated to
    //bit 32-63 : extra bits used to differentiate between boards that map
    //            to the same hash key

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
    //return the index of the combo (in respect to the array generated by
    //genMoves()) that is either the best move to take or a move that last
    //generated a beta cutoff
    //////////////////////////////////////////////////////////////////////////
    unsigned char getMoveIndex()
    {
        return (data >> 19) & 0xFF;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the depth the position was evaluated to
    //////////////////////////////////////////////////////////////////////////
    unsigned char getDepth()
    {
        return (data >> 27) & 0x1F;
    }

    //////////////////////////////////////////////////////////////////////////
    //returns the extra bits that contains basically another hash of the board
    //to differentiate positions that map to the same hashkey. Note that
    //the extra bits are given unshifted.
    //////////////////////////////////////////////////////////////////////////
    Int64 getExtra()
    {
        return data & (((Int64)0xFFFFFFFF) << 32);
    }

    //////////////////////////////////////////////////////////////////////////
    //Sets the data in this entry to some specified values. Note that the
    //extra bits are given already shifted into place
    //////////////////////////////////////////////////////////////////////////
    void set(bool filled, unsigned char scoreType, short score, 
             unsigned char moveIndex, unsigned char depth, Int64 extra)
    {
        data = (filled & 0x1) 
             | (((Int64)scoreType & 0x3) << 1)
             | (((Int64)score & 0xFFFF) << 3)
             | (((Int64)moveIndex & 0xFF) << 19) 
             | (((Int64)depth & 0x1F) << 27) 
             | extra;

		//cout << "data:\n";
		//cout << Int64ToString(data) << endl;
    }

    private:
    Int64 data;
};

//Generic Hash table template class
template <class T> 
class HashTable
{
    public:
	//////////////////////////////////////////////////////////////////////////
	//Constructor. Initializes a zero array.
	//////////////////////////////////////////////////////////////////////////
    HashTable()
	{
		numEntries = 0; 
		entries = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//Deconstructor. Cleans up the array
	//////////////////////////////////////////////////////////////////////////
    ~HashTable()
	{
		if (entries)
		    delete[] entries;
	}

	//////////////////////////////////////////////////////////////////////////
	//Copy constructor. Make sure to make an actual copy of the array
	//////////////////////////////////////////////////////////////////////////
    HashTable(const HashTable<T>& copy)
	{
		//check for self-assignment
		if (&copy == this)
		    return;

		if (entries)    
		    delete []entries;

		if (copy.entries != NULL)
		{
		    entries = new T[copy.numEntries];
		    numEntries = copy.numEntries;
		}
		else
		{
		    entries = NULL;
		    numEntries = 0;
		}
	} 

	//////////////////////////////////////////////////////////////////////////
	//initializes the hashtable with the specified number of entries, and 
	//zeroes out the entries
	//////////////////////////////////////////////////////////////////////////
    void init(Int64 numEntries)
	{
		if (entries)
		    delete [] entries;

		entries = new T[numEntries];
		this->numEntries = numEntries;

		memset(entries, 0, sizeof(T) * numEntries);
	}

	//////////////////////////////////////////////////////////////////////////
	//returns a reference to the entry from the given hash key
	//////////////////////////////////////////////////////////////////////////
	T& getEntry(Int64 key)
	{
		assert (key>=0 && key < numEntries);

		return entries[key];
	}


    //////////////////////////////////////////////////////////////////////////
	//Sets the entry for the given hash key
	//////////////////////////////////////////////////////////////////////////
	void setEntry(Int64 key, T& entry)
	{
		assert (key>=0 && key < numEntries);
		entries[key] = entry;
	}

    private:

    T * entries;       //array of entries in the hashtable
    Int64 numEntries;  //number of entries
};    

#endif
    
