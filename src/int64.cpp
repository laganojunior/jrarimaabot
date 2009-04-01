#include "square.h"
#include "int64.h"
#include <sstream>
#include <string>
#include <cstdlib>

using namespace std;

//some pre-defined arrays

Int64 bits[64];//64 bit integers with all bits set to 0, except for
               //a single bit that is 1, indicated by the index of the
               //array
unsigned int firstBit[65536]; //the lowest bit present in a given
                              //16-bit value
Int64 neighbors[64]; //64 bit integers with all bits set to 0, except for
                     //the bits that are "neighbors" to that index when
                     //considering the bitboard representation, which are
                     //1   

Int64 neighborsup[64]; //same as neighbors, but doesn't include the south 
                       //neighbor

Int64 neighborsdown[64]; //same as neighbors, but doesn't include the north
                         //neighbor

Int64 traps; //bitboard that represents the trap squares

Int64 trapNeighbors; //bitboard that represents the neighbors of (but not
                     //including) trap squares

Int64 rows[8]; //bitboards that represent the rows on a bitboard
Int64 cols[8]; //bitboards that represent the columns on a bitboard

Int64 centerRings[4]; //bitboards that represent the concentric rings of
                      //squares about the center.


//////////////////////////////////////////////////////////////////////////////
//Generate the bit arrays for use for other functions
//////////////////////////////////////////////////////////////////////////////
void initInt64()
{
    //initialize bits array
    bits[0] = 1;
    for (int i = 1; i < 64; i ++)
        bits[i] = bits[i-1] << 1;   

    //initialize neighbors array
    for (int row = 0; row < 8; row ++)
    {
        for (int col = 0; col < 8; col ++)
        {
            int index = col + row * 8;
            neighbors[index] = 0;
            
            if (row != 0)
                neighbors[index] |= Int64FromIndex(index - 8);
            
            if (row != 7)
                neighbors[index] |= Int64FromIndex(index + 8);
        
            if (col != 0)
                neighbors[index] |= Int64FromIndex(index - 1);
    
            if (col != 7)
                neighbors[index] |= Int64FromIndex(index + 1);
        }
    }

    //initialize neighborsup
    for (int row = 0; row < 8; row ++)
    {
        for (int col = 0; col < 8; col ++)
        {
            int index = col + row * 8;
            neighborsup[index] = 0;
            
            if (row != 0)
                neighborsup[index] |= Int64FromIndex(index - 8);
        
            if (col != 0)
                neighborsup[index] |= Int64FromIndex(index - 1);
    
            if (col != 7)
                neighborsup[index] |= Int64FromIndex(index + 1);
        }
    }

    //initialize neighborsdown array
    for (int row = 0; row < 8; row ++)
    {
        for (int col = 0; col < 8; col ++)
        {
            int index = col + row * 8;
            neighborsdown[index] = 0;
            
            if (row != 7)
                neighborsdown[index] |= Int64FromIndex(index + 8);
        
            if (col != 0)
                neighborsdown[index] |= Int64FromIndex(index - 1);
    
            if (col != 7)
                neighborsdown[index] |= Int64FromIndex(index + 1);
        }
    }

    //initialize traps
    traps = Int64FromIndex(C3) | Int64FromIndex(C6) | Int64FromIndex(F6)
           |Int64FromIndex(F3);

    //initialize trapNeighbors
    trapNeighbors = getNeighbors(C3) | getNeighbors(C6) | getNeighbors(F6)
                  | getNeighbors(F3) ;

    //initialize rows
    for (int i = 0; i < 8; i++)
    {
        rows[i] = (Int64)0xFF << (8 * i);
    }

    //initialize columns
    for (int i = 0; i < 8; i++)
    {
        cols[i] = ((Int64)1 << i) | ((Int64)1 << i + 8)
                | ((Int64)1 << i + 16) | ((Int64)1 << i + 24)
                | ((Int64)1 << i + 32) | ((Int64)1 << i + 40)
                | ((Int64)1 << i + 48) | ((Int64)1 << i + 56);
    }

    //initialize center rings
    for (int i = 0; i < 4; i++)
    {
        centerRings[i] = 0;

        //initialize top part of ring
        for (int j = 3 - i; j <= 4 + i; j++)
        {
            centerRings[i] |= Int64FromIndex( (3 - i) * 8 + j);    
        }

        //initialize the sides
        for (int j = 3 - i; j <= 4 + i; j++)
        {
            centerRings[i] |= Int64FromIndex( j * 8 + (3 - i)); 
            centerRings[i] |= Int64FromIndex( j * 8 + (4 + i)); 
        }

        //initialize bottom part
        for (int j = 3 - i; j <= 4 + i; j++)
        {
            centerRings[i] |= Int64FromIndex( (4 + i) * 8 + j);    
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//Returns a 64 bit integer as a string in base-2 format (i.e. 0 and 1's) 
//////////////////////////////////////////////////////////////////////////////
string Int64ToString(Int64 i)
{
    stringstream out;

    for (int index = 63; index >= 0; index--)
    {
        if (i & Int64FromIndex(index))
            out << '1';
        else
            out << '0';
    }

    return out.str();
}

//////////////////////////////////////////////////////////////////////////////
//Returns a 64 bit integer as a string that represents it as a bitboard, i.e.
//a 8x8 array of bits.
//////////////////////////////////////////////////////////////////////////////
string BitboardToString(Int64 i)
{
    stringstream out;

    for (int row = 0; row < 8; row ++)
    {
        for (int col = 0; col < 8; col ++)  
        {
            if (i & Int64FromIndex(col + row * 8))
                out << '1';
            else
                out << '0';
        }
        out << endl;
    }

    return out.str();
}

//////////////////////////////////////////////////////////////////////////////
//returns a random 64 bit integer
//////////////////////////////////////////////////////////////////////////////
Int64 randInt64()
{
    return (Int64)rand() ^ ((Int64)rand() << 16) ^ ((Int64)rand() << 32)
           ^ ((Int64)rand() << 48);
}



