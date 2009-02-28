#include "hash.h"
#include "error.h"
#include "int64.h"
#include <string.h>
/*
//////////////////////////////////////////////////////////////////////////////
//Constructor. Initializes a zero array.
//////////////////////////////////////////////////////////////////////////////
template <class T>
HashTable<T> :: HashTable()
{
    numEntries = 0; 
    entries = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor. Cleans up the array
//////////////////////////////////////////////////////////////////////////////
template <class T>
HashTable<T> :: ~HashTable()
{
    if (entries)
        delete[] entries;
}

//////////////////////////////////////////////////////////////////////////////
//Copy constructor. Make sure to make an actual copy of the array
//////////////////////////////////////////////////////////////////////////////
template <class T>
HashTable<T> :: HashTable(const HashTable<T>& copy)
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


//////////////////////////////////////////////////////////////////////////////
//initializes the hashtable with the specified number of entries, and 
//zeroes out the entries
//////////////////////////////////////////////////////////////////////////////
template <class T>
void HashTable<T> :: init(Int64 numEntries)
{
    if (entries)
        delete [] entries;

    entries = new T[numEntries];
    this->numEntries = numEntries;
}

//////////////////////////////////////////////////////////////////////////
//returns a reference to the entry from the given hash key
//////////////////////////////////////////////////////////////////////////
template <class T>
T& HashTable<T> :: getEntry(Int64 key)
{
    assert (key>=0 && key < numEntries);

    return entries[key];
}

//////////////////////////////////////////////////////////////////////////
//Sets the entry for the given hash key
//////////////////////////////////////////////////////////////////////////
template <class T>
void HashTable<T> :: setEntry(Int64 key, T& entry)
{
    assert (key>=0 && key < numEntries);
    entries[key] = entry;
}*/
