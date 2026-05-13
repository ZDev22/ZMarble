#define _POSIX_C_SOURCE 199309L

#define BOARD_SCALE .5f
#define SQUARES 128
#define GRAVITY 2.5f

#include "zmarble.h"
#include "../zengine.h"
#include "../zcollide.h"
#include "../ztext.h"
#include "../deps/stb_image_write.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define MAX_OBSTICLES 20
#define MARBLES 12 + MAX_OBSTICLES // also remember the 4 other sprites, multiply, release, and the two walls

typedef struct Marble {
    float velocity[2];
    float radius;
    float weight;
    _Bool colliding;
    _Bool stale;
} Marble;

typedef struct Projectile {
    float velocity[2];
    unsigned int health;
} Projectile;

typedef struct Points {
    unsigned int points;
    unsigned int releasingPoints;
    _Bool update;
    _Bool releasing;
} Points;

static unsigned char* board;
static unsigned char* boardImage;

static Marble marbles[MARBLES];
static unsigned char marbleSize = 0;
static unsigned int obsticleCount;
static Projectile projectiles[0xFFFF];
static Points points[4];
unsigned int projectileSize = 0;
static float projectileTimer[4];
static float projectileTime = 0.f;

static unsigned long long randomstate;
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
    while (a >= b) {
        a -= b;
    }

    return a;
}


static void createProjectile(const unsigned char corner, unsigned int health) {
    projectiles[projectileSize].velocity[0] = -1.05f * fabs((modFloat(projectileTime, 2.f)) - 1.f) + 1.025f;
    projectiles[projectileSize].velocity[1] = -1.05f * fabs((modFloat((projectileTime + 1.f), 2.f)) - 1.f) + 1.025f;
    switch (corner) {
    case 0: // red
        createSprite(squareModel, 0, -BOARD_SCALE + .01f, -BOARD_SCALE + .01f, .01f, .01f, 0.f);
        break;
    case 1: // green
        projectiles[projectileSize].velocity[0] = -projectiles[projectileSize].velocity[0];
        createSprite(squareModel, 1, BOARD_SCALE - .01f, -BOARD_SCALE + .01f, .01f, .01f, 0.f);
        break;
    case 2: // blue
        projectiles[projectileSize].velocity[1] = -projectiles[projectileSize].velocity[1];
        createSprite(squareModel, 2, -BOARD_SCALE + .01f, BOARD_SCALE - .01f, .01f, .01f, 0.f);
        break;
    case 3: // yellow
        projectiles[projectileSize].velocity[0] = -projectiles[projectileSize].velocity[0];
        projectiles[projectileSize].velocity[1] = -projectiles[projectileSize].velocity[1];
        createSprite(squareModel, 3, BOARD_SCALE - .01f, BOARD_SCALE - .01f, .01f, .01f, 0.f);
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

static void createMarble(float velocityx, float velocityy, float radius, float weight, _Bool stale) {
    marbles[marbleSize].velocity[0] = velocityx;
    marbles[marbleSize].velocity[1] = velocityy;
    marbles[marbleSize].radius = radius = radius;
    marbles[marbleSize].weight = weight;
    marbles[marbleSize].colliding = 0;
    marbles[marbleSize].stale = stale;
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

    stbi_write_png("assets/images/board.png", SQUARES, SQUARES, 3, boardImage, SQUARES * 3);

    /* load marbles & projectiles */
    Texture texture;
    createTexture(&texture, "red.png");
    updateTexture(0, &texture);
    createTexture(&texture, "green.png");
    updateTexture(1, &texture);
    createTexture(&texture, "blue.png");
    updateTexture(2, &texture);
    createTexture(&texture, "yellow.png");
    updateTexture(3, &texture);
    createTexture(&texture, "board.png");
    updateTexture(4, &texture);
    createTexture(&texture, "gray.png");
    updateTexture(9, &texture);
    createTexture(&texture, "multiply.png");
    updateTexture(10, &texture);
    createTexture(&texture, "release.png");
    updateTexture(11, &texture);

    sRandom();
 
    for (unsigned int i = 0; i < MARBLES - MAX_OBSTICLES; i++) { /* spawn normal marbles */
        createSprite(squareModel, i % 4, (float)Random(800, 1200) / 1000.f, (float)Random(-2000, -1100) / 1000.f, .025f, .025f, 0.f);
        createMarble((float)Random(-500, 500) / 1000.f, (float)Random(-1000, 1000) / 1000.f, .0125f, .0125f * 40.f, 0);
    }

    /* load obsticles */
    createSprite(squareModel, 9, .61f, -.8f, .2f, .2f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, 1.21f, -.8f, .2f, .2f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, .91f, -.6f, .3f, .3f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, 1.13f, -.4f, .07f, .07f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, 1.13f, .4f, .07f, .07f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, .7f, -.2f, .1f, .1f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, 1.21f, -.05f, .16f, .16f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, .85f, 0.f, .05f, .05f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, .75f, .15f, .05f, .05f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, .9f, .2f, .05f, .05f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, 1.f, .6f, .08f, .08f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    createSprite(squareModel, 9, .61f, .52f, .09f, .09f, 0.f);
    createMarble(0.f, 0.f, *sprites[spritesSize - 1].scale / 2.f, *sprites[spritesSize - 1].scale * 20.f, 1);
    obsticleCount = 12;

    /* load points */
    for (unsigned char i = 0; i < 4; i++) {
        points[i].points = 1;
        points[i].releasingPoints = 0;
        points[i].update = 0;
        points[i].releasing = 0;
        projectileTimer[i] = 0.f;
    }

    loadFont("assets/fonts/CubicCoreMono.ttf");
    Texture* font = createText("1", 0, 128);
    updateTexture(5, font);
    updateTexture(6, font);
    updateTexture(7, font);
    updateTexture(8, font);

    createSprite(squareModel, 4, 0.f, 0.f, BOARD_SCALE * 2.f, BOARD_SCALE * 2.f, 0.f); /* board */

    createSprite(squareModel, 5, -BOARD_SCALE + .05f, -BOARD_SCALE - .05f, .2f, .1f, 0.f);
    createSprite(squareModel, 6, BOARD_SCALE + .05f, -BOARD_SCALE - .05f, .2f, .1f, 0.f);
    createSprite(squareModel, 7, -BOARD_SCALE + .05f, BOARD_SCALE + .1f, .2f, .1f, 0.f);
    createSprite(squareModel, 8, BOARD_SCALE + .05f, BOARD_SCALE + .1f, .2f, .1f, 0.f);

    createSprite(squareModel, 11, 1.f, 1.f, .42f, .1f, 0.f); // release
    createSprite(squareModel, 10, .725f, 1.f, .25f, .1f, 0.f); // multiply
    createSprite(squareModel, 9, .6f, 0.f, .025f, 3.f, 0.f); // wall1
    createSprite(squareModel, 9, 1.218f, 0.f, .025f, 3.f, 0.f); // wall2
}

void updateBoard(void) {
    /* update marbles */
    for (unsigned int i = 0; i < MARBLES - obsticleCount; i++) {
        if (marbles[i].stale) { continue; }
        marbles[i].velocity[1] += GRAVITY * deltaTime;
        sprites[i].position[0] += marbles[i].velocity[0] * deltaTime;
        sprites[i].position[1] += marbles[i].velocity[1] * deltaTime;
        marbles[i].colliding = 0;

        for (unsigned int j = i + 1; j < MARBLES - (MAX_OBSTICLES - obsticleCount); j++) {
            if (zcollide_circleCollision(&sprites[i], marbles[i].radius, &sprites[j], marbles[j].radius)) {
                float normalx = sprites[j].position[0] - sprites[i].position[0];
                float normaly = sprites[j].position[1] - sprites[i].position[1];
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

                sprites[i].position[0] -= overlapx;
                sprites[i].position[1] -= overlapy;
                if (!marbles[j].stale) {
                    sprites[j].position[0] += overlapx;
                    sprites[j].position[1] += overlapy;
                }
            }
        }


        if (sprites[i].position[1] > .9f) {
            if (zcollide_squareCollision(&sprites[i], &sprites[MARBLES - (MAX_OBSTICLES - obsticleCount) + 6])) {
                points[sprites[i].textureIndex].points *= 2;
                points[sprites[i].textureIndex].update = 1;

                sprites[i].position[1] = -1.125f;
                marbles[i].velocity[0] = (float)Random(-500, 500) / 1000.f;
                marbles[i].velocity[1] = 0.f;
            }
            else if (zcollide_squareCollision(&sprites[i], &sprites[MARBLES - (MAX_OBSTICLES - obsticleCount) + 5])) {
                points[sprites[i].textureIndex].releasingPoints += points[sprites[i].textureIndex].points;
                points[sprites[i].textureIndex].points = 1;
                points[sprites[i].textureIndex].releasing = 1;
                points[sprites[i].textureIndex].update = 1;

                sprites[i].position[1] = -1.5f;
                marbles[i].velocity[0] = (float)Random(-1000, 1000) / 1000.f;
                marbles[i].velocity[1] = 0.f;
            } 
        }
        if (sprites[i].position[0] > 1.2f || sprites[i].position[0] < .62f) {
            marbles[i].velocity[0] = -marbles[i].velocity[0];
            sprites[i].position[0] += marbles[i].velocity[0] * deltaTime;
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
            updateTexture(i + 5, createText(pointstr, 0, 128));
            points[i].update = 0;
        }
    }

    unsigned int j = 0;
    _Bool redrawBoard = 0;

    for (unsigned int i = MARBLES - (MAX_OBSTICLES - obsticleCount) + 9; i < spritesSize;) {
        sprites[i].position[0] += projectiles[j].velocity[0] * deltaTime;
        sprites[i].position[1] += projectiles[j].velocity[1] * deltaTime;

        if (sprites[i].position[0] < -BOARD_SCALE || sprites[i].position[0] > BOARD_SCALE) {
            projectiles[j].velocity[0] = -projectiles[j].velocity[0];
            sprites[i].position[0] += projectiles[j].velocity[0] * deltaTime;
        }
        if (sprites[i].position[1] < -BOARD_SCALE || sprites[i].position[1] > BOARD_SCALE) {
            projectiles[j].velocity[1] = -projectiles[j].velocity[1];
            sprites[i].position[1] += projectiles[j].velocity[1] * deltaTime;
        }

        unsigned int squarex = (unsigned int)((sprites[i].position[0] + BOARD_SCALE) * (float)SQUARES);
        unsigned int squarey = (unsigned int)((sprites[i].position[1] + BOARD_SCALE) * (float)SQUARES);
        unsigned int pos = squarey * SQUARES + squarex;
        if (board[pos] != sprites[i].textureIndex) {
            if (pos == 0 || pos == SQUARES - 1 || pos == (SQUARES * SQUARES) - SQUARES || pos == SQUARES * SQUARES - 1) {
                points[board[pos]].releasing = 0;
                points[board[pos]].update = 1;
                for (unsigned char i = board[pos]; i < 12; i+= 4) {
                    sprites[i].position[0] = 10.f;
                    marbles[i].stale = 1;
                }
            }

            board[pos] = sprites[i].textureIndex;
            setSquare(pos);
            redrawBoard = 1;
            projectiles[j].health--;

            if (projectiles[j].health == 0) {
                deleteSprite(i);
                if (j < projectileSize - 1) {
                    memmove(&projectiles[j], &projectiles[j + 1], (projectileSize - j - 1) * sizeof(Projectile));
                }
                projectileSize--;
                continue;
            }
        }

        i++;
        j++;
    }

    if (redrawBoard) {
        stbi_write_png("assets/images/board.png", SQUARES, SQUARES, 3, boardImage, SQUARES * 3);

        Texture texture;
        createTexture(&texture, "board.png");
        updateTexture(4, &texture);
    }

    /* update misc stuffs */
    if (sprites[MARBLES - (MAX_OBSTICLES - obsticleCount) + 6].scale[0] < .45f) {
        sprites[MARBLES - (MAX_OBSTICLES - obsticleCount) + 6].scale[0] += deltaTime / 1250.f;
        sprites[MARBLES - (MAX_OBSTICLES - obsticleCount) + 6].position[0] = (sprites[MARBLES - (MAX_OBSTICLES - obsticleCount) + 6].scale[0] / 2.f) + .6f;
    }
}

