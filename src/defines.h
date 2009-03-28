#ifndef __JR_DEFINES_H__
#define __JR_DEFINES_H__

//some constant defines that many files may need

//Evaluation Constants
#define EVAL_KILLER_MOVES_PER_PLY 2


//Hash table constants
#define SCORE_ENTRY_EXACT 0
#define SCORE_ENTRY_UPPER 1
#define SCORE_ENTRY_LOWER 2

#define SCORE_MOVE_1STEP 1
#define SCORE_MOVE_2STEP 2

//Bitscan return code
#define NO_BIT_FOUND -1

//piece and side colors
#define MAX_COLORS 2
#define GOLD 0
#define SILVER 1

//piece types
#define MAX_TYPES 6
#define ELEPHANT 0
#define CAMEL    1
#define HORSE    2
#define DOG      3
#define CAT      4
#define RABBIT   5
#define NO_PIECE 6

//square constants
#define C6 18
#define F6 21
#define C3 42
#define F3 45
#define ILLEGAL_SQUARE 64

#define NUM_SQUARES 64

//Search constants
#define SEARCH_MAX_DEPTH 20

#endif
