/*******************************************************************************************
 *
 * Asteroid Field Tech Demo
 *
 * Author: Alessandro Belli
 * Based on raylib [core] example - 3d camera first person
 *
 * This tech demo features:
 * - Procedurally generated asteroid field.
 * - First-person camera controls.
 * - Basic UI (Main Menu, Pause Menu with mouse interaction).
 * - Score system and particle effects for destruction.
 * - Uniform Grid for collision detection optimization.
 * - Simple frustum culling for rendering optimization.
 *
 ********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <cfloat> // Required for FLT_MAX
#include <string> // Required for std::string, TextFormat

#include "custom_camera.h"
#include "asteroid_field.h" // Includes AsteroidFieldConstants
#include "score_system.h"
#include "background.h"
#include "particle_system.h"
#include "uniform_grid.h" // Include the grid header

// Game Screen Enum
typedef enum GameScreen
{
    MAIN_MENU,
    LOADING,
    GAMEPLAY,
    PAUSE_MENU
} GameScreen;

// Program main entry point
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_INFO); // Show INFO log messages
    InitWindow(screenWidth, screenHeight, "Asteroid Field Demo - A. Belli");

    srand(time(NULL)); // Seed random number generator

    // --- Initialize Game Components ---
    CustomCamera customCamera(
        (Vector3){0.0f, 2.0f, 5.0f}, (Vector3){0.0f, 1.8f, 0.0f},   // Initial position and target
        (Vector3){0.0f, 1.0f, 0.0f}, 60.0f, CAMERA_PERSPECTIVE,     // Up vector, FOV (60 degrees), Projection
        (Vector3){0.15f, 0.15f, 0.15f}, (Vector2){0.003f, 0.003f}); // Movement speed, Mouse sensitivity
    Vector3 initialCameraPos = customCamera.GetCamera().position;

    // Background colors
    Color spaceBlueDark = {0, 0, 20, 255};
    Color spaceBlueLight = {0, 20, 50, 255};

    // Initialize background stars
    const int numStars = 700;
    std::vector<Star> stars = InitializeStars(screenWidth, screenHeight, numStars);

    // Load default material for asteroids
    Material defaultMaterial = LoadMaterialDefault();

    // --- Grid Initialization ---
    UniformGrid *collisionGrid = nullptr;         // Pointer for the collision grid (initialized in LOADING)
    Vector3 gridCellSize = {10.0f, 10.0f, 10.0f}; // Size of each cell in the uniform grid

    // Initialize other systems
    InitializeScore();
    InitializeParticles();
    // --- End Initialize Game Components ---

    // --- Game State Variables ---
    GameScreen currentScreen = MAIN_MENU; // Start at the main menu
    int mainMenuSelection = 0;            // Currently selected main menu option index
    const int mainMenuOptions = 2;        // Number of main menu options
    const int pauseMenuOptions = 3;       // Number of pause menu options
    bool shouldExit = false;              // Flag to control game loop exit
    // --- End Game State Variables ---

    // --- Gameplay State Variables ---
    std::vector<Asteroid> asteroids; // Vector to hold all asteroid data
    bool gameInitialized = false;    // Flag: true when asteroids and grid are loaded

    // Player collision bounce state
    bool isBouncing = false;
    float bounceTimer = 0.0f;
    Vector3 bounceDirection = {0.0f, 0.0f, 0.0f};
    const float BOUNCE_DURATION = 0.4f;       // How long the bounce effect lasts
    const float INITIAL_BOUNCE_SPEED = 10.0f; // Initial speed of the bounce-back

    // Asteroid hit visual effect state
    const float SHAKE_DURATION = 0.25f;   // How long the asteroid shakes after being hit
    const float HIT_MAX_DISTANCE = 50.0f; // Maximum distance the player can click-hit an asteroid

    // Debugging flag
    bool showDebug = false; // Toggle with F1 to show collision spheres etc.
    int drawnAsteroids = 0; // Counter for how many asteroids are drawn after culling
    // --- End Gameplay State ---

    SetTargetFPS(60); // Set desired frame rate
    //--------------------------------------------------------------------------------------
    // End Initialization
    //--------------------------------------------------------------------------------------

    // Main game loop - called every frame
    while (!WindowShouldClose() && !shouldExit)
    {
        // Update
        //----------------------------------------------------------------------------------
        float deltaTime = GetFrameTime();      // Time since last frame
        Vector2 mousePos = GetMousePosition(); // Current mouse position

        // State machine for Update logic
        switch (currentScreen)
        {
        case MAIN_MENU:
        {
            EnableCursor(); // Show mouse cursor in menu
            // Keyboard Input for Main Menu Navigation
            if (IsKeyPressed(KEY_DOWN))
                mainMenuSelection = (mainMenuSelection + 1) % mainMenuOptions;
            else if (IsKeyPressed(KEY_UP))
                mainMenuSelection = (mainMenuSelection - 1 + mainMenuOptions) % mainMenuOptions;

            // Handle Main Menu Selection
            if (IsKeyPressed(KEY_ENTER))
            {
                if (mainMenuSelection == 0) // New Game selected
                {
                    InitializeScore();                          // Reset score
                    customCamera.SetPosition(initialCameraPos); // Reset camera position
                    InitializeParticles();                      // Reset particles
                    currentScreen = LOADING;                    // Switch to Loading screen
                    gameInitialized = false;                    // Mark game as not initialized yet
                    TraceLog(LOG_INFO, "MENU: Switched to LOADING state");
                }
                else if (mainMenuSelection == 1)
                    shouldExit = true; // Quit selected
            }
        }
        break;

        case LOADING:
        {
            // Actual loading happens synchronously after drawing the "Loading..." text
            // See the block after EndDrawing()
        }
        break;

        case PAUSE_MENU:
        {
            EnableCursor(); // Show mouse cursor in pause menu
            // Resume game if ESC is pressed
            if (IsKeyPressed(KEY_ESCAPE))
            {
                currentScreen = GAMEPLAY;
                DisableCursor(); // Hide cursor for gameplay
                TraceLog(LOG_INFO, "PAUSE: Resumed to GAMEPLAY via ESC key");
            }

            // Calculate Pause Menu button rectangles for mouse interaction
            int optionFontSize = 35;
            const char *pauseOpt1 = "Continue", *pauseOpt2 = "New Game", *pauseOpt3 = "Exit";
            float opt1Width = (float)MeasureText(pauseOpt1, optionFontSize);
            float opt2Width = (float)MeasureText(pauseOpt2, optionFontSize);
            float opt3Width = (float)MeasureText(pauseOpt3, optionFontSize);
            float optHeight = (float)optionFontSize; // Approximate height
            // Center buttons horizontally
            Rectangle continueRec = {screenWidth / 2.0f - opt1Width / 2.0f, screenHeight / 2.0f - 10.0f, opt1Width, optHeight};
            Rectangle newGameRec = {screenWidth / 2.0f - opt2Width / 2.0f, screenHeight / 2.0f + 40.0f, opt2Width, optHeight};
            Rectangle exitRec = {screenWidth / 2.0f - opt3Width / 2.0f, screenHeight / 2.0f + 90.0f, opt3Width, optHeight};

            // Handle Pause Menu Mouse Clicks
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (CheckCollisionPointRec(mousePos, continueRec)) // Continue clicked
                {
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                    TraceLog(LOG_INFO, "PAUSE: Resumed via Click");
                }
                else if (CheckCollisionPointRec(mousePos, newGameRec)) // New Game clicked
                {
                    InitializeScore();
                    customCamera.SetPosition(initialCameraPos);
                    InitializeParticles();
                    currentScreen = LOADING; // Go back to Loading to reset game state
                    gameInitialized = false;
                    TraceLog(LOG_INFO, "PAUSE: New Game via Click");
                }
                else if (CheckCollisionPointRec(mousePos, exitRec)) // Exit clicked
                {
                    shouldExit = true;
                }
            }
        }
        break;

        case GAMEPLAY:
        {
            if (!gameInitialized)
                break; // Don't run if assets aren't loaded

            customCamera.UpdateLook(deltaTime); // Update camera orientation based on mouse movement

            // Check for pausing input
            if (IsKeyPressed(KEY_P))
            {
                currentScreen = PAUSE_MENU;
                EnableCursor();
                TraceLog(LOG_INFO, "GAMEPLAY: Switched to PAUSE_MENU state");
            }
            else // Not pausing this frame, run gameplay logic
            {
                DisableCursor(); // Hide cursor during gameplay
                if (IsKeyPressed(KEY_F1))
                    showDebug = !showDebug; // Toggle debug view

                // Update Asteroid Shake Timers
                for (size_t i = 0; i < asteroids.size(); ++i)
                {
                    if (asteroids[i].isShaking)
                    {
                        asteroids[i].shakeTimer -= deltaTime;
                        if (asteroids[i].shakeTimer <= 0.0f)
                        {
                            asteroids[i].isShaking = false;
                            // Consider resetting color here if not bouncing
                            // asteroids[i].currentColor = asteroids[i].color;
                        }
                    }
                }

                // Update active particles
                UpdateParticles(deltaTime);

                // Handle Player Bounce State OR Normal Movement/Interaction
                if (isBouncing)
                {
                    // Update bounce timer and position
                    bounceTimer -= deltaTime;
                    if (bounceTimer <= 0.0f)
                    {
                        isBouncing = false;
                    }
                    else
                    {
                        // Apply decaying bounce movement
                        float decayFactor = bounceTimer / BOUNCE_DURATION;
                        float currentSpeed = INITIAL_BOUNCE_SPEED * decayFactor;
                        Vector3 bounceMovement = Vector3Scale(bounceDirection, currentSpeed * deltaTime);
                        customCamera.ApplyBounce(bounceMovement);
                    }
                }
                else // Not currently bouncing
                {
                    Vector3 previousPlayerPos = customCamera.GetCamera().position;
                    customCamera.UpdatePosition(deltaTime); // Update player position based on WASD/Ctrl/Space
                    Vector3 currentPlayerPos = customCamera.GetCamera().position;

                    // Flag to track if a collision occurred this frame (used for Click Miss logic)
                    bool physicalCollisionOccurred = false;

                    // Physical Collision Check between Player and Asteroids (Uses Grid)
                    if (collisionGrid != nullptr)
                    {
                        // Query the grid for asteroid indices near the player
                        std::vector<int> nearbyIndices = collisionGrid->Query(currentPlayerPos);
                        for (int index : nearbyIndices)
                        {
                            // Validate index
                            if (index < 0 || index >= asteroids.size())
                                continue;
                            const Asteroid &ast = asteroids[index];
                            if (!ast.isActive)
                                continue;

                            // Perform detailed sphere check only on nearby candidates
                            if (CheckCollisionSpheres(currentPlayerPos, 0.5f, ast.position, ast.collisionRadius)) // Added 0.5f player radius
                            {
                                // Collision detected - start bouncing
                                isBouncing = true;
                                bounceTimer = BOUNCE_DURATION;
                                bounceDirection = Vector3Normalize(Vector3Subtract(currentPlayerPos, ast.position));
                                customCamera.SetPosition(previousPlayerPos); // Move player back to pre-collision position
                                asteroids[index].currentColor = RED;         // Make the hit asteroid red
                                physicalCollisionOccurred = true;
                                TraceLog(LOG_INFO, "Player collided with nearby Asteroid %d - BOUNCING", index);
                                break; // Stop checking once a collision is found
                            }
                        }
                    }
                    else
                    {
                        TraceLog(LOG_WARNING, "Collision grid null, skip player collision");
                    }

                    // Click-to-Hit Logic (Uses Grid Raycast)
                    if (!isBouncing && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        Ray actionRay = customCamera.GetForwardRay(); // Get ray from camera center
                        RayCollision closestHit = {0};
                        closestHit.hit = false;
                        closestHit.distance = FLT_MAX;
                        int closestAsteroidIndex = -1;

                        // Query the grid for asteroid indices along the ray
                        if (collisionGrid != nullptr)
                        {
                            std::vector<int> potentialHitIndices = collisionGrid->QueryRay(actionRay, HIT_MAX_DISTANCE);
                            for (int index : potentialHitIndices)
                            {
                                if (index < 0 || index >= asteroids.size())
                                    continue; // Validate index
                                const Asteroid &ast = asteroids[index];
                                if (!ast.isActive)
                                    continue;

                                // Perform detailed ray-sphere check only on potential candidates
                                RayCollision hitInfo = GetRayCollisionSphere(actionRay, ast.position, ast.collisionRadius);

                                // Check if hit, within max distance, and closer than previous hits
                                if (hitInfo.hit && hitInfo.distance < closestHit.distance && hitInfo.distance <= HIT_MAX_DISTANCE)
                                {
                                    closestHit = hitInfo;
                                    closestAsteroidIndex = index;
                                }
                            }
                        }
                        else
                        {
                            TraceLog(LOG_WARNING, "Collision grid null, skip raycast");
                        }

                        // Process the closest hit found
                        if (closestHit.hit && closestAsteroidIndex != -1)
                        {
                            // Apply damage and effects to the hit asteroid
                            asteroids[closestAsteroidIndex].hitPoints--;
                            asteroids[closestAsteroidIndex].isShaking = true;
                            asteroids[closestAsteroidIndex].shakeTimer = SHAKE_DURATION;
                            asteroids[closestAsteroidIndex].currentColor = RED;
                            TraceLog(LOG_INFO, "Asteroid %d clicked! HP: %d Dist: %.2f", closestAsteroidIndex, asteroids[closestAsteroidIndex].hitPoints, closestHit.distance);

                            // Check if asteroid is destroyed
                            if (asteroids[closestAsteroidIndex].hitPoints <= 0)
                            {
                                asteroids[closestAsteroidIndex].isActive = false; // Deactivate asteroid
                                AddScore(10);                                     // Add score
                                TraceLog(LOG_INFO, "Asteroid %d destroyed!", closestAsteroidIndex);
                                // Emit particles at destruction point
                                EmitParticles(asteroids[closestAsteroidIndex].position, 50, 2.0f, 1.0f, asteroids[closestAsteroidIndex].color);
                            }
                        }
                        else
                        {
                            // Log a miss only if no physical collision happened this frame either
                            if (!physicalCollisionOccurred)
                                TraceLog(LOG_INFO, "Click Miss!");
                        }
                    }

                } // End else (!isBouncing)

                // Reset Asteroid Colors (Simplified Logic)
                // Checks if asteroid is not shaking and player isn't currently colliding with it
                Vector3 finalPlayerPos = customCamera.GetCamera().position;
                for (size_t i = 0; i < asteroids.size(); ++i)
                {
                    if (!asteroids[i].isActive)
                        continue;

                    bool currentlyCollidingWithThis = false;
                    // Rough check if player is colliding with this asteroid 'i'
                    if (CheckCollisionSpheres(finalPlayerPos, 0.5f, asteroids[i].position, asteroids[i].collisionRadius))
                    {
                        currentlyCollidingWithThis = true;
                    }

                    // Reset color to default grey if not shaking and player isn't colliding with it
                    if (!asteroids[i].isShaking && !currentlyCollidingWithThis)
                    {
                        asteroids[i].currentColor = asteroids[i].color;
                    }
                    else
                    {
                        asteroids[i].currentColor = RED; // Keep red if shaking or colliding
                    }
                }

                // Update Asteroid Rotations
                for (size_t i = 0; i < asteroids.size(); ++i)
                {
                    if (!asteroids[i].isActive)
                        continue;
                    asteroids[i].rotationAngle += asteroids[i].rotationSpeed * deltaTime;
                    // Keep angle within 0-360 range
                    while (asteroids[i].rotationAngle >= 360.0f)
                        asteroids[i].rotationAngle -= 360.0f;
                    while (asteroids[i].rotationAngle < 0.0f)
                        asteroids[i].rotationAngle += 360.0f;
                }
            } // End else (not pausing)
        }
        break;

        default:
            break;
        } // End switch (currentScreen) for Update

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(spaceBlueDark); // Clear with dark blue
        // Draw background gradient
        DrawRectangleGradientV(0, 0, (float)screenWidth, (float)screenHeight, spaceBlueDark, spaceBlueLight);
        DrawStars(stars); // Draw background stars

        // Draw UI or 3D Scene based on current screen
        switch (currentScreen)
        {
        case MAIN_MENU:
        {
            // Draw Main Menu Text
            const char *title = "ASTEROID FIELD";
            int titleFontSize = 60;
            int titleWidth = MeasureText(title, titleFontSize);
            DrawText(title, screenWidth / 2 - titleWidth / 2, screenHeight / 4, titleFontSize, YELLOW);
            const char *option1 = "New Game", *option2 = "Quit";
            int optionFontSize = 40;
            int option1Width = MeasureText(option1, optionFontSize);
            int option2Width = MeasureText(option2, optionFontSize);
            DrawText(option1, screenWidth / 2 - option1Width / 2, screenHeight / 2 + 0, optionFontSize, mainMenuSelection == 0 ? MAROON : GRAY);  // Highlight selected
            DrawText(option2, screenWidth / 2 - option2Width / 2, screenHeight / 2 + 50, optionFontSize, mainMenuSelection == 1 ? MAROON : GRAY); // Highlight selected
            DrawText("Use UP/DOWN keys and ENTER", 10, screenHeight - 30, 20, LIGHTGRAY);
        }
        break;
        case LOADING:
        {
            // Draw Loading Text
            const char *loadingText = "Loading Assets...";
            int loadingFontSize = 40;
            int loadingTextWidth = MeasureText(loadingText, loadingFontSize);
            DrawText(loadingText, screenWidth / 2 - loadingTextWidth / 2, screenHeight / 2 - loadingFontSize / 2, loadingFontSize, RAYWHITE);
        }
        break;
        case PAUSE_MENU:
        {
            // Draw Pause Menu Text and handle hover highlighting
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f)); // Darken background
            const char *pauseTitle = "PAUSED";
            int pauseTitleFontSize = 50;
            int pauseTitleWidth = MeasureText(pauseTitle, pauseTitleFontSize);
            DrawText(pauseTitle, screenWidth / 2 - pauseTitleWidth / 2, screenHeight / 4, pauseTitleFontSize, YELLOW);
            int optionFontSize = 35;
            const char *pauseOpt1 = "Continue", *pauseOpt2 = "New Game", *pauseOpt3 = "Exit";
            float opt1Width = (float)MeasureText(pauseOpt1, optionFontSize);
            float opt2Width = (float)MeasureText(pauseOpt2, optionFontSize);
            float opt3Width = (float)MeasureText(pauseOpt3, optionFontSize);
            float optHeight = (float)optionFontSize;
            Rectangle continueRec = {screenWidth / 2.0f - opt1Width / 2.0f, screenHeight / 2.0f - 10.0f, opt1Width, optHeight};
            Rectangle newGameRec = {screenWidth / 2.0f - opt2Width / 2.0f, screenHeight / 2.0f + 40.0f, opt2Width, optHeight};
            Rectangle exitRec = {screenWidth / 2.0f - opt3Width / 2.0f, screenHeight / 2.0f + 90.0f, opt3Width, optHeight};
            DrawText(pauseOpt1, (int)continueRec.x, (int)continueRec.y, optionFontSize, CheckCollisionPointRec(mousePos, continueRec) ? MAROON : GRAY); // Highlight on hover
            DrawText(pauseOpt2, (int)newGameRec.x, (int)newGameRec.y, optionFontSize, CheckCollisionPointRec(mousePos, newGameRec) ? MAROON : GRAY);    // Highlight on hover
            DrawText(pauseOpt3, (int)exitRec.x, (int)exitRec.y, optionFontSize, CheckCollisionPointRec(mousePos, exitRec) ? MAROON : GRAY);             // Highlight on hover
            DrawText("Click option or press ESC to Continue", 10, screenHeight - 30, 20, LIGHTGRAY);
        }
        break;

        case GAMEPLAY:
        {
            if (!gameInitialized)
                break; // Don't draw if not loaded

            // Enter 3D mode
            BeginMode3D(customCamera.GetCamera());

            // Simple Frustum Culling Setup
            Camera3D cam = customCamera.GetCamera();
            Vector3 camPos = cam.position;
            Vector3 camFwd = Vector3Normalize(Vector3Subtract(cam.target, camPos));
            float maxDrawDistanceSq = 250.0f * 250.0f;              // Max distance to draw (squared)
            float minDotProduct = cosf(cam.fovy * DEG2RAD * 0.85f); // Angle threshold based on FOV
            drawnAsteroids = 0;                                     // Reset drawn counter

            // Draw Asteroids (with Culling)
            for (size_t i = 0; i < asteroids.size(); ++i)
            {
                if (!asteroids[i].isActive)
                    continue; // Skip inactive asteroids

                // --- Frustum Culling Check ---
                Vector3 toAsteroid = Vector3Subtract(asteroids[i].position, camPos);
                float distSq = Vector3LengthSqr(toAsteroid);

                // 1. Distance Check
                if (distSq > maxDrawDistanceSq)
                    continue; // Skip if too far

                // 2. Angle Check (skip if very close)
                if (distSq > 1.0f)
                {
                    Vector3 asteroidDir = Vector3Normalize(toAsteroid);
                    float dotProduct = Vector3DotProduct(camFwd, asteroidDir);
                    if (dotProduct < minDotProduct)
                        continue; // Skip if outside rough FOV cone
                }
                // --- End Culling Check ---

                drawnAsteroids++; // Increment count of asteroids actually drawn

                // Calculate asteroid's transform (rotation, translation with shake)
                Vector3 shakeOffset = {0};
                if (asteroids[i].isShaking)
                {
                    shakeOffset.x = GetRandomFloat(-asteroids[i].shakeIntensity, asteroids[i].shakeIntensity);
                    shakeOffset.y = GetRandomFloat(-asteroids[i].shakeIntensity, asteroids[i].shakeIntensity);
                    shakeOffset.z = GetRandomFloat(-asteroids[i].shakeIntensity, asteroids[i].shakeIntensity);
                }
                Matrix matRotation = MatrixRotate(asteroids[i].rotationAxis, asteroids[i].rotationAngle * DEG2RAD);
                Matrix matTranslation = MatrixTranslate(asteroids[i].position.x + shakeOffset.x,
                                                        asteroids[i].position.y + shakeOffset.y,
                                                        asteroids[i].position.z + shakeOffset.z);
                Matrix matTransform = MatrixMultiply(matRotation, matTranslation);

                // Update material color and draw the mesh
                asteroids[i].material.maps[MATERIAL_MAP_DIFFUSE].color = asteroids[i].currentColor;
                DrawMesh(asteroids[i].mesh, asteroids[i].material, matTransform);

                // Draw debug collision spheres if enabled
                if (showDebug)
                {
                    DrawSphereWires(asteroids[i].position, asteroids[i].collisionRadius, 16, 16, YELLOW);
                }
            }

            // Draw active particles
            DrawParticles();

            // Exit 3D mode
            EndMode3D();

            // Draw Gameplay UI (on top of 3D scene)
            DrawFPS(10, 10); // Show FPS
            // Show how many asteroids were drawn after culling vs total active
            DrawText(TextFormat("Asteroids Drawn: %d/%zu", drawnAsteroids, asteroids.size()), 10, 40, 20, RAYWHITE);
            DrawText("[LMB] Hit | [P] Menu", 10, 70, 20, RAYWHITE); // Controls help text
            DrawScoreUI(screenWidth - 150, 10, 30, YELLOW);         // Draw current score
            // Show debug status
            if (showDebug)
                DrawText("Debug Spheres: ON (F1)", 10, screenHeight - 30, 20, YELLOW);
            else
                DrawText("Debug Spheres: OFF (F1)", 10, screenHeight - 30, 20, GRAY);
        }
        break;

        default:
            break;
        } // End switch (currentScreen) for Draw

        EndDrawing();
        //----------------------------------------------------------------------------------

        // --- State Transition & Heavy Loading ---
        // This block executes after drawing, primarily for the LOADING state
        if (currentScreen == LOADING && !gameInitialized)
        {
            // Cleanup previous game state if it exists
            if (collisionGrid != nullptr)
            {
                delete collisionGrid;
                collisionGrid = nullptr;
                TraceLog(LOG_INFO, "Deleted previous collision grid.");
            }
            if (!asteroids.empty())
            {
                TraceLog(LOG_INFO, "Unloading previous asteroid meshes...");
                for (size_t i = 0; i < asteroids.size(); ++i)
                    if (asteroids[i].mesh.vboId != nullptr)
                        UnloadMesh(asteroids[i].mesh);
                asteroids.clear();
            }

            // Load new game assets
            TraceLog(LOG_INFO, "Loading asteroids...");
            asteroids = InitializeAsteroidField(defaultMaterial); // Generate asteroids
            gameInitialized = true;                               // Mark as initialized
            TraceLog(LOG_INFO, "Asteroid loading complete.");

            // Initialize Collision Grid based on Asteroid Field Constants
            using namespace AsteroidFieldConstants;
            float maxPossibleAsteroidRadius = BASE_MESH_RADIUS * 3.0f;
            float extraPadding = gridCellSize.x; // Padding based on cell size
            // Calculate max world extent based on generation parameters
            float maxExtent = CLUSTER_SPREAD_RADIUS + ASTEROID_SCATTER_RADIUS + maxPossibleAsteroidRadius + extraPadding;
            Vector3 minBounds = {-maxExtent, -maxExtent, -maxExtent}; // Grid min corner
            Vector3 maxBounds = {maxExtent, maxExtent, maxExtent};    // Grid max corner
            TraceLog(LOG_INFO, "Calculated Grid Bounds: Min(%.2f) Max(%.2f)", minBounds.x, maxBounds.x);

            // Create the grid object dynamically
            collisionGrid = new UniformGrid(minBounds, maxBounds, gridCellSize);

            // Populate the grid with the loaded asteroids
            if (!asteroids.empty())
            {
                TraceLog(LOG_INFO, "Building collision grid...");
                collisionGrid->BuildInstanced(asteroids); // Add asteroids to grid cells
                TraceLog(LOG_INFO, "Collision grid built.");
            }
            else
            {
                TraceLog(LOG_WARNING, "No asteroids loaded, grid initialized empty.");
            }

            // Switch to Gameplay state
            currentScreen = GAMEPLAY;
            DisableCursor(); // Hide cursor for gameplay
        }
        // --- End State Transition ---

    } // End main game loop

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Clean up allocated resources
    if (collisionGrid != nullptr)
    {
        delete collisionGrid; // Free grid memory
        collisionGrid = nullptr;
        TraceLog(LOG_INFO, "Deleted collision grid.");
    }
    if (!asteroids.empty())
    {
        // Unload asteroid meshes from GPU memory
        TraceLog(LOG_INFO, "Unloading final asteroid meshes...");
        for (size_t i = 0; i < asteroids.size(); ++i)
            if (asteroids[i].mesh.vboId != nullptr)
                UnloadMesh(asteroids[i].mesh);
    }
    UnloadMaterial(defaultMaterial); // Unload default material

    EnableCursor(); // Ensure cursor is visible on exit
    CloseWindow();  // Close window and unload OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}