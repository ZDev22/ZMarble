#define _POSIX_C_SOURCE 199309L

/* projectile types and color types */
#define NORMAL 0
#define CRAZY 1
#define RANDOM 2

/* only color types */
#define WARM 10
#define COLD 11
#define FAIRYTALE 12
#define OCEAN 13
#define WOOD 14

#define MAX_COLORS 14
/* colors */
#define RED 0
#define GREEN 1
#define BLUE 2
#define YELLOW 3
#define PINK 4
#define PURPLE 5
#define LIGHT_BLUE 6
#define ORANGE 7
#define BROWN 8
#define LAVENDER 9
#define BLACK 10
#define WHITE 11
#define LIGHT_BROWN 12
#define DARK_RED 13
#define DARK_BLUE 14

/* customizable: */
#define SQUARES 128
#define POINTS_MULT 2.f
#define PROJECTILE_TYPE NORMAL
#define COLOR_TYPE NORMAL

#if PROJECTILE_TYPE == CRAZY
    #define PROJECTILE_TIME ((float)-SQUARES / 128.f) + 3.f
#elif PROJECTILE_TYPE == NORMAL
    #define PROJECTILE_TIME 1.f
#elif PROJECTILE_TYPE == RANDOM
    #define PROJECTILE_TIME 0.f
#endif

/* non-customizable: */
#define BOARD_SCALE .5f
#define GRAVITY 2.5f

#include "zmarble.h"
#include "../zengine.h"
#include "../zcollide.h"
#include "../ztext.h"
#include "../sprites.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define MARBLES 64

float boardUpdateTimer = 0.f;

typedef struct Marble {
    float velocity[2];
    float radius;
    float weight;
    _Bool stale;

    Sprite* sprite;
} Marble;

typedef struct Projectile {
    float velocity[2];
    unsigned int health;

    Sprite* sprite;
} Projectile;

typedef struct Points {
    float points;
    unsigned int releasingPoints;
    _Bool update;
    _Bool releasing;

    Sprite* sprite;
} Points;

static unsigned char board[SQUARES * SQUARES];
static unsigned char* boardImage;

static Marble marbles[MARBLES];
static unsigned char marbleSize = 0;
static Projectile projectiles[8192];
static unsigned short projectileSize = 0;
static Points points[4];
static float projectileTimer[4];
static float projectileTime = 0.f;
static unsigned long long randomstate = 0;

static unsigned char* marbleColor;
static unsigned char* colors;

static float timer = 0.f;

Sprite* boardSprite = NULL;
Sprite* releaseSprite = NULL;
Sprite* multiplySprite = NULL;
Sprite* wall1Sprite = NULL;
Sprite* wall2Sprite = NULL;

static void sRandom(void) {
    struct timespec tsm;
    clock_gettime(CLOCK_REALTIME, &tsm);

    randomstate = (unsigned long long)tsm.tv_sec * 1000000000ULL + (unsigned long long)tsm.tv_nsec;
}

static long long Random(const long long min, const long long max) {
    randomstate ^= randomstate << 13;
    randomstate ^= randomstate >> 17;
    randomstate ^= randomstate << 5;

    return min + (long long)(randomstate % (unsigned long long)(max - min + 1));
}


static float modFloat(float a, float b) {
    while (a >= b) { a -= b; }
    return a;
}

void createMarbleColor(unsigned char R, unsigned char G, unsigned char B, unsigned int index) {
    for (unsigned char i = 0; i < 16; i++) {
        marbleColor[i * 4] = R;
        marbleColor[i * 4 + 1] = G;
        marbleColor[i * 4 + 2] = B;
        marbleColor[i * 4 + 3] = 255;
    }
    marbleColor[3] = 0;
    marbleColor[15] = 0;
    marbleColor[63] = 0;
    marbleColor[51] = 0;
    createTextureExt(marbleColor, index, 4, 4, 64, VK_FORMAT_R8G8B8A8_SRGB);
}

void setColor(unsigned char* color, unsigned short startingIndex, unsigned char colorID) {
    switch (colorID) {
    case 0: // red
        color[startingIndex + 0] = 255;
        color[startingIndex + 1] = 0;
        color[startingIndex + 2] = 0;
        break;
    case 1: // green
        color[startingIndex + 0] = 0;
        color[startingIndex + 1] = 255;
        color[startingIndex + 2] = 0;
        break;
    case 2: // blue
        color[startingIndex + 0] = 0;
        color[startingIndex + 1] = 0;
        color[startingIndex + 2] = 255;
        break;
    case 3: // yellow
        color[startingIndex + 0] = 255;
        color[startingIndex + 1] = 255;
        color[startingIndex + 2] = 0;
        break;
    case 4: // pink
        color[startingIndex + 0] = 255;
        color[startingIndex + 1] = 100;
        color[startingIndex + 2] = 220;
        break;
    case 5: // purple
        color[startingIndex + 0] = 180;
        color[startingIndex + 1] = 50;
        color[startingIndex + 2] = 180;
        break;
    case 6: // light blue
        color[startingIndex + 0] = 0;
        color[startingIndex + 1] = 255;
        color[startingIndex + 2] = 255;
        break;
    case 7: // orange
        color[startingIndex + 0] = 255;
        color[startingIndex + 1] = 120;
        color[startingIndex + 2] = 0;
        break;
    case 8: // brown
        color[startingIndex + 0] = 50;
        color[startingIndex + 1] = 0;
        color[startingIndex + 2] = 0;
        break;
    case 9: // lavender
        color[startingIndex + 0] = 255;
        color[startingIndex + 1] = 220;
        color[startingIndex + 2] = 255;
        break;
    case 10: // black
        color[startingIndex + 0] = 0;
        color[startingIndex + 1] = 0;
        color[startingIndex + 2] = 0;
        break;
    case 11: // white
        color[startingIndex + 0] = 255;
        color[startingIndex + 1] = 255;
        color[startingIndex + 2] = 255;
        break;
    case 12: // light brown
        color[startingIndex + 0] = 70;
        color[startingIndex + 1] = 30;
        color[startingIndex + 2] = 0;
        break;
    case 13: // dark red
        color[startingIndex + 0] = 150;
        color[startingIndex + 1] = 0;
        color[startingIndex + 2] = 0;
        break;
    case 14: // dark blue
        color[startingIndex + 0] = 0;
        color[startingIndex + 1] = 0;
        color[startingIndex + 2] = 150;
        break;
    }
}


static void createProjectile(const unsigned char corner, unsigned int health) {
    if (projectileSize >= 8192) { return; }

#if PROJECTILE_TYPE == RANDOM
    projectiles[projectileSize].velocity[0] = (float)Random(-10075, 10075) / 10000.f;
    projectiles[projectileSize].velocity[1] = (float)Random(-10075, 10075) / 10000.f;
#else
    projectiles[projectileSize].velocity[0] = -1.015f * fabs((modFloat(projectileTime, 2.f)) - 1.f) + 1.0075f;
    projectiles[projectileSize].velocity[1] = -1.015f * fabs((modFloat((projectileTime + 1.f), 2.f)) - 1.f) + 1.0075f;
#endif

    switch (corner) {
    case 0:
        projectiles[projectileSize].sprite = createspriteptr(-BOARD_SCALE + .005f, -BOARD_SCALE + .005f, .01f, .01f, 0.f, 4);
        break;
    case 1:
        projectiles[projectileSize].velocity[0] = -projectiles[projectileSize].velocity[0];
        projectiles[projectileSize].sprite = createspriteptr(BOARD_SCALE - .005f, -BOARD_SCALE + .005f, .01f, .01f, 0.f, 5);
        break;
    case 2:
        projectiles[projectileSize].velocity[1] = -projectiles[projectileSize].velocity[1];
        projectiles[projectileSize].sprite = createspriteptr(-BOARD_SCALE + .005f, BOARD_SCALE - .005f, .01f, .01f, 0.f, 6);
        break;
    case 3:
        projectiles[projectileSize].velocity[0] = -projectiles[projectileSize].velocity[0];
        projectiles[projectileSize].velocity[1] = -projectiles[projectileSize].velocity[1];
        projectiles[projectileSize].sprite = createspriteptr(BOARD_SCALE - .005f, BOARD_SCALE - .005f, .01f, .01f, 0.f, 7);
        break;
    }

    projectiles[projectileSize].health = health;
    projectiles[projectileSize].velocity[0] /= (float)SQUARES / 90.f;
    projectiles[projectileSize].velocity[1] /= (float)SQUARES / 90.f;
    projectileSize++;

    points[corner].releasingPoints -= health;
    points[corner].update = 1;
    if (points[corner].releasingPoints == 0) {
        points[corner].releasing = 0;
    }
}

static void setSquare(unsigned int index) {
    switch(board[index]) {
#if COLOR_TYPE == CRAZY
    case 0:
        if (Random(0, 1) == 0) {
            setColor(boardImage, index * 4, WHITE);
        }
        else {
            setColor(boardImage, index * 4, BLACK);
        }
        boardImage[index * 4 + 3] = 255;
        return;
#else
    case 0:
        boardImage[index * 4] = colors[0];
        boardImage[index * 4 + 1] = colors[1];
        boardImage[index * 4 + 2] = colors[2];
        boardImage[index * 4 + 3] = 255;
        return;
#endif
    case 1:
        boardImage[index * 4] = colors[3];
        boardImage[index * 4 + 1] = colors[4];
        boardImage[index * 4 + 2] = colors[5];
        boardImage[index * 4 + 3] = 255;
        return;
    case 2:
        boardImage[index * 4] = colors[6];
        boardImage[index * 4 + 1] = colors[7];
        boardImage[index * 4 + 2] = colors[8];
        boardImage[index * 4 + 3] = 255;
        return;
    case 3:
        boardImage[index * 4] = colors[9];
        boardImage[index * 4 + 1] = colors[10];
        boardImage[index * 4 + 2] = colors[11];
        boardImage[index * 4 + 3] = 255;
        return;
    }
}

static void createMarble(float positionx, float positiony, float scalex, float scaley, float rotation, unsigned int textureIndex, float velocityx, float velocityy, _Bool stale) {
    marbles[marbleSize].velocity[0] = velocityx;
    marbles[marbleSize].velocity[1] = velocityy;
    marbles[marbleSize].radius = scalex / 2.f;
    marbles[marbleSize].weight = scalex * 20.f;
    marbles[marbleSize].stale = stale;

    marbles[marbleSize].sprite = createspriteptr(positionx, positiony, scalex, scaley, rotation, textureIndex);
    marbleSize++;
}

void initBoard(void) {
    sRandom();

    boardImage = (unsigned char*)malloc(SQUARES * SQUARES * 4);
    colors = (unsigned char*)malloc(12);
    marbleColor = (unsigned char*)malloc(64);

#if COLOR_TYPE == NORMAL
    /* board */
    setColor(colors, 0, RED);
    setColor(colors, 3, GREEN);
    setColor(colors, 6, BLUE);
    setColor(colors, 9, YELLOW);
    /* marbles */
    createMarbleColor(255, 0, 0, 0);
    createMarbleColor(0, 255, 0, 1);
    createMarbleColor(0, 0, 255, 2);
    createMarbleColor(255, 255, 0, 3);
    /* projectiles */
    createMarbleColor(140, 35, 35, 4);
    createMarbleColor(35, 140, 35, 5);
    createMarbleColor(35, 35, 140, 6);
    createMarbleColor(140, 140, 35, 7);
#elif COLOR_TYPE == CRAZY
    /* board */
    setColor(colors, 9, RED);
    createMarbleColor(140, 35, 35, 3);
#elif COLOR_TYPE == WARM
    /* board */
    setColor(colors, 0, RED);
    setColor(colors, 3, ORANGE);
    setColor(colors, 6, YELLOW);
    setColor(colors, 9, BROWN);
    /* marbles */
    createMarbleColor(255, 0, 0, 0);
    createMarbleColor(255, 120, 0, 1);
    createMarbleColor(255, 255, 0, 2);
    createMarbleColor(50, 0, 0, 3);
    /* projectiles */
    createMarbleColor(140, 35, 35, 4);
    createMarbleColor(140, 70, 35, 5);
    createMarbleColor(140, 140, 35, 6);
    createMarbleColor(0, 0, 0, 7);
#elif COLOR_TYPE == COLD
    /* board */
    setColor(colors, 0, GREEN);
    setColor(colors, 3, BLUE);
    setColor(colors, 6, LIGHT_BLUE);
    setColor(colors, 9, PURPLE);
    /* marbles */
    createMarbleColor(0, 255, 0, 0);
    createMarbleColor(0, 0, 255, 1);
    createMarbleColor(0, 255, 255, 2);
    createMarbleColor(180, 50, 180, 3);
    /* projectiles */
    createMarbleColor(35, 140, 35, 4);
    createMarbleColor(35, 35, 140, 5);
    createMarbleColor(35, 140, 140, 6);
    createMarbleColor(80, 0, 80, 7);
#elif COLOR_TYPE == FAIRYTALE
    /* board */
    setColor(colors, 0, PINK);
    setColor(colors, 3, LIGHT_BLUE);
    setColor(colors, 6, WHITE);
    setColor(colors, 9, LAVENDER);
    /* marbles */
    createMarbleColor(255, 100, 220, 0);
    createMarbleColor(0, 255, 255, 1);
    createMarbleColor(255, 255, 255, 2);
    createMarbleColor(255, 220, 255, 3);
    /* projectiles */
    createMarbleColor(140, 60, 160, 4);
    createMarbleColor(35, 140, 140, 5);
    createMarbleColor(150, 150, 150, 6);
    createMarbleColor(150, 110, 150, 7);
#elif COLOR_TYPE == OCEAN
    /* board */
    setColor(colors, 0, BLUE);
    setColor(colors, 3, LIGHT_BLUE);
    setColor(colors, 6, DARK_BLUE);
    setColor(colors, 9, PURPLE);
    /* marbles */
    createMarbleColor(0, 0, 255, 0);
    createMarbleColor(0, 255, 255, 1);
    createMarbleColor(0, 0, 150, 2);
    createMarbleColor(180, 50, 180, 3);
    /* projectiles */
    createMarbleColor(0, 0, 150, 4);
    createMarbleColor(0, 150, 150, 5);
    createMarbleColor(0, 0, 255, 6);
    createMarbleColor(90, 25, 90, 7);
#elif COLOR_TYPE == WOOD
    /* board */
    setColor(colors, 0, BROWN);
    setColor(colors, 3, LIGHT_BROWN);
    setColor(colors, 6, BLACK);
    setColor(colors, 9, DARK_RED);
    /* marbles */
    createMarbleColor(50, 0, 0, 0);
    createMarbleColor(70, 20, 0, 1);
    createMarbleColor(0, 0, 0, 2);
    createMarbleColor(150, 0, 0, 3);
    /* projectiles */
    createMarbleColor(0, 0, 0, 4);
    createMarbleColor(150, 50, 0, 5);
    createMarbleColor(150, 150, 150, 6);
    createMarbleColor(255, 0, 0, 7);
#endif

#if COLOR_TYPE == RANDOM
    unsigned char types[4] = {0, 0, 0, 0};
    while (types[0] == types[1] || types[0] == types[2] || types[0] == types[3] || types[1] == types[0] || types[1] == types[2] || types[1] == types[3] || types[2] == types[0] || types[2] == types[1] || types[2] == types[3]) {
        types[0] = Random(0, MAX_COLORS);
        types[1] = Random(0, MAX_COLORS);
        types[2] = Random(0, MAX_COLORS);
        types[3] = Random(0, MAX_COLORS);
    }
    for (unsigned char i = 0; i < 4; i++) {
        setColor(colors, i * 3, types[i]);
        setColor(marbleColor, 0, types[i]);
        createMarbleColor(marbleColor[0], marbleColor[1], marbleColor[2], i);
    }
#endif
    /* load board */
    for (unsigned int i = 0; i < SQUARES * SQUARES; i++) {
        if (i < (SQUARES * SQUARES) / 2) {
            if (i % SQUARES < SQUARES / 2) { board[i] = 0; }
            else { board[i] = 1; }
        }
        else {
            if (i % SQUARES < SQUARES / 2) { board[i] = 2; }
            else { board[i] = 3; }
        }
        setSquare(i);
    }

    createTextureExt(boardImage, 12, SQUARES, SQUARES, SQUARES * SQUARES * 4, VK_FORMAT_R8G8B8A8_SRGB);

    /* load marbles */
    createTexture("assets/img/gray.png", 1.f, 9);
    createTexture("assets/img/multiply.png", 1.f, 10);
    createTexture("assets/img/release.png", 1.f, 11);

    for (unsigned int i = 0; i < 12; i++) { /* spawn normal marbles */
        createMarble((float)Random(800, 1200) / 1000.f, (float)Random(-2000, -1100) / 1000.f, .025f, .025f, 0.f, i % 4, (float)Random(-500, 500) / 1000.f, (float)Random(-1000, 1000) / 1000.f, 0);
    }

    /* load obsticles */
    createMarble(.61f, -.8f, .2f, .2f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(1.21f, -.8f, .2f, .2f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(.91f, -.6f, .3f, .3f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(1.13f, -.4f, .07f, .07f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(1.13f, .4f, .07f, .07f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(.7f, -.2f, .1f, .1f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(1.21f, -.05f, .16f, .16f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(.85f, 0.f, .05f, .05f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(.75f, .15f, .05f, .05f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(.9f, .2f, .05f, .05f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(1.f, .6f, .08f, .08f, 0.f, 9, 0.f, 0.f, 1);
    createMarble(.61f, .52f, .09f, .09f, 0.f, 9, 0.f, 0.f, 1);

    /* load points */
    for (unsigned char i = 0; i < 4; i++) {
        points[i].points = 1.f;
        points[i].releasingPoints = 0;
        points[i].update = 0;
        points[i].releasing = 0;
        projectileTimer[i] = 0.f;
    }

    loadFont("assets/fonts/CubicCoreMono.ttf", 0);
    createText("1", 128, 32, 16, 0, 13);
    createText("1", 128, 32, 16, 0, 14);
    createText("1", 128, 32, 16, 0, 15);
    createText("1", 128, 32, 16, 0, 16);

    points[0].sprite = createspriteptr(-BOARD_SCALE + .05f, -BOARD_SCALE - .05f, .4f, .1f, 0.f, 13);
    points[1].sprite = createspriteptr(BOARD_SCALE + .05f, -BOARD_SCALE - .05f, .4f, .1f, 0.f, 14);
    points[2].sprite = createspriteptr(-BOARD_SCALE + .05f, BOARD_SCALE + .1f, .4f, .1f, 0.f, 15);
    points[3].sprite = createspriteptr(BOARD_SCALE + .05f, BOARD_SCALE + .1f, .4f, .1f, 0.f, 16);

    boardSprite = createspriteptr(0.f, 0.f, BOARD_SCALE * 2.f, BOARD_SCALE * 2.f, 0.f, 12);
    releaseSprite = createspriteptr(1.f, 1.f, .42f, .1f, 0.f, 11);
    multiplySprite = createspriteptr(.725f, 1.f, .25f, .1f, 0.f, 10);
    wall1Sprite = createspriteptr(.6f, 0.f, .025f, 3.f, 0.f, 9);
    wall2Sprite = createspriteptr(1.218f, 0.f, .025f, 3.f, 0.f, 9);
}

void updateBoard(void) {
    /* update marbles */
    for (unsigned int i = 0; i < marbleSize; i++) {
        if (marbles[i].stale) { continue; }
        marbles[i].velocity[1] += GRAVITY * deltaTime;
        marbles[i].sprite->position[0] += marbles[i].velocity[0] * deltaTime;
        marbles[i].sprite->position[1] += marbles[i].velocity[1] * deltaTime;

        for (unsigned int j = 0; j < marbleSize; j++) {
            if (j == i) { continue; }
            if (zcollide_circleCollision(marbles[i].sprite->position[0], marbles[i].sprite->position[1], marbles[i].radius, marbles[j].sprite->position[0], marbles[j].sprite->position[1], marbles[j].radius)) {
                float normalx = marbles[j].sprite->position[0] - marbles[i].sprite->position[0];
                float normaly = marbles[j].sprite->position[1] - marbles[i].sprite->position[1];
                float length = sqrtf(normalx * normalx + normaly * normaly);

                if (length == 0.f) { continue; }

                normalx /= length;
                normaly /= length;

                float velocityx = marbles[j].velocity[0] - marbles[i].velocity[0];
                float velocityy = marbles[j].velocity[1] - marbles[i].velocity[1];

                float impulse = -(((1.f + .8f) * ((velocityx * normalx) + (velocityy * normaly))) / ((1.f / marbles[i].weight) + (1.f / marbles[j].weight)));
                float impulsex = normalx * impulse;
                float impulsey = normaly * impulse;

                marbles[i].velocity[0] -= impulsex / marbles[i].weight;
                marbles[i].velocity[1] -= impulsey / marbles[i].weight;
                if (!marbles[j].stale) {
                    marbles[j].velocity[0] += impulsex / marbles[j].weight;
                    marbles[j].velocity[1] += impulsey / marbles[j].weight;
                }

                float overlap = ((marbles[i].radius + marbles[j].radius) - length) * .5f;
                float overlapx = normalx * overlap;
                float overlapy = normaly * overlap;

                marbles[i].sprite->position[0] -= overlapx;
                marbles[i].sprite->position[1] -= overlapy;
                if (!marbles[j].stale) {
                    marbles[j].sprite->position[0] += overlapx;
                    marbles[j].sprite->position[1] += overlapy;
                }
            }
        }

        if (marbles[i].sprite->position[1] > .9f) {
            if (zcollide_squareCollision(marbles[i].sprite->position[0], marbles[i].sprite->position[1], marbles[i].sprite->scale[0], marbles[i].sprite->scale[1], multiplySprite->position[0], multiplySprite->position[1], multiplySprite->scale[0], multiplySprite->scale[1])) {
                points[marbles[i].sprite->textureIndex].points *= POINTS_MULT;
                points[marbles[i].sprite->textureIndex].update = 1;

                marbles[i].sprite->position[1] = -1.125f;
                marbles[i].velocity[0] = (float)Random(-500, 500) / 1000.f;
                marbles[i].velocity[1] = 0.f;
            }
            else if (zcollide_squareCollision(marbles[i].sprite->position[0], marbles[i].sprite->position[1], marbles[i].sprite->scale[0], marbles[i].sprite->scale[1], releaseSprite->position[0], releaseSprite->position[1], releaseSprite->scale[0], releaseSprite->scale[1])) {
                points[marbles[i].sprite->textureIndex].releasingPoints += (unsigned int)points[marbles[i].sprite->textureIndex].points;
                points[marbles[i].sprite->textureIndex].points = 1.f;
                points[marbles[i].sprite->textureIndex].releasing = 1;
                points[marbles[i].sprite->textureIndex].update = 1;

                marbles[i].sprite->position[1] = -1.5f;
                marbles[i].velocity[0] = (float)Random(-1000, 1000) / 1000.f;
                marbles[i].velocity[1] = 0.f;
            }
        }
        if (marbles[i].sprite->position[0] > 1.2f || marbles[i].sprite->position[0] < .62f) {
            marbles[i].velocity[0] = -marbles[i].velocity[0] * 1.1f;
            marbles[i].sprite->position[0] += marbles[i].velocity[0] * deltaTime;
        }
    }

    /* update projectiles */
    projectileTime += (deltaTime / 3.f) * PROJECTILE_TIME;
    for (unsigned char i = 0; i < 4; i++) {
        if (points[i].releasing) {
            projectileTimer[i] += deltaTime;
            if (projectileTimer[i] > .008f) {
                createProjectile(i, (points[i].releasingPoints / 512) + 1);
                projectileTimer[i] = 0.f;
            }
        }

        if (points[i].update) {
            char pointstr[15];
            if (points[i].releasing) {
                sprintf(pointstr, "%d", points[i].releasingPoints);
            }
            else {
                sprintf(pointstr, "%d", (unsigned int)points[i].points);
            }
            createText(pointstr, 128, 32, 16, 0, i + 13);
            points[i].update = 0;
        }
    }

    _Bool redrawBoard = 0;

    for (unsigned int i = 0; i < projectileSize; i++) {
        projectiles[i].sprite->position[0] += projectiles[i].velocity[0] * deltaTime;
        projectiles[i].sprite->position[1] += projectiles[i].velocity[1] * deltaTime;

        if (projectiles[i].sprite->position[0] < -BOARD_SCALE || projectiles[i].sprite->position[0] > BOARD_SCALE) {
            projectiles[i].velocity[0] = -projectiles[i].velocity[0];
            projectiles[i].sprite->position[0] += projectiles[i].velocity[0] * deltaTime;
        }
        if (projectiles[i].sprite->position[1] < -BOARD_SCALE || projectiles[i].sprite->position[1] > BOARD_SCALE) {
            projectiles[i].velocity[1] = -projectiles[i].velocity[1];
            projectiles[i].sprite->position[1] += projectiles[i].velocity[1] * deltaTime;
        }

        unsigned int squarex = (unsigned int)((projectiles[i].sprite->position[0] + BOARD_SCALE) * (float)SQUARES);
        unsigned int squarey = (unsigned int)((projectiles[i].sprite->position[1] + BOARD_SCALE) * (float)SQUARES);
        if (squarex >= SQUARES) { continue; }
        if (squarey >= SQUARES) { continue; }

        unsigned int pos = squarey * SQUARES + squarex;
        if (board[pos] != projectiles[i].sprite->textureIndex - 4) {
            if (pos == 0 || pos == SQUARES - 1 || pos == (SQUARES * SQUARES) - SQUARES || pos == SQUARES * SQUARES - 1) {
                points[board[pos]].releasing = 0;
                points[board[pos]].update = 1;
                for (unsigned char i = 0; i < marbleSize; i++) {
                    if (marbles[i].sprite->textureIndex == board[pos]) {
                        deletesprite(marbles[i].sprite);
                        marbles[i].stale = 1;
                    }
                }
                deletesprite(points[board[pos]].sprite);
            }

            board[pos] = projectiles[i].sprite->textureIndex - 4;
            setSquare(pos);
            redrawBoard = 1;
            projectiles[i].health--;

            if (projectiles[i].health == 0) {
                projectileSize--;
                deletesprite(projectiles[i].sprite);
                projectiles[i] = projectiles[projectileSize];
                continue;
            }
        }
    }

#if COLOR_TYPE == CRAZY
    redrawBoard = 1;
    timer += deltaTime;

    colors[3] = (sin(timer) + 1.f) * 126.f;
    colors[4] = colors[3];
    colors[5] = colors[3];

    colors[6] = Random(0, 255);
    colors[7] = Random(0, 255);
    colors[8] = Random(0, 255);

    for (unsigned int i = 0; i < SQUARES * SQUARES; i++) { setSquare(i); }

    marbleColor[0] = Random(0, 255);
    createMarbleColor(marbleColor[0], marbleColor[0], marbleColor[0], 0);
    createMarbleColor(marbleColor[0], marbleColor[0], marbleColor[0], 4);
    marbleColor[0] = 255 - colors[6];
    createMarbleColor(marbleColor[0], marbleColor[0], marbleColor[0], 1);
    createMarbleColor(marbleColor[0], marbleColor[0], marbleColor[0], 5);
    createMarbleColor(Random(0, 255), Random(0, 255), Random(0, 255), 2);
    createMarbleColor(Random(0, 255), Random(0, 255), Random(0, 255), 6);
#endif

    boardUpdateTimer += deltaTime;
    if (redrawBoard && boardUpdateTimer > .032f) {
        createTextureExt(boardImage, 12, SQUARES, SQUARES, SQUARES * SQUARES * 4, VK_FORMAT_R8G8B8A8_SRGB);
    }

    /* update multiply scale */
    if (multiplySprite->scale[0] < .45f) {
        multiplySprite->scale[0] += deltaTime / 500.f;
        multiplySprite->position[0] = (multiplySprite->scale[0] / 2.f) + .6f;
    }
}

void deinitBoard(void) {
    free(boardImage);
    free(marbleColor);
    free(colors);
}
