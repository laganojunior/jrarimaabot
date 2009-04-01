#ifndef __JR_HASH_H__
#define __JR_HASH_H__

//hash table data structures to keep data for transpositions

#include "defines.h"
#include "int64.h"
#include <assert.h>
#include <string.h>
#include <vector>

using namespace std;

//Generic Hash table template class
template <class T>
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
