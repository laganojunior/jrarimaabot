#include "board.h"
#include "error.h"
#include "int64.h"
#include "step.h"
#include "search.h"
#include "maxheap.h"
#include "hash.h"
#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include <cstdlib>
#include <iomanip>

//different behavior modes
#define MODE_NONE 0
#define MODE_GAMEROOM 1
#define MODE_HELP 2


using namespace std;

//////////////////////////////////////////////////////////////////////////////
//run in gameroom mode, just load the data from the 3 files and then print a
//move to stdout.
//////////////////////////////////////////////////////////////////////////////
void gameroom(fstream& logFile, string positionFile, string moveFile,  
              string gamestateFile, string evalWeightFile,
              int maxDepth, int hashTableBytes)
{

    //start logging, noting the time.
    logFile << "*******************************************\n";
    time_t rawtime = time(0);
    tm * timeinfo = localtime(&rawtime);
    logFile << asctime(timeinfo) << endl;
    logFile << "Started with:\n"
            << "Position file " << positionFile << endl
            << "Move file " << moveFile << endl
            << "Gamestate file " << gamestateFile << endl; 

    Board board;
    board.genRandomHashes();
    board.loadPositionFile(positionFile);

    logFile << "Loaded file. Board state is:\n";
    logFile << board << endl;

    //if first move, just play fixed position
    if (board.turnNumber == 1)  
    {
        logFile << "First move, playing fixed position\n";
        
        if (board.sideToMove == GOLD)
        {
            cout << "Ra1 Rb1 Rc1 Dd1 De1 Rf1 Rg1 Rh1 Ra2 Hb2 Cc2 Md2 Ee2 Cf2 Hg2 Rh2\n";
        }
        else
        {
            cout << "ra8 rb8 rc8 dd8 de8 rf8 rg8 rh8 ra7 hb7 cc7 md7 ee7 cf7 hg7 rh7\n";
        }
    }
    else //otherwise actually go through a search
    {
        
        Search search(hashTableBytes);
        search.loadMoveFile(moveFile, board);
        search.eval.loadWeights(evalWeightFile);
        
        StepCombo bestMove = search.iterativeDeepen(board, maxDepth, logFile);
                                                    
        logFile << "Finished Search\n";
        logFile << "Doing move " << bestMove.toString() << endl;
        cout << bestMove.toString() << endl;
    }
}

int main(int argc, char * args[])
{
    //keep a log file
    fstream logFile;
    logFile.open("Log.txt", fstream::out | fstream::app);

    try
    {    
        //initialize the 64 bit arrays
        initInt64();
        srand(0);

        int mode = MODE_HELP;
        //set depth to default 8
        int maxDepth = 8;

        //set hash size to default 50MB.
        Int64 hashTableBytes = 50 * 1024 * 1024;

        string positionFile;
        string moveFile;
        string gamestateFile;
        string evalWeightFile = string("evalWeights/weights.txt");

        //parse the arguments to see what behavior is desired.
        for (int i = 1; i < argc; ++i)
        {
            if (string(args[i]) == string("--help"))
            {
                mode = MODE_HELP;
            }
            else if (string(args[i]) == string("--gameroom"))
            {
                //arimaa gameroom behavior, just load the 3 file names
                //(position, move, and gamestate resp.)
                //and print out next move to stdout. The file name is 
                //specified after the --gameroom flag.
                mode = MODE_GAMEROOM;
                positionFile = args[i+1];
                moveFile = args[i+2];
                gamestateFile = args[i+3];
                i += 3;
            }
            else if (string(args[i]) == string("--depth"))
            {
                //set the depth to search to the next argument
                maxDepth = atoi(args[i+1]);
                ++i;
            }
            else if (string(args[i]) == string("--hashtablesize"))
            {
                hashTableBytes = atoi(args[i+1]) * 1024 * 1024;
                ++i;
            }
            else if (string(args[i]) == string("--genmoves"))
            {
                mode = MODE_NONE;
                Board board;
                positionFile = args[i+1];
                board.loadPositionFile(positionFile);

                vector<StepCombo> combos;
                board.genMoves(combos);
                cout << "Generated these moves:\n";
                for (int i = 0; i < combos.size(); i++)
                {
                    cout << combos[i].toString() << endl;
                }
            }
            else if (string(args[i]) == string("--eval"))
            {
                mode = MODE_NONE;
                Board board;
                positionFile = args[i+1];
                board.loadPositionFile(positionFile);

                Eval eval;
                eval.loadWeights(evalWeightFile);
                cout << eval.evalBoard(board, board.sideToMove) << endl;
            }
            else if (string(args[i]) == string("--test"))
            {
				mode = MODE_NONE;

                vector<int> intvec;
                for (int i = 0; i < 1000; ++i)
                {
                    intvec.push_back(rand());
                }

                maxHeapCreate<int>(intvec);
                cout << "Size is " << intvec.size() << endl;
                while (!intvec.empty())
                {
                    cout << maxHeapGetTopAndRemove(intvec) << endl;
                }
            }

            //ignore any other flags
        }
         
        if (mode == MODE_HELP)
        {
            //print some information to standard output
            cout << "Jr Arimaa Bot\n";
            cout << "Usage: jrarimaabot [flags]\n\n";
            cout << "Flags:\n\n";
            cout << "--help\nDisplays this message\n\n";
            cout << "--gameroom positionFile moveFile gamestateFile\n"
                 << "Behaves according to the arimaa gameroom specification\n\n";
            cout << "--depth max\nSets the max search depth. Defaults to 4\n\n";
            cout << "--hashtablesize num\nSets the size of the hash table in"
                 << " MB. Defaults to 50\n\n";
            cout << "--genmoves positionFile\nDisplays the set of moves that"
                 << " the move generator generates from a position\n\n";
            cout << "--eval positionFile\nDisplays the static evaluation"
                 << " score the evaluator returns from a position\n\n";
        }

            
        if (mode == MODE_GAMEROOM)
        {
            gameroom(logFile, positionFile, moveFile, gamestateFile,
                     evalWeightFile, maxDepth, hashTableBytes);
        }

        logFile.flush();
        logFile.close();
    }
        
    catch (Error error)
    {
        logFile << "Caught Error: \n"
                << error << endl;
        logFile.flush();
        logFile.close();
        return 1;
    }

    catch (exception * e)
    {
        logFile << "Caught std exception: \n"
                << e->what() << endl;
        logFile.flush();
        logFile.close();
        return 1;
    }

    return 0;
}
