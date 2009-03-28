#ifndef __JR_HASH_H__
#define __JR_HASH_H__

//hash table data structures to keep data for transpositions

#include "defines.h"
#include "int64.h"
#include <assert.h>
#include <string.h>
#include <vector>

using namespace std;

//Hash entry to see if a position has already occurred down the current 
//search path. Used to avoid repititions
class SearchHistEntry
{
    public:
    SearchHistEntry()
    {
        reset();
    }

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
	//initializes the hashtable with the specified number of entries
	//////////////////////////////////////////////////////////////////////////
    void init(Int64 numEntries)
	{
		entries.resize(numEntries);
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
