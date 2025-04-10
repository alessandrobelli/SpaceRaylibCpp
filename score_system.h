#include <raylib.h>
#ifndef SCORE_SYSTEM_H
#define SCORE_SYSTEM_H

//------------------------------------------------------------------------------------
// Score System Functions - Declaration
//------------------------------------------------------------------------------------

// Initializes the score to zero
void InitializeScore();

// Adds the given points to the current score
void AddScore(int points);

// Returns the current score
int GetScore();

// Draws the current score onto the screen
void DrawScoreUI(int posX, int posY, int fontSize, Color color);

#endif // SCORE_SYSTEM_H
