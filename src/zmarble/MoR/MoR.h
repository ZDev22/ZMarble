#ifndef MOR_H
#define MOR_H

#include "../../zengine.h"

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

void initMoR(void);
void tickMoR(void);
void deinitMoR(void);

#endif // ZMARBLE_H
