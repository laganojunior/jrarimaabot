#ifndef __JR__INT64_H__
#define __JR__INT64_H__

//////////////////////////////////////////////////////////////////////////////
//This class contains classes and functions to generate and manipulate 64 bit 
//integers, wheter they are bitboard representations, hashes, etc..
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

//bitscanforward bit not found error
#define NO_BIT_FOUND -1

using namespace std;

//Create a typedef for 64 bit integers, as the actual implementation might
//differ in different systems
typedef unsigned long long Int64;

//allow access to some arrays and variables in int64.cpp
extern Int64 bits[64];
extern Int64 neighbors[64];
extern Int64 neighborsup[64];
extern Int64 neighborsdown[64];
extern Int64 traps;
extern Int64 trapNeighbors;
extern Int64 rows[8];
extern Int64 centerRings[4];

//some functions implemented in int64.cpp
void initInt64();
string Int64ToString(Int64 i);
string BitboardToString(Int64 i);
Int64 randInt64();

//inline functions implemented here

//////////////////////////////////////////////////////////////////////////////
//Returns the 64 bit integer with all bits set to 0 except for the bit
//at the specified index, which is set to 1
//////////////////////////////////////////////////////////////////////////////
inline Int64 Int64FromIndex(unsigned int index)
{
    return bits[index];
}

//////////////////////////////////////////////////////////////////////////////
//Returns the lowest index of the lowest 1 bit in a 64 bit integer. If there
//are none found, -1 is returned
//////////////////////////////////////////////////////////////////////////////
inline int bitScanForward(Int64 i)
{
    return __builtin_ffsll(i) - 1;
}

//////////////////////////////////////////////////////////////////////////////
//returns the neighbors bitboard for the position index specified
//////////////////////////////////////////////////////////////////////////////
inline Int64 getNeighbors(unsigned int index)
{
    return neighbors[index];
}

//////////////////////////////////////////////////////////////////////////////
//returns the neighbors bitboard for the position index specified without
//the south neighbor
//////////////////////////////////////////////////////////////////////////////
inline Int64 getNeighborsUp(unsigned int index)
{
    return neighborsup[index];
}

//////////////////////////////////////////////////////////////////////////////
//returns the neighbors bitboard for the position index specified without the
//north neighbor
//////////////////////////////////////////////////////////////////////////////
inline Int64 getNeighborsDown(unsigned int index)
{
    return neighborsdown[index];
}

//////////////////////////////////////////////////////////////////////////////
//Returns the bitboard that represents the trap squares
//////////////////////////////////////////////////////////////////////////////
inline Int64 getTraps()
{
    return traps;
}

//////////////////////////////////////////////////////////////////////////////
//Returns the bitboard that represents the neighbors of (but not including)
//trap squares
//////////////////////////////////////////////////////////////////////////////
inline Int64 getTrapNeighbors()
{
    return trapNeighbors;
}

//////////////////////////////////////////////////////////////////////////////
//Returns the bitboard that represents the row specified, starting from the
//top of the board.
//////////////////////////////////////////////////////////////////////////////
inline Int64 getRow(int row)
{
    return rows[row];
}

//////////////////////////////////////////////////////////////////////////////
//Returns the ring centered about the center of the board, starting from the
//center and going outward
//////////////////////////////////////////////////////////////////////////////
inline Int64 getCenterRing(int i)
{
    return centerRings[i];
}

#endif
