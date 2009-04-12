#include "error.h"
#include "int64.h"
#include <assert.h>
#include <ctype.h>
#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//return the index of the square referred to by a string in human-readable
//format.
//////////////////////////////////////////////////////////////////////////////
unsigned char squareFromString(string squareString)
{
    if (squareString.length() != 2)
    {
        Error error;
        error << "From squareFromString(string)\n"
              << "String must be 2 characters\n"
              << "Got: " << squareString << '\n';
        throw error;
    }

    int x = squareString[0] - 'a';
    int y = 8 - (squareString[1] - '0');

    if (x < 0 || x > 7 || y < 0 || y > 7)
    {
        Error error;
        error << "From squareFromString(string)\n"
              << "Invalid square\n"
              << "Got " << squareString << "\n";
        throw error;
    }
    
    return x + y * 8;
}

//////////////////////////////////////////////////////////////////////////////
//Returns the string representation of a square
//////////////////////////////////////////////////////////////////////////////
string stringFromSquare(unsigned char index)
{
    string squareString = "00"; //init some two character string
    
    int row = index / 8;
    int col = index % 8;
    
    squareString[0] = col + 'a';
    squareString[1] = (8 - row) + '0';
    
    return squareString;
}

//////////////////////////////////////////////////////////////////////////////
//returns true if the index refers to a capture square on the board
//////////////////////////////////////////////////////////////////////////////
bool isCaptureSquare(unsigned char index)
{
    assert (index < 64);

    int row = index / 8;
    int col = index % 8;
    
    return ( (row == 2 || row == 5) && (col == 2 || col == 5));
}
