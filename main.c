#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "title.h"
#include "play.h"
#include "gba.h"
#include "gameplay.h"
#include "computer.h"
#include "sprite.h"

#define NUM_STATIONS 6

enum gba_state {
    START,
    PLAY,
    CONTRACT,
    RESULT,
};



Station stations[NUM_STATIONS] = {
    { {0, 0, 30, 27}, 1, 0, 0, {0,0,0} },
    { {0, 210, 30, 27}, 1, 0, 0, {0,0,0} },
    { {133, 0, 30, 27}, 1, 0, 0, {0,0,0} },
    { {133, 210, 30, 27}, 1, 0, 0, {0,0,0} },
    { {63, 49, 30, 27}, 1, 0, 0, {0,0,0} },
    { {62, 145, 30, 27}, 1, 0, 0, {0,0,0} },
};

int main(void) {
    // Initialize player.
    Player player;
    player.money = 0;
    
    REG_DISPCNT = MODE3 | BG2_ENABLE;
    u32 previousButtons = BUTTONS;
    u32 currentButtons = BUTTONS;
    enum gba_state state = START;
    
    // Variables for the player's sprite in PLAY state.
    int playerRow = 80, playerCol = 120;
    int playerWidth = 10, playerHeight = 20; 
    int oldPlayerRow = 80, oldPlayerCol = 80;
    int startRow = 80, startCol = 120;
    int playStartTime = 0;
    int timerHeight = 10, timerWidth = 12;
    int timerLeft = (WIDTH - timerWidth) / 2;
    int currentStationIndex = -1;
    int firstFrame = 1;
    int firstDisplay = 1;
    int firstResult = 1;
    int firstStart = 1;
    
    // Sprite variables for the bouncing sprite in START state.
    int spriteRow = 10, spriteCol = 10;       
    int spriteRowVelocity = 1, spriteColVelocity = 1; 
    // The bouncing sprite is 10x20.
    
    while (1) {
        
        // Wait for the vertical blank once per frame.
        waitForVBlank();
        currentButtons = BUTTONS;
        
        // Update contract timers for all stations regardless of game state.
        for (int i = 0; i < NUM_STATIONS; i++) {
            if (stations[i].hasContract) {
                stations[i].contractTimer++;
                if (stations[i].contractTimer > 900) {  // 15 seconds at 60 fps
                    stations[i].hasContract = 0;
                    stations[i].contractTimer = 0;
                    stations[i].respawnTimer = 0;
                }
            } else {
                stations[i].respawnTimer++;
                if (stations[i].respawnTimer > 300) {  // 5 seconds at 60 fps
                    stations[i].hasContract = 1;
                    stations[i].contractTimer = 0;
                    stations[i].respawnTimer = 0;
                    generateContract(&stations[i].contract); 
                }
            }
        }
        
        switch (state) {
            case START:
                if (firstStart) {
                    // Draw the title background.
                    drawFullScreenImageDMA(title);
                    firstStart = 0;
                } else {
                    // Undraw the sprite at its previous location.
                    undrawImageDMA(spriteRow - spriteRowVelocity, spriteCol - spriteColVelocity, 10, 20, title);
                }
                // Update the bouncing sprite position.
                spriteRow += spriteRowVelocity;
                spriteCol += spriteColVelocity;
                
                // Bounce off vertical boundaries (HEIGHT assumed 160).
                if (spriteRow < 0) { 
                    spriteRow = 0; 
                    spriteRowVelocity = -spriteRowVelocity; 
                }
                if (spriteRow > HEIGHT - 20) { 
                    spriteRow = HEIGHT - 20; 
                    spriteRowVelocity = -spriteRowVelocity; 
                }
                // Bounce off horizontal boundaries (WIDTH assumed 240).
                if (spriteCol < 0) { 
                    spriteCol = 0; 
                    spriteColVelocity = -spriteColVelocity; 
                }
                if (spriteCol > WIDTH - 10) { 
                    spriteCol = WIDTH - 10; 
                    spriteColVelocity = -spriteColVelocity; 
                }
                
                // Draw the bouncing sprite.
                drawImageDMA(spriteRow, spriteCol, 10, 20, sprite);
                
                if (KEY_DOWN(BUTTON_START, currentButtons)) {
                    state = PLAY;
                    playerRow = startRow;
                    playerCol = startCol;
                    oldPlayerRow = playerRow;
                    oldPlayerCol = playerCol;
                    playStartTime = vBlankCounter;
                    firstFrame = 1;
                    // Reset stations.
                    for (int i = 0; i < NUM_STATIONS; i++) {
                        stations[i].hasContract = 1;
                        stations[i].contractTimer = 0;
                        stations[i].respawnTimer = 0;
                    }
                    firstStart = 1;
                }
                // Initialize contracts on the start screen.
                seed_rand((int)vBlankCounter); // use for random seed
                for (int i = 0; i < NUM_STATIONS; i++) {
                    generateContract(&stations[i].contract);
                }
                break;
                
            case PLAY:
                //first load to avoid reloading background every time
                if (firstFrame) {
                    drawFullScreenImageDMA(play);
                    firstFrame = 0;
                } else {
                    undrawImageDMA(oldPlayerRow, oldPlayerCol, playerWidth, playerHeight, play);
                }
                
                // Process player movement.
                if (KEY_DOWN(BUTTON_UP, currentButtons)) playerRow--;
                if (KEY_DOWN(BUTTON_DOWN, currentButtons)) playerRow++;
                if (KEY_DOWN(BUTTON_LEFT, currentButtons)) playerCol--;
                if (KEY_DOWN(BUTTON_RIGHT, currentButtons)) playerCol++;
                
                // Constrain player within screen bounds.
                if (playerRow < 0) playerRow = 0;
                if (playerRow > HEIGHT - playerHeight) playerRow = HEIGHT - playerHeight;
                if (playerCol < 0) playerCol = 0;
                if (playerCol > WIDTH - playerWidth) playerCol = WIDTH - playerWidth;
                
                // Draw the player sprite.
                drawImageDMA(playerRow, playerCol, playerWidth, playerHeight, sprite);
                oldPlayerRow = playerRow;
                oldPlayerCol = playerCol;
                
                // Update and display the game clock timer.
                {
                    int elapsedFrames = vBlankCounter - playStartTime;
                    int elapsedSeconds = elapsedFrames / 60;
                    int remainingTime = 60 - elapsedSeconds;
                    undrawImageDMA(150, timerLeft, timerWidth, timerHeight, play);
                    char timeBuffer[3];
                    sprintf(timeBuffer, "%02d", remainingTime);
                    drawString(150, timerLeft, timeBuffer, WHITE);
                    if (remainingTime <= 0) {
                        state = RESULT;
                        break;
                    }
                }
                
                // Display current player money.
                {
                    char moneyBuffer[30];
                    sprintf(moneyBuffer, "%d", player.money);
                    drawString(150, timerLeft + timerWidth + 10, moneyBuffer, WHITE);
                }
                
                // Draw green indicators for stations with an active contract.
                for (int i = 0; i < NUM_STATIONS; i++) {
                    if (stations[i].hasContract) {
                        drawRectDMA(stations[i].rect.row + 10, stations[i].rect.col + 10, 10, 10, GREEN);
                    } else {
                        undrawImageDMA(stations[i].rect.row + 10, stations[i].rect.col + 10, 10, 10, play);
                    }
                }
                
                // Check for collision with any station that has a contract.
                {
                    Rectangle playerRect = { playerRow, playerCol, playerWidth, playerHeight };
                    for (int i = 0; i < NUM_STATIONS; i++) {
                        if (stations[i].hasContract && checkCollision(playerRect, stations[i].rect)) {
                            currentStationIndex = i;
                            state = CONTRACT;
                            break;
                        }
                    }
                }
                
                if (KEY_DOWN(BUTTON_SELECT, currentButtons)) state = START;
                break;
                
            case CONTRACT:
                //first display of contract details to avoid reloading
                if (firstDisplay) {
                    drawFullScreenImageDMA(computer);
                    Contract *contract = &stations[currentStationIndex].contract;
                    char buffer[30];
                    
                    // Display the Price.
                    sprintf(buffer, "%d", contract->price);
                    drawString(50, 170, buffer, WHITE);
                    
                    // Display the Percent to Payout.
                    sprintf(buffer, "%d%%", contract->percentToPayout);
                    drawString(80, 170, buffer, WHITE);
                    
                    // Display the Potential Payout.
                    sprintf(buffer, "%d", contract->potentialPayout);
                    drawString(110, 170, buffer, WHITE);
                    
                    firstDisplay = 0;
                }
                
                //display remaining time
                {
                    int elapsedFrames = vBlankCounter - playStartTime;
                    int elapsedSeconds = elapsedFrames / 60;
                    int remainingTime = 60 - elapsedSeconds;
                    undrawImageDMA(150, timerLeft, timerWidth, timerHeight, computer);
                    char timeBuffer[3];
                    sprintf(timeBuffer, "%02d", remainingTime);
                    drawString(150, timerLeft, timeBuffer, WHITE);
                    if (remainingTime <= 0) {
                        state = RESULT;
                        break;
                    }
                }
                
                //display current money
                {
                    char moneyBuffer[30];
                    sprintf(moneyBuffer, "%d", player.money);
                    drawString(150, timerLeft + timerWidth + 10, moneyBuffer, WHITE);
                }
                
                //kick out of countract if expires while viewing it
                if (stations[currentStationIndex].hasContract == 0) {
                    firstFrame = 1;
                    firstDisplay = 1;
                    state = PLAY;
                    break;
                }
                
                //button action logic
                if (KEY_DOWN(BUTTON_B, currentButtons)) {
                    playerRow = startRow;
                    playerCol = startCol;
                    oldPlayerRow = playerRow;
                    oldPlayerCol = playerCol;
                    firstFrame = 1;
                    firstDisplay = 1;
                    state = PLAY;
                } else if (KEY_DOWN(BUTTON_A, currentButtons)) {
                    int paid = payout(stations[currentStationIndex].contract);
                    if (paid) {
                        player.money += stations[currentStationIndex].contract.potentialPayout - stations[currentStationIndex].contract.price;
                    } else {
                        player.money -= stations[currentStationIndex].contract.price;
                    }
                    if (currentStationIndex != -1) {
                        stations[currentStationIndex].hasContract = 0;
                        stations[currentStationIndex].contractTimer = 0;
                        stations[currentStationIndex].respawnTimer = 0;
                    }
                    firstFrame = 1;
                    firstDisplay = 1;
                    state = PLAY;
                }
                break;
        	case RESULT:
                if (firstResult) {
                    fillScreenDMA(BLACK);
                    drawString(80, 40, "Time's Up!", WHITE);
                    char moneyBuffer[30];
                    sprintf(moneyBuffer, "Money: %d", player.money);
                    drawString(100, 60, moneyBuffer, WHITE);
                    firstResult = 0;
                }

                if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
                    
                    firstResult = 1;
                    state = START;
                player.money=0;
                }
                break;        
                    }
                    
                    UNUSED(previousButtons);
                    previousButtons = currentButtons;
                }
                
                return 0;
}

