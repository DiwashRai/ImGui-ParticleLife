#ifndef PARTICLE_OBJECT_H
#define PARTICLE_OBJECT_H

#include "imgui.h"

struct ParticleObject
{
    // Posititon
    float x;
    float y;

    // Velocity
    float vx;
    float vy;

    // Color
    ImU32 color;
};

#endif // PARTICLE_OBJECT_H
