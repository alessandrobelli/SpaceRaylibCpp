#ifndef UNIFORM_GRID_H
#define UNIFORM_GRID_H

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <set>

// Include Asteroid definition needed for BuildInstanced parameter
#include "asteroid_field.h"

// Helper struct for integer grid coordinates
typedef struct Vector3Int
{
    int x;
    int y;
    int z;
} Vector3Int;

class UniformGrid
{
public:
    UniformGrid(Vector3 worldMin, Vector3 worldMax, Vector3 cellSize);
    void Clear();
    // Add still takes index and world bounds
    void Add(int instanceIndex, BoundingBox worldBounds);

    // Build method for original Asteroid struct (can be kept or removed)
    // void Build(const std::vector<Asteroid>& asteroids);

    // --- Added Build Method for Instances ---
    void BuildInstanced(const std::vector<Asteroid> &instances);
    // --- End Added Build Method ---

    std::vector<int> Query(Vector3 worldPos);
    std::vector<int> QueryRay(Ray ray, float maxDistance);

    Vector3Int GetCellIndices(Vector3 worldPos) const;
    int Get1DIndex(int ix, int iy, int iz) const;
    bool IsValidIndex(int ix, int iy, int iz) const;

    Vector3 GetMinBounds() const { return gridMinBounds; }
    Vector3 GetMaxBounds() const { return gridMaxBounds; }
    Vector3 GetCellSize() const { return gridCellSize; }
    Vector3Int GetDimensions() const { return Vector3Int{gridDimX, gridDimY, gridDimZ}; }

private:
    Vector3 gridMinBounds;
    Vector3 gridMaxBounds;
    Vector3 gridCellSize;
    int gridDimX, gridDimY, gridDimZ;
    int totalCells;
    std::vector<std::vector<int>> gridCells;
};

#endif // UNIFORM_GRID_H
