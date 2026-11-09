#include "UI.h"
#include "zmarble.h"
#include "MoR/MoR.h"
#include "../zengine.h"

static unsigned char UIScreen = 0;
static float UIAnimTimer = 0.f;

//----MAIN MENU---//
static Sprite* MoRUI = NULL;
static Sprite* MoRSprite = NULL;

//-------MoR------//
/* color customization */
static Sprite* colorNormal = NULL;
static Sprite* colorWarm = NULL;
static Sprite* colorCold = NULL;
static Sprite* colorOcean = NULL;
static Sprite* colorFairytale = NULL;
static Sprite* colorCRAZY = NULL;
static Sprite* colorRANDOM = NULL;

/* values */
static Sprite* multiplyVal = NULL;
static Sprite* gravityVal = NULL;
static Sprite* boardSizeVal = NULL;
static Sprite* multiplyerMultiplierVal = NULL;
static Sprite* multiplyScaleTimerVal = NULL;
static Sprite* multiplyStartSizeVal = NULL;

/* projectile type */
static Sprite* projectileNormal = NULL;
static Sprite* projectileCrazy = NULL;
static Sprite* projectileRandom = NULL;

/* bools */
static Sprite* multiplyMultiplierActivated = NULL;
static Sprite* multiplyScaleSize = NULL;

void initMainMenuUI(void) {
    createTexture("assets/img/ZMARBLE.png", 1.f, 0);
    createTexture("assets/img/ZMARBLE.png", 1.f, 1);

    MoRUI = createSprite(0.f, -.6f, 3.2f, .8f, 0.f, 0);
    MoRSprite = createSprite(0.f, .5f, .2f, .2f, 0.f, 1);
}

void initMoRUI(void) {
    /* color customization */
    colorNormal    = createSprite(-1.2f, -.7f, .3f, .3f, 0.f, 0);
    colorWarm      = createSprite(-.8f, -.7f, .3f, .3f, 0.f, 0);
    colorCold      = createSprite(-.4f, -.7f, .3f, .3f, 0.f, 0);
    colorOcean     = createSprite(0.f, -.7f, .3f, .3f, 0.f, 0);
    colorFairytale = createSprite(.4f, -.7f, .3f, .3f, 0.f, 0);
    colorCRAZY     = createSprite(.8f, -.7f, .3f, .3f, 0.f, 0);
    colorRANDOM    = createSprite(1.2f, -.7f, .3f, .3f, 0.f, 0);

    /* values */
    multiplyVal             = createSprite(.2f, -.2f, .3f, .3f, 0.f, 0);
    gravityVal              = createSprite(.6f, -.2f, .3f, .3f, 0.f, 0);
    boardSizeVal            = createSprite(-.2f, -.2f, .3f, .3f, 0.f, 0);
    multiplyerMultiplierVal = createSprite(1.f, -.2f, .3f, .3f, 0.f, 0);
    multiplyScaleTimerVal   = createSprite(-.6f, -.2f, .3f, .3f, 0.f, 0);
    multiplyStartSizeVal    = createSprite(-1.f, -.2f, .3f, .3f, 0.f, 0);

    /* projectile type */
    projectileNormal = createSprite(.5f, .3f, .4f, .3f, 0.f, 0);
    projectileCrazy  = createSprite(0.f, .3f, .4f, .3f, 0.f, 0);
    projectileRandom = createSprite(-.5f, .3f, .4f, .3f, 0.f, 0);

    /* bools */
    multiplyMultiplierActivated = createSprite(.2f, .8f, .3f, .3f, 0.f, 0);
    multiplyScaleSize           = createSprite(-.2f, .8f, .3f, .3f, 0.f, 0);
}

void tickUI(void) {
    UIAnimTimer += deltaTime;
    switch(UIScreen) {
    case 0:
        //if (RGFW_isKeyPressed(RGFW_keyEnter)) {
            UIScreen = 1;
            deleteAllSprites();
            initMoRUI();
            break;
        // }
        // MoRUI->scale[0] += ((1.6f + ((sin(UIAnimTimer) + .5f) / 8.f)) - MoRUI->scale[0]) * 2.f * deltaTime;
        // MoRUI->scale[1] += ((.3f  + ((sin(UIAnimTimer / 1.25f) + .6f) / 8.f))- MoRUI->scale[1]) * 2.f * deltaTime;
        // break;
    case 1:
        //if (RGFW_isKeyPressed(RGFW_keyEnter)) {
            gameState = 1;
            deleteAllSprites();
            initMoR();
            break;
        // }
        // break;
    case 2:
        break;
    case 3:
        break;
    }
}
