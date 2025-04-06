#include "gameplay.h"

int checkCollision(Rectangle a, Rectangle b) {
    if (a.row < b.row + b.height &&
        a.row + a.height > b.row &&
        a.col < b.col + b.width &&
        a.col + a.width > b.col) {
        return 1;
    }
    return 0;
}

void generateContract(Contract* contract) {
    int ev = randint(50, 100);


    contract->percentToPayout = randint(40,80);

    contract->potentialPayout = ev*100/(contract->percentToPayout);
    contract->price = randint(ev-30, ev+30);


}


int payout(Contract contract) {
   int x = randint(1,100);

   if(contract.percentToPayout >=x) {
     return 1;
   } else {return 0;}
}


