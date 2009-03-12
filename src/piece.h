#ifndef __JR_PIECE_H__
#define __JR_PIECE_H__

#include "defines.h"
#include <string>

using namespace std;

//conversion from internal to/from ASCII representations
unsigned char pieceFromChar(char pieceChar);
char          charFromPiece(unsigned char piece);
string        stringFromColor(unsigned char color);
char          charFromColor(unsigned char color);

//set/extract attributes
unsigned char colorOfPiece(unsigned char piece);
unsigned char typeOfPiece(unsigned char piece);
unsigned char genPiece(unsigned char color,unsigned char type);

//some other stuff
unsigned char oppColorOf(unsigned char color);
#endif
