#include "zmarble.h"
#include "UI.h"
#include "MoR/MoR.h"

unsigned short gameState = 0;

void initZMarble(void) {
    initMainMenuUI();
}

void tickZMarble(void) {
    switch(gameState) {
    case 0:
        tickUI();
        break;
    case 1:
        tickMoR();
        break;
    }
}

void deinitZMarble(void) {
    //deinitMoR();
}
