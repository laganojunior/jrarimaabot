#ifndef __JR_SQUARE_H__
#define __JR_SQUARE_H__

//some useful functions that are common in many places in the code
#include "defines.h"
#include <string>

using namespace std;

//functions to convert to/from ascii representations to internal
//representations
unsigned char squareFromString(string squareString);
string        stringFromSquare(unsigned char index);

//functions to extract/set attributes of pieces  
unsigned char colorOfPiece(unsigned char piece);
unsigned char typeOfPiece(unsigned char piece);
unsigned char genPiece(unsigned char color,unsigned char type);

//functions to extract attributes from squares
bool isCaptureSquare(unsigned char index);

#endif
