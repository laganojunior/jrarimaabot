#include "piece.h"
#include "error.h"
#include <assert.h>
#include <ctype.h>



//////////////////////////////////////////////////////////////////////////////
//return a encoded piece (read step.h) from a readable character
//////////////////////////////////////////////////////////////////////////////
unsigned char pieceFromChar(char pieceChar)
{
    unsigned char type;
    switch (tolower(pieceChar))
	{
        case 'e':
            type = ELEPHANT;
            break;
        case 'm':
            type = CAMEL;
            break;
        case 'h':
            type = HORSE;
            break;
        case 'd':
            type = DOG;
            break;
        case 'c':
            type = CAT;
            break;
        case 'r':
            type = RABBIT;
            break;
        default:
        {
            Error error;
            error << "From pieceFromChar(char)"
                  << "Invalid piece character\n"
                  << "Got " << pieceChar << '\n';
            throw error;
        }
    }

    unsigned char color;

    if (islower(pieceChar))
        color = SILVER;
    else
        color = GOLD;

    return genPiece(color,type);
}

//////////////////////////////////////////////////////////////////////////////
//return a human readable character referring to the piece that was given
//////////////////////////////////////////////////////////////////////////////
char          charFromPiece(unsigned char piece)
{
    unsigned char color = colorOfPiece(piece);
    unsigned char type  = typeOfPiece(piece);
    
    assert(color >= 0 && color <= MAX_COLORS 
            && type >= 0 && type <= MAX_TYPES);

    char pieceChar;
    
    switch (type)
    {
        case ELEPHANT:
            pieceChar = 'e';
            break;
        case CAMEL:
            pieceChar = 'm';
            break;
        case HORSE:
            pieceChar = 'h';
            break;
        case DOG:
            pieceChar = 'd';
            break;
        case CAT:
            pieceChar = 'c';
            break;
        case RABBIT:
            pieceChar = 'r';
            break;
        default:
            pieceChar = '?';
    }
    
    if (color == GOLD)
        pieceChar = toupper(pieceChar);
    
    return pieceChar;
}

//////////////////////////////////////////////////////////////////////////////
//returns the string representation of a color
//////////////////////////////////////////////////////////////////////////////
string stringFromColor(unsigned char color)
{
    if (color == GOLD)
        return "GOLD";
    if (color == SILVER)
        return "SILVER";
    
    return "NONE"; //??? should never get here
}

//////////////////////////////////////////////////////////////////////////////
//returns the one character representation of a color
//////////////////////////////////////////////////////////////////////////////
char charFromColor(unsigned char color)
{
    if (color == GOLD)
        return 'g';
    
    if (color == SILVER)
        return 's';
    
    return '?';
}


//////////////////////////////////////////////////////////////////////////////
//return the color of a piece
//////////////////////////////////////////////////////////////////////////////
unsigned char colorOfPiece(unsigned char piece)
{
    return piece >> 3; //return the 4th bit
}

//////////////////////////////////////////////////////////////////////////////
//return the type of a piece
//////////////////////////////////////////////////////////////////////////////
unsigned char typeOfPiece(unsigned char piece)
{
    //return the first 3 bits
    return piece & 0x7;
}

//////////////////////////////////////////////////////////////////////////////
//return a piece with the specified color and type
//////////////////////////////////////////////////////////////////////////////
unsigned char genPiece(unsigned char color,unsigned char type)
{
    return type | (color << 3);
}

//////////////////////////////////////////////////////////////////////////////
//returns the opposite color of the color given.
//////////////////////////////////////////////////////////////////////////////
unsigned char oppColorOf(unsigned char color)
{
    assert (color == GOLD || color == SILVER);
    
    return color ^ 1;
}
