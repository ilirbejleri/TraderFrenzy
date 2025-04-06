#ifndef MAIN_H
#define MAIN_H

#include "gba.h"
#include "gameplay.h"

// TODO: Create any necessary structs

/*
* For example, for a Snake game, one could be:
*
* struct snake {
*   int heading;
*   int length;
*   int row;
*   int col;
* };
*
* Example of a struct to hold state machine data:
*
* struct state {
*   int currentState;
*   int nextState;
* };
*
*/

//player instance
typedef struct{
    int money;
    
} Player;



//Station for contract management
typedef struct {
    Rectangle rect;
    int hasContract;
    int contractTimer;
    int respawnTimer;
    Contract contract;
} Station;

#endif
