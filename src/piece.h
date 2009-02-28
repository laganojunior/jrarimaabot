#ifndef __JR_PIECE_H__
#define __JR_PIECE_H__

#include <string>

//colors
#define MAX_COLORS 2
#define GOLD 0
#define SILVER 1

//types
#define MAX_TYPES 6
#define ELEPHANT 0
#define CAMEL    1
#define HORSE    2
#define DOG      3
#define CAT      4
#define RABBIT   5
#define NO_PIECE 6

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
