#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "raylib.h"
#include <vector>

//------------------------------------------------------------------------------------
// Structure Definition for Particles
//------------------------------------------------------------------------------------
typedef struct {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float lifeTime;
    bool isActive;
} Particle;

//------------------------------------------------------------------------------------
// Particle System Functions - Declaration
//------------------------------------------------------------------------------------

// Initializes the particle pool
void InitializeParticles();

// Updates positions and lifetimes of active particles
void UpdateParticles(float deltaTime);

// Draws active particles
void DrawParticles();

// Emits a burst of particles from a position
void EmitParticles(Vector3 position, int count, float speed, float duration, Color color);


#endif // PARTICLE_SYSTEM_H
