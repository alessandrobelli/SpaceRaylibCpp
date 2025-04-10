#include "uniform_grid.h"
#include <cmath> // For floorf, ceilf
#include <vector>
#include <set>       // For easily getting unique indices in Query
#include <algorithm> // For std::max, std::min
#include <limits>    // Required for QueryRay

//------------------------------------------------------------------------------------
// UniformGrid Class - Implementation
//------------------------------------------------------------------------------------

// Constructor
UniformGrid::UniformGrid(Vector3 worldMin, Vector3 worldMax, Vector3 cellSize)
    : gridMinBounds(worldMin), gridMaxBounds(worldMax), gridCellSize(cellSize)
{
    // Ensure cell size is positive
    if (gridCellSize.x <= 0.0f)
        gridCellSize.x = 1.0f;
    if (gridCellSize.y <= 0.0f)
        gridCellSize.y = 1.0f;
    if (gridCellSize.z <= 0.0f)
        gridCellSize.z = 1.0f;

    // Calculate dimensions
    Vector3 gridTotalSize = Vector3Subtract(gridMaxBounds, gridMinBounds);
    gridDimX = (int)ceilf(gridTotalSize.x / gridCellSize.x);
    gridDimY = (int)ceilf(gridTotalSize.y / gridCellSize.y);
    gridDimZ = (int)ceilf(gridTotalSize.z / gridCellSize.z);

    // Ensure minimum dimensions
    if (gridDimX <= 0)
        gridDimX = 1;
    if (gridDimY <= 0)
        gridDimY = 1;
    if (gridDimZ <= 0)
        gridDimZ = 1;

    totalCells = gridDimX * gridDimY * gridDimZ;

    // Resize the main grid vector
    gridCells.resize(totalCells);

    TraceLog(LOG_INFO, "UniformGrid initialized: Dims(%d, %d, %d), Cells=%d", gridDimX, gridDimY, gridDimZ, totalCells);
}

// Clear grid
void UniformGrid::Clear()
{
    for (int i = 0; i < totalCells; ++i)
    {
        gridCells[i].clear(); // Clear the list of indices for each cell
    }
}

// Get integer cell indices from world position
Vector3Int UniformGrid::GetCellIndices(Vector3 worldPos) const
{
    Vector3Int indices = {0};
    Vector3 relativePos = Vector3Subtract(worldPos, gridMinBounds);

    indices.x = (int)floorf(relativePos.x / gridCellSize.x);
    indices.y = (int)floorf(relativePos.y / gridCellSize.y);
    indices.z = (int)floorf(relativePos.z / gridCellSize.z);

    // Clamp indices to be within valid grid range
    indices.x = std::max(0, std::min(indices.x, gridDimX - 1));
    indices.y = std::max(0, std::min(indices.y, gridDimY - 1));
    indices.z = std::max(0, std::min(indices.z, gridDimZ - 1));

    return indices;
}

// Check if 3D indices are valid
bool UniformGrid::IsValidIndex(int ix, int iy, int iz) const
{
    return (ix >= 0 && ix < gridDimX &&
            iy >= 0 && iy < gridDimY &&
            iz >= 0 && iz < gridDimZ);
}

// Convert 3D cell indices to 1D vector index
int UniformGrid::Get1DIndex(int ix, int iy, int iz) const
{
    // Assumes indices are already validated or clamped
    // Formula: x + y*width + z*width*height
    return ix + iy * gridDimX + iz * gridDimX * gridDimY;
}

// Add an asteroid instance index to all cells its bounds overlap
void UniformGrid::Add(int instanceIndex, BoundingBox worldBounds)
{
    Vector3Int minIndices = GetCellIndices(worldBounds.min);
    Vector3Int maxIndices = GetCellIndices(worldBounds.max);

    for (int iz = minIndices.z; iz <= maxIndices.z; ++iz)
    {
        for (int iy = minIndices.y; iy <= maxIndices.y; ++iy)
        {
            for (int ix = minIndices.x; ix <= maxIndices.x; ++ix)
            {
                if (IsValidIndex(ix, iy, iz))
                {
                    int cellIndex = Get1DIndex(ix, iy, iz);
                    // Cast gridCells.size() to int for comparison
                    if (cellIndex >= 0 && cellIndex < (int)gridCells.size())
                    { // Safety check
                        gridCells[cellIndex].push_back(instanceIndex);
                    }
                    else
                    {
                        TraceLog(LOG_WARNING, "GRID ADD: Calculated invalid cell index %d for coords (%d, %d, %d)", cellIndex, ix, iy, iz);
                    }
                }
            }
        }
    }
}

// --- Added BuildInstanced Method Definition ---
void UniformGrid::BuildInstanced(const std::vector<Asteroid> &instances)
{
    Clear(); // Clear any previous data
    TraceLog(LOG_INFO, "Building Uniform Grid with %zu asteroid instances...", instances.size());

    for (size_t i = 0; i < instances.size(); ++i)
    {
        if (!instances[i].isActive)
            continue; // Only add active asteroids

        // Approximate world bounds using position and collision radius
        // This avoids needing access to baseMeshes here and complex scaled bounds calculation
        float r = instances[i].collisionRadius;
        if (r <= 0.0f)
            r = 0.5f; // Use a minimum radius if calculated as zero
        BoundingBox worldBounds = {
            (Vector3){instances[i].position.x - r, instances[i].position.y - r, instances[i].position.z - r},
            (Vector3){instances[i].position.x + r, instances[i].position.y + r, instances[i].position.z + r}};

        // Add the instance's index (i) to the grid cells it overlaps
        Add((int)i, worldBounds);
    }
    TraceLog(LOG_INFO, "Uniform Grid build complete.");
}
// --- End Added BuildInstanced Method ---

// Query for potential colliders near a world position
std::vector<int> UniformGrid::Query(Vector3 worldPos)
{
    std::set<int> uniqueIndices;
    Vector3Int centerIndices = GetCellIndices(worldPos);
    for (int dz = -1; dz <= 1; ++dz)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                int checkX = centerIndices.x + dx;
                int checkY = centerIndices.y + dy;
                int checkZ = centerIndices.z + dz;
                if (IsValidIndex(checkX, checkY, checkZ))
                {
                    int cellIndex = Get1DIndex(checkX, checkY, checkZ);
                    // Cast gridCells.size() to int for comparison
                    if (cellIndex >= 0 && cellIndex < (int)gridCells.size())
                    { // Safety check
                        uniqueIndices.insert(gridCells[cellIndex].begin(), gridCells[cellIndex].end());
                    }
                    else
                    {
                        TraceLog(LOG_WARNING, "GRID QUERY: Calculated invalid cell index %d for coords (%d, %d, %d)", cellIndex, checkX, checkY, checkZ);
                    }
                }
            }
        }
    }
    std::vector<int> resultIndices(uniqueIndices.begin(), uniqueIndices.end());
    return resultIndices;
}

// QueryRay Implementation
std::vector<int> UniformGrid::QueryRay(Ray ray, float maxDistance)
{
    std::set<int> uniqueIndices;

    if (Vector3LengthSqr(ray.direction) < 0.0001f)
    {
        return Query(ray.position);
    }

    Vector3Int currentIndices = GetCellIndices(ray.position);
    int ix = currentIndices.x;
    int iy = currentIndices.y;
    int iz = currentIndices.z;
    int stepX = (ray.direction.x >= 0) ? 1 : -1;
    int stepY = (ray.direction.y >= 0) ? 1 : -1;
    int stepZ = (ray.direction.z >= 0) ? 1 : -1;
    float tMaxX = std::numeric_limits<float>::infinity();
    float tMaxY = std::numeric_limits<float>::infinity();
    float tMaxZ = std::numeric_limits<float>::infinity();
    float tDeltaX = std::numeric_limits<float>::infinity();
    float tDeltaY = std::numeric_limits<float>::infinity();
    float tDeltaZ = std::numeric_limits<float>::infinity();

    if (fabsf(ray.direction.x) > 0.0001f)
    {
        float nextBoundXCoord = (stepX > 0) ? (float)(ix + 1) * gridCellSize.x + gridMinBounds.x : (float)(ix)*gridCellSize.x + gridMinBounds.x;
        tMaxX = (nextBoundXCoord - ray.position.x) / ray.direction.x;
        tDeltaX = fabsf(gridCellSize.x / ray.direction.x);
    }
    if (fabsf(ray.direction.y) > 0.0001f)
    {
        float nextBoundYCoord = (stepY > 0) ? (float)(iy + 1) * gridCellSize.y + gridMinBounds.y : (float)(iy)*gridCellSize.y + gridMinBounds.y;
        tMaxY = (nextBoundYCoord - ray.position.y) / ray.direction.y;
        tDeltaY = fabsf(gridCellSize.y / ray.direction.y);
    }
    if (fabsf(ray.direction.z) > 0.0001f)
    {
        float nextBoundZCoord = (stepZ > 0) ? (float)(iz + 1) * gridCellSize.z + gridMinBounds.z : (float)(iz)*gridCellSize.z + gridMinBounds.z;
        tMaxZ = (nextBoundZCoord - ray.position.z) / ray.direction.z;
        tDeltaZ = fabsf(gridCellSize.z / ray.direction.z);
    }

    float currentT = 0.0f;

    if (IsValidIndex(ix, iy, iz))
    {
        int cellIndex = Get1DIndex(ix, iy, iz);
        // Cast gridCells.size() to int for comparison
        if (cellIndex >= 0 && cellIndex < (int)gridCells.size())
        {
            uniqueIndices.insert(gridCells[cellIndex].begin(), gridCells[cellIndex].end());
        }
    }

    while (currentT < maxDistance)
    {
        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                currentT = tMaxX;
                tMaxX += tDeltaX;
                ix += stepX;
            }
            else
            {
                currentT = tMaxZ;
                tMaxZ += tDeltaZ;
                iz += stepZ;
            }
        }
        else
        {
            if (tMaxY < tMaxZ)
            {
                currentT = tMaxY;
                tMaxY += tDeltaY;
                iy += stepY;
            }
            else
            {
                currentT = tMaxZ;
                tMaxZ += tDeltaZ;
                iz += stepZ;
            }
        }

        if (currentT >= maxDistance)
            break;
        if (!IsValidIndex(ix, iy, iz))
            break;

        int cellIndex = Get1DIndex(ix, iy, iz);
        // Cast gridCells.size() to int for comparison
        if (cellIndex >= 0 && cellIndex < (int)gridCells.size())
        {
            uniqueIndices.insert(gridCells[cellIndex].begin(), gridCells[cellIndex].end());
        }
        else
        {
            TraceLog(LOG_WARNING, "GRID RAY QUERY: Calculated invalid cell index %d for coords (%d, %d, %d)", cellIndex, ix, iy, iz);
        }
    }
    std::vector<int> resultIndices(uniqueIndices.begin(), uniqueIndices.end());
    return resultIndices;
}
