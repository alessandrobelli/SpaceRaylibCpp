#include "background.h"
#include "asteroid_field.h" // For GetRandomFloat - Alternatively move GetRandomFloat to its own utils header/cpp
#include <vector>
#include <cstdlib> // For rand()

//------------------------------------------------------------------------------------
// Background Functions - Implementation
//------------------------------------------------------------------------------------

std::vector<Star> InitializeStars(int screenWidth, int screenHeight, int starCount)
{
    std::vector<Star> stars(starCount);
    for (int i = 0; i < starCount; ++i) {
        stars[i].position.x = (float)(rand() % screenWidth);
        stars[i].position.y = (float)(rand() % screenHeight);
        // Use basic rand() or GetRandomFloat if accessible
        stars[i].radius = 0.5f + (float)(rand() % 100) / 100.0f; // Radius between 0.5 and 1.5
        // stars[i].radius = GetRandomFloat(0.5f, 1.5f); // If GetRandomFloat is available
        unsigned char brightness = (unsigned char)((rand() % 106) + 150); // Simple rand for brightness [150-255]
        stars[i].color = { brightness, brightness, brightness, 255 };
    }
    return stars;
}

void DrawStars(const std::vector<Star>& stars)
{
    for (const auto& star : stars) {
        DrawCircleV(star.position, star.radius, star.color);
    }
}
