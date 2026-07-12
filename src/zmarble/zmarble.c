#define _POSIX_C_SOURCE 199309L

#define BOARD_SCALE .5f
#define SQUARES 256
#define GRAVITY 2.5f

#include "zmarble.h"
#include "../zengine.h"
#include "../zcollide.h"
#include "../ztext.h"
#include "../sprites.h"
#include "../deps/stb_image_write.h"

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
    unsigned int points;
    unsigned int releasingPoints;
    _Bool update;
    _Bool releasing;

    Sprite* sprite;
} Points;

static unsigned char* board;
static unsigned char* boardImage;

static Marble marbles[MARBLES];
static unsigned char marbleSize = 0;
static Projectile projectiles[8192];
static unsigned short projectileSize = 0;
static Points points[4];
static float projectileTimer[4];
static float projectileTime = 0.f;
static unsigned long long randomstate = 0;

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


static void createProjectile(const unsigned char corner, unsigned int health) {
    if (projectileSize >= 8192) { return; }

    projectiles[projectileSize].velocity[0] = -1.025f * fabs((modFloat(projectileTime, 2.f)) - 1.f) + 1.0125f;
    projectiles[projectileSize].velocity[1] = -1.025f * fabs((modFloat((projectileTime + 1.f), 2.f)) - 1.f) + 1.0125f;
    switch (corner) {
    case 0: // red
        projectiles[projectileSize].sprite = createspriteptr(-BOARD_SCALE + .005f, -BOARD_SCALE + .005f, .01f, .01f, 0.f, 0);
        break;
    case 1: // green
        projectiles[projectileSize].velocity[0] = -projectiles[projectileSize].velocity[0];
        projectiles[projectileSize].sprite = createspriteptr(BOARD_SCALE - .005f, -BOARD_SCALE + .005f, .01f, .01f, 0.f, 1);
        break;
    case 2: // blue
        projectiles[projectileSize].velocity[1] = -projectiles[projectileSize].velocity[1];
        projectiles[projectileSize].sprite = createspriteptr(-BOARD_SCALE + .005f, BOARD_SCALE - .005f, .01f, .01f, 0.f, 2);
        break;
    case 3: // yellow
        projectiles[projectileSize].velocity[0] = -projectiles[projectileSize].velocity[0];
        projectiles[projectileSize].velocity[1] = -projectiles[projectileSize].velocity[1];
        projectiles[projectileSize].sprite = createspriteptr(BOARD_SCALE - .005f, BOARD_SCALE - .005f, .01f, .01f, 0.f, 3);
        break;
    }

    projectiles[projectileSize].health = health;
    projectiles[projectileSize].velocity[0] /= (float)(SQUARES / 90);
    projectiles[projectileSize].velocity[1] /= (float)(SQUARES / 90);
    projectileSize++;

    points[corner].releasingPoints -= health;
    points[corner].update = 1;
    if (points[corner].releasingPoints == 0) {
        points[corner].releasing = 0;
    }
}


static void setSquare(unsigned int index) {
    switch(board[index]) {
    case 0: // red
        boardImage[index * 3] = 255;
        boardImage[index * 3 + 1] = 0;
        boardImage[index * 3 + 2] = 0;
        return;
    case 1: // green
        boardImage[index * 3] = 0;
        boardImage[index * 3 + 1] = 255;
        boardImage[index * 3 + 2] = 0;
        return;
    case 2: // blue
        boardImage[index * 3] = 0;
        boardImage[index * 3 + 1] = 0;
        boardImage[index * 3 + 2] = 255;
        return;
    case 3: // yellow
        boardImage[index * 3] = 255;
        boardImage[index * 3 + 1] = 255;
        boardImage[index * 3 + 2] = 0;
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
    /* load board */
    board = (unsigned char*)malloc(SQUARES * SQUARES);
    boardImage = (unsigned char *)malloc(SQUARES * SQUARES * 3);

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

    stbi_write_png("assets/img/board.png", SQUARES, SQUARES, 3, boardImage, SQUARES * 3);

    /* load marbles & projectiles */
    createTexture("assets/img/red.png", 1.f, 0);
    createTexture("assets/img/green.png", 1.f, 1);
    createTexture("assets/img/blue.png", 1.f, 2);
    createTexture("assets/img/yellow.png", 1.f, 3);
    createTexture("assets/img/board.png", 1.f, 4);
    createTexture("assets/img/gray.png", 1.f, 9);
    createTexture("assets/img/multiply.png", 1.f, 10);
    createTexture("assets/img/release.png", 1.f, 11);

    sRandom();

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
        points[i].points = 1;
        points[i].releasingPoints = 0;
        points[i].update = 0;
        points[i].releasing = 0;
        projectileTimer[i] = 0.f;
    }

    loadFont("assets/fonts/CubicCoreMono.ttf", 0);
    createText("1", 128, 32, 16, 0, 5);
    createText("1", 128, 32, 16, 0, 6);
    createText("1", 128, 32, 16, 0, 7);
    createText("1", 128, 32, 16, 0, 8);

    points[0].sprite = createspriteptr(-BOARD_SCALE + .05f, -BOARD_SCALE - .05f, .4f, .1f, 0.f, 5);
    points[1].sprite = createspriteptr(BOARD_SCALE + .05f, -BOARD_SCALE - .05f, .4f, .1f, 0.f, 6);
    points[2].sprite = createspriteptr(-BOARD_SCALE + .05f, BOARD_SCALE + .1f, .4f, .1f, 0.f, 7);
    points[3].sprite = createspriteptr(BOARD_SCALE + .05f, BOARD_SCALE + .1f, .4f, .1f, 0.f, 8);

    boardSprite = createspriteptr(0.f, 0.f, BOARD_SCALE * 2.f, BOARD_SCALE * 2.f, 0.f, 4);
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
            if (zcollide_pointCircleCollision(marbles[i].sprite->position[0], marbles[i].sprite->position[1], marbles[j].sprite->position[0], marbles[j].sprite->position[1], marbles[j].radius + marbles[i].radius)) {
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
                points[marbles[i].sprite->textureIndex].points *= 2;
                points[marbles[i].sprite->textureIndex].update = 1;

                marbles[i].sprite->position[1] = -1.125f;
                marbles[i].velocity[0] = (float)Random(-500, 500) / 1000.f;
                marbles[i].velocity[1] = 0.f;
            }
            else if (zcollide_squareCollision(marbles[i].sprite->position[0], marbles[i].sprite->position[1], marbles[i].sprite->scale[0], marbles[i].sprite->scale[1], releaseSprite->position[0], releaseSprite->position[1], releaseSprite->scale[0], releaseSprite->scale[1])) {
                points[marbles[i].sprite->textureIndex].releasingPoints += points[marbles[i].sprite->textureIndex].points;
                points[marbles[i].sprite->textureIndex].points = 1;
                points[marbles[i].sprite->textureIndex].releasing = 1;
                points[marbles[i].sprite->textureIndex].update = 1;

                marbles[i].sprite->position[1] = -1.5f;
                marbles[i].velocity[0] = (float)Random(-1000, 1000) / 1000.f;
                marbles[i].velocity[1] = 0.f;
            }
        }
        if (marbles[i].sprite->position[0] > 1.2f || marbles[i].sprite->position[0] < .62f) {
            marbles[i].velocity[0] = -marbles[i].velocity[0];
            marbles[i].sprite->position[0] += marbles[i].velocity[0] * deltaTime;
        }
    }

    /* update projectiles */
    projectileTime += deltaTime / 3.f;
    for (unsigned char i = 0; i < 4; i++) {
        if (points[i].releasing) {
            projectileTimer[i] += deltaTime;
            if (projectileTimer[i] > .008f) {
                createProjectile(i, (points[i].releasingPoints / 512) + 1);
                projectileTimer[i] = 0.f;
            }
        }

        if (points[i].update) {
            char pointstr[20];
            if (points[i].releasing) {
                sprintf(pointstr, "%d", points[i].releasingPoints);
            }
            else {
                sprintf(pointstr, "%d", points[i].points);
            }
            createText(pointstr, 128, 32, 16, 0, i + 5);
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
        if (board[pos] != projectiles[i].sprite->textureIndex) {
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

            board[pos] = projectiles[i].sprite->textureIndex;
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

    boardUpdateTimer += deltaTime;
    if (redrawBoard && boardUpdateTimer > .032f) {
        stbi_write_png("assets/img/board.png", SQUARES, SQUARES, 3, boardImage, SQUARES * 3);
        createTexture("assets/img/board.png", 1.f, 4);
    }

    /* update multiply scale */
    if (multiplySprite->scale[0] < .45f) {
        multiplySprite->scale[0] += deltaTime / 500.f;
        multiplySprite->position[0] = (multiplySprite->scale[0] / 2.f) + .6f;
    }
}
