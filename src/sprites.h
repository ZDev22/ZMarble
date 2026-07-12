#ifndef SPRITES_H
#define SPRITES_H

#include "zengine.h"

extern unsigned int availibleSpriteSize;

Sprite* createspriteptr(float positionx, float positiony, float scalex, float scaley, float rotation, unsigned int textureIndex);
void deletesprite(Sprite* sprite);

#ifdef SPRITES_IMPLEMENTATION

unsigned int availibleSprites[ZENGINE_MAX_SPRITES];
unsigned int availibleSpriteSize = 0;

Sprite* createspriteptr(float positionx, float positiony, float scalex, float scaley, float rotation, unsigned int textureIndex) {
    if (availibleSpriteSize == 0) {
        return createSpritePtr(positionx, positiony, scalex, scaley, rotation, textureIndex);
    }
    else {
        unsigned int index = availibleSprites[availibleSpriteSize - 1];
        sprites[index].position[0] = positionx;
        sprites[index].position[1] = positiony;
        sprites[index].scale[0] = scalex;
        sprites[index].scale[1] = scaley;
        sprites[index].rotation = rotation;
        sprites[index].textureIndex = textureIndex;

        availibleSpriteSize--;
        return &sprites[index];
    }
}

void deletesprite(Sprite* sprite) {
    unsigned int index = sprite - sprites;
    availibleSprites[availibleSpriteSize] = index;
    availibleSpriteSize++;
    sprite->scale[0] = 0.f;
    sprite->scale[1] = 0.f;
}

#endif // SPRITES_IMPLEMENTATION
#endif // SPRITES_H
