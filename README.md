# SpaceCPP - Asteroid Field Tech Demo

A simple 3D tech demo built with C++ and the Raylib library, featuring a procedurally generated asteroid field and first-person navigation.

## Description

This project demonstrates several graphics and game development techniques, including procedural generation, 3D camera controls, basic physics interactions, UI elements, and optimization strategies within a space setting.

## Features

* **Procedurally Generated Asteroid Field:** Creates a field of asteroids with varying shapes, sizes, and orientations distributed in clusters.
* **First-Person Camera:** Custom camera controller implementing mouse-look (pitch/yaw) and keyboard movement (WASD, Space, Ctrl/C) relative to the camera's direction.
* **Asteroid Interaction:** Players can "hit" asteroids by clicking the left mouse button when aiming at them. Asteroids have hit points and visual feedback (shaking, color change) upon being hit.
* **Particle System:** Simple particle effects are generated when an asteroid is destroyed.
* **Score System:** Tracks and displays the player's score, incrementing when asteroids are destroyed.
* **Basic UI:** Includes a Main Menu (New Game, Quit) and a Pause Menu (Continue, New Game, Exit).
* **Collision Detection (Uniform Grid):** Utilizes a uniform grid spatial partitioning structure to optimize collision detection between the player/mouse clicks and asteroids.
* **Rendering Optimization:** Implements basic frustum culling to avoid drawing asteroids outside the camera's view or beyond a certain distance.
* **Background:** Simple starfield background.

## Controls

* **W, A, S, D:** Move Forward / Left / Backward / Right
* **Space:** Move Up
* **Left Ctrl / C:** Move Down
* **Mouse:** Look Around
* **Left Mouse Button (LMB):** Hit targeted asteroid
* **P:** Pause / Unpause Game
* **F1:** Toggle Debug View (Show Collision Spheres)
* **ESC:** Resume game from Pause Menu
* **Up/Down Arrows (Menu):** Navigate options
* **Enter (Menu):** Select option

## Dependencies

* **Raylib:** The core graphics and utility library used.

## How to Build (Example - Requires Raylib setup)

*(You would typically add specific build instructions here, e.g., using CMake or a simple Makefile)*

```bash
# Example using g++ (replace with your compiler and Raylib paths)
g++ src/*.cpp -o asteroid_demo -lraylib -lGL -lm -lpthread -ldl -lrt -lX11