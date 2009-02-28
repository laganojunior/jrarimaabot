#include "hash.h"
#include "error.h"
#include "int64.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
//Constructor. Initializes an array of hash entries so that a key with the
//specified number of bits falls within the entries. This means the number
//of entries is initialized to 2 ^ (numBits)
//////////////////////////////////////////////////////////////////////////////
HashTable :: HashTable(unsigned int numBits)
{
    numEntries = Int64FromIndex(numBits);
    entries = new HashTableEntry[numEntries];

    //zero out the hash entries
    memset(entries, 0, sizeof(HashTableEntry) * numEntries);
}

//////////////////////////////////////////////////////////////////////////////
//Deconstructor. Cleans up the array
//////////////////////////////////////////////////////////////////////////////
HashTable :: ~HashTable()
{
    delete[] entries;
}

