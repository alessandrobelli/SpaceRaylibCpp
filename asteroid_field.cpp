// asteroid_field.cpp
#include "asteroid_field.h" // Include the header file (defines constants now)

#include "raymath.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

//------------------------------------------------------------------------------------
// Helper Function for Random Float (Definition)
//------------------------------------------------------------------------------------
float GetRandomFloat(float min, float max)
{
    if (max <= min)
        return min;
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

//------------------------------------------------------------------------------------
// Procedural Asteroid Mesh Generation Function (Static - internal use)
//------------------------------------------------------------------------------------
static Mesh GenerateAsteroidMesh(float baseRadius, float irregularity)
{
    // Use GenMeshSphere with low segment count instead of Icosphere
    int rings = (rand() % 9) + 4;  // random number between 4 and 12
    int slices = (rand() % 9) + 4; // Adjust detail level
    Mesh mesh = GenMeshSphere(baseRadius, rings, slices);

    if (mesh.vertices == nullptr)
    {
        TraceLog(LOG_WARNING, "Failed to generate base mesh for asteroid");
        return mesh; // Return empty mesh if generation failed
    }

    int vertexCount = mesh.vertexCount;
    float *vertices = mesh.vertices; // Work directly with the float* vertex buffer

    for (int i = 0; i < vertexCount; ++i) // 'int i' is fine here as vertexCount is int
    {
        // Read current vertex position components
        float vx = vertices[i * 3 + 0];
        float vy = vertices[i * 3 + 1];
        float vz = vertices[i * 3 + 2];
        Vector3 vertexPos = {vx, vy, vz};

        // Calculate random offset
        float offsetMagnitude = baseRadius * irregularity * GetRandomFloat(0.5f, 1.0f);
        Vector3 offsetDir = {0};
        if (Vector3LengthSqr(vertexPos) > 0.0001f)
        {
            offsetDir = Vector3Normalize(vertexPos);
        }
        else
        {
            offsetDir = {1.0f, 0.0f, 0.0f}; // Default direction
        }
        // Slightly bias towards pushing out more than in for chunkier rocks
        Vector3 randomOffset = Vector3Scale(offsetDir, offsetMagnitude * GetRandomFloat(-0.5f, 1.0f));
        Vector3 newPos = Vector3Add(vertexPos, randomOffset);

        // Write new position components back to the float buffer
        vertices[i * 3 + 0] = newPos.x;
        vertices[i * 3 + 1] = newPos.y;
        vertices[i * 3 + 2] = newPos.z;
    }

    return mesh;
}

//------------------------------------------------------------------------------------
// Function Definition for Initializing Asteroids
//------------------------------------------------------------------------------------
std::vector<Asteroid> InitializeAsteroidField(Material defaultMaterial)
{
    // Constants are now defined in asteroid_field.h via AsteroidFieldConstants namespace
    using namespace AsteroidFieldConstants;

    std::vector<Asteroid> asteroids;
    asteroids.reserve(NUM_ASTEROIDS);
    std::vector<Vector3> clusterCenters(NUM_CLUSTERS);

    // Generate cluster centers
    for (int i = 0; i < NUM_CLUSTERS; ++i)
    {
        clusterCenters[i].x = GetRandomFloat(-CLUSTER_SPREAD_RADIUS, CLUSTER_SPREAD_RADIUS);
        clusterCenters[i].y = GetRandomFloat(-CLUSTER_SPREAD_RADIUS, CLUSTER_SPREAD_RADIUS);
        clusterCenters[i].z = GetRandomFloat(-CLUSTER_SPREAD_RADIUS, CLUSTER_SPREAD_RADIUS);
    }

    // Generate asteroids
    for (int i = 0; i < NUM_ASTEROIDS; ++i)
    {
        Asteroid currentAsteroid = {0};

        int clusterIndex = rand() % NUM_CLUSTERS;
        Vector3 clusterCenter = clusterCenters[clusterIndex];
        currentAsteroid.position.x = clusterCenter.x + GetRandomFloat(-ASTEROID_SCATTER_RADIUS, ASTEROID_SCATTER_RADIUS);
        currentAsteroid.position.y = clusterCenter.y + GetRandomFloat(-ASTEROID_SCATTER_RADIUS, ASTEROID_SCATTER_RADIUS);
        currentAsteroid.position.z = clusterCenter.z + GetRandomFloat(-ASTEROID_SCATTER_RADIUS, ASTEROID_SCATTER_RADIUS);

        float sizeMultiplier = 1.0f;
        if (GetRandomFloat(0.0f, 1.0f) < LARGE_ASTEROID_CHANCE)
        {
            sizeMultiplier = GetRandomFloat(1.8f, 3.0f);
        }

        float currentRadius = BASE_MESH_RADIUS * sizeMultiplier;
        float currentIrregularity = MESH_IRREGULARITY * GetRandomFloat(0.8f, 1.2f);
        currentAsteroid.mesh = GenerateAsteroidMesh(currentRadius, currentIrregularity);

        if (currentAsteroid.mesh.vertices == nullptr)
        {
            TraceLog(LOG_WARNING, "Skipping asteroid %d due to mesh generation failure.", i);
            continue;
        }

        currentAsteroid.material = defaultMaterial;

        unsigned char grayValue = (unsigned char)GetRandomFloat(50.0f, 200.0f);
        currentAsteroid.color = {grayValue, grayValue, grayValue, 255};
        currentAsteroid.currentColor = currentAsteroid.color;

        currentAsteroid.rotationAngle = GetRandomFloat(0.0f, 360.0f);
        currentAsteroid.rotationSpeed = GetRandomFloat(MIN_ROTATION_SPEED, MAX_ROTATION_SPEED) * (rand() % 2 == 0 ? 1.0f : -1.0f);
        do
        {
            currentAsteroid.rotationAxis = Vector3Normalize({GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f)});
        } while (Vector3LengthSqr(currentAsteroid.rotationAxis) < 0.01f);

        BoundingBox bounds = GetMeshBoundingBox(currentAsteroid.mesh);
        Vector3 boundsSize = Vector3Subtract(bounds.max, bounds.min);
        float maxDim = fmaxf(fmaxf(boundsSize.x, boundsSize.y), boundsSize.z);
        currentAsteroid.collisionRadius = maxDim * 0.5f;

        currentAsteroid.isActive = true;
        currentAsteroid.hitPoints = INITIAL_HIT_POINTS;
        currentAsteroid.isShaking = false;
        currentAsteroid.shakeTimer = 0.0f;
        currentAsteroid.shakeIntensity = SHAKE_MAGNITUDE_BASE * sizeMultiplier;

        asteroids.push_back(currentAsteroid);
    }

    TraceLog(LOG_INFO, "Generated %d asteroids.", (int)asteroids.size());

    return asteroids;
}