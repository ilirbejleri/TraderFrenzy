#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "gba.h"

//contract with price, probabilty to payout, and potential payout
typedef struct {
    int price;            
    int percentToPayout;   
    int potentialPayout;  
} Contract;

//Rectangle for collision detetction
typedef struct {
    int row;
    int col;
    int width;
    int height;
} Rectangle;

int checkCollision(Rectangle a, Rectangle b);

void generateContract(Contract* contract);

int payout(Contract contract);
#endif

