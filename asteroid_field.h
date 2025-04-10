// asteroid_field.h
#ifndef ASTEROID_FIELD_H
#define ASTEROID_FIELD_H

#include "raylib.h"
#include <vector> // Required for std::vector

//------------------------------------------------------------------------------------
// Constants for Asteroid Field Generation
//------------------------------------------------------------------------------------
namespace AsteroidFieldConstants
{
    constexpr int NUM_ASTEROIDS = 1000;
    constexpr int NUM_CLUSTERS = 10;
    constexpr float CLUSTER_SPREAD_RADIUS = 125.0f;
    constexpr float ASTEROID_SCATTER_RADIUS = 8.0f;
    constexpr float LARGE_ASTEROID_CHANCE = 0.1f;
    constexpr float MIN_ROTATION_SPEED = 5.0f;
    constexpr float MAX_ROTATION_SPEED = 30.0f;
    constexpr int INITIAL_HIT_POINTS = 3;
    constexpr float BASE_MESH_RADIUS = 0.5f;
    constexpr float MESH_IRREGULARITY = 0.7f;
    constexpr float SHAKE_MAGNITUDE_BASE = 0.08f;
} // namespace AsteroidFieldConstants

//------------------------------------------------------------------------------------
// Structure Definition for Asteroids
//------------------------------------------------------------------------------------
typedef struct
{
    Vector3 position;
    Mesh mesh;          // Mesh data
    Material material;  // Material (can share default, but allows customization)
    Color color;        // Original color (used to tint material)
    Color currentColor; // Current color (changes on collision/hit)
    float rotationAngle;
    Vector3 rotationAxis;
    float rotationSpeed;
    float collisionRadius; // Will be calculated from mesh bounds
    bool isActive;

    int hitPoints;
    bool isShaking;
    float shakeTimer;
    float shakeIntensity;

} Asteroid;

//------------------------------------------------------------------------------------
// Function Declaration for Initializing Asteroids
//------------------------------------------------------------------------------------
std::vector<Asteroid> InitializeAsteroidField(Material defaultMaterial);

//------------------------------------------------------------------------------------
// Helper Function Declaration (Needed by main.cpp for shake effect, or other files)
//------------------------------------------------------------------------------------
float GetRandomFloat(float min, float max);

#endif // ASTEROID_FIELD_H