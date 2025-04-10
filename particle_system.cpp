#include "particle_system.h"
#include "asteroid_field.h" // For GetRandomFloat - Alternatively move GetRandomFloat to its own utils header/cpp
#include "raymath.h" // For Vector math
#include <vector>
#include <cstdlib> // For rand()

//------------------------------------------------------------------------------------
// Particle System Module Data (Static - internal to this file)
//------------------------------------------------------------------------------------
const int MAX_PARTICLES = 500;
static std::vector<Particle> particles(MAX_PARTICLES);
static int nextParticleIndex = 0;

//------------------------------------------------------------------------------------
// Particle System Functions - Implementation
//------------------------------------------------------------------------------------

void InitializeParticles()
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        particles[i].isActive = false;
    }
    nextParticleIndex = 0;
}

void UpdateParticles(float deltaTime)
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (particles[i].isActive) {
            particles[i].lifeTime -= deltaTime;
            if (particles[i].lifeTime <= 0.0f) {
                particles[i].isActive = false;
            } else {
                particles[i].position = Vector3Add(particles[i].position, Vector3Scale(particles[i].velocity, deltaTime));
                // Optional: Add fade, gravity etc. here
            }
        }
    }
}

void DrawParticles()
{
     // Assumes BeginMode3D has been called
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (particles[i].isActive) {
            // Draw particles as small spheres
            DrawSphere(particles[i].position, 0.05f, particles[i].color);
            // Or use DrawPoint3D(particles[i].position, particles[i].color);
        }
    }
}

void EmitParticles(Vector3 position, int count, float speed, float duration, Color color)
{
     for (int i = 0; i < count; ++i) {
        int pIndex = nextParticleIndex;
        particles[pIndex].isActive = true;
        particles[pIndex].position = position;
        particles[pIndex].color = color;
        // Use GetRandomFloat if available, otherwise basic rand
        particles[pIndex].lifeTime = duration * (0.5f + (float)(rand() % 100) / 100.0f); // Vary lifetime 0.5x to 1.5x
        // particles[pIndex].lifeTime = duration * GetRandomFloat(0.5f, 1.5f);


        Vector3 velocity = { GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f) };
        if (Vector3LengthSqr(velocity) < 0.001f) velocity = {1.0f, 0.0f, 0.0f}; // Default if random is zero
        // Use GetRandomFloat if available, otherwise basic rand
        float speedVariation = 0.5f + (float)(rand() % 100) / 100.0f; // Vary speed 0.5x to 1.5x
        // float speedVariation = GetRandomFloat(0.5f, 1.5f);
        particles[pIndex].velocity = Vector3Scale(Vector3Normalize(velocity), speed * speedVariation);

        nextParticleIndex++;
        if (nextParticleIndex >= MAX_PARTICLES) nextParticleIndex = 0; // Wrap around pool
    }
}

