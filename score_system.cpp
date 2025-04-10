#include "score_system.h"
#include "raylib.h" // Required for DrawText, TextFormat, Color

//------------------------------------------------------------------------------------
// Score System Module - Implementation
//------------------------------------------------------------------------------------

// Static variable to hold the score (internal to this file)
static int currentScore = 0;

void InitializeScore()
{
    currentScore = 0;
}

void AddScore(int points)
{
    if (points > 0) // Optional: Ensure points are positive
    {
        currentScore += points;
    }
}

int GetScore()
{
    return currentScore;
}

void DrawScoreUI(int posX, int posY, int fontSize, Color color)
{
    // Use TextFormat to create the string "Score: <value>"
    DrawText(TextFormat("Score: %d", currentScore), posX, posY, fontSize, color);
}
