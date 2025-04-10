#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "raylib.h"
#include <vector>

//------------------------------------------------------------------------------------
// Structure Definition for Stars
//------------------------------------------------------------------------------------
typedef struct {
    Vector2 position;
    Color color;
    float radius;
} Star;

//------------------------------------------------------------------------------------
// Background Functions - Declaration
//------------------------------------------------------------------------------------

// Generates a vector of stars with random positions, sizes, and brightness
std::vector<Star> InitializeStars(int screenWidth, int screenHeight, int starCount);

// Draws the stars from the provided vector
void DrawStars(const std::vector<Star>& stars); // Pass by const reference

#endif // BACKGROUND_H
