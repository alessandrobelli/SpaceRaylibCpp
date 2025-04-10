/**
 * @file custom_camera.h
 * @brief Defines a custom first-person camera controller for Raylib.
 *
 * This class encapsulates Raylib's Camera3D structure and adds specific
 * functionality for first-person shooter (FPS) style controls, including:
 * - Mouse-based look controls (pitch/yaw).
 * - Keyboard-based movement (WASD/Space/Ctrl) relative to the camera's facing direction.
 * - Management of internal orientation state (yaw, pitch, forward vector).
 * - Methods for game-specific camera interactions (e.g., bounce effects).
 *
 * It aims to provide a self-contained camera system tailored to the game's needs.
 */
#ifndef CUSTOM_CAMERA_H
#define CUSTOM_CAMERA_H

#include "raylib.h"
#include "raymath.h"
#include <cmath>

class CustomCamera
{
public:
    Camera3D camera;
    Vector3 moveSpeed;
    Vector2 mouseSensitivity;

private:
    float yaw;
    float pitch;
    Vector3 cameraFront;

public:
    CustomCamera(
        Vector3 position = {0.0f, 2.0f, 5.0f},
        Vector3 target = {0.0f, 1.8f, 0.0f},
        Vector3 up = {0.0f, 1.0f, 0.0f},
        float fovy = 60.0f,
        int projection = CAMERA_PERSPECTIVE,
        Vector3 speed = {0.15f, 0.15f, 0.15f},
        Vector2 sensitivity = {0.003f, 0.003f} // Sensitivity set in main.cpp
    )
    {
        camera.position = position;
        camera.up = up;
        camera.fovy = fovy;
        camera.projection = projection;
        moveSpeed = speed;
        mouseSensitivity = sensitivity; // Store sensitivity passed from main
        Vector3 direction = Vector3Normalize(Vector3Subtract(target, position));
        yaw = atan2f(direction.x, direction.z);
        pitch = asinf(direction.y);
        pitch = Clamp(pitch, -PI / 2.0f + 0.01f, PI / 2.0f - 0.01f);
        UpdateCameraVectors();
        camera.target = Vector3Add(camera.position, cameraFront);
    }

    // Updates camera orientation based on mouse input
    void UpdateLook(float deltaTime)
    {

        Vector2 mouseDelta = GetMouseDelta();

        yaw -= mouseDelta.x * mouseSensitivity.x;
        pitch -= mouseDelta.y * mouseSensitivity.y;

        UpdateCameraVectors();                                    // Update internal vectors based on new yaw/pitch
        camera.target = Vector3Add(camera.position, cameraFront); // Update target based on new orientation AND current position
    }

    // UpdatePosition method... (keep as is)
    void UpdatePosition(float deltaTime)
    {
        Vector3 moveVector = {0.0f, 0.0f, 0.0f};
        Vector3 worldUp = {0.0f, 1.0f, 0.0f};
        // Ensure cameraFront is up-to-date before calculating right vector
        // UpdateCameraVectors(); // Called in UpdateLook, should be current
        Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraFront, worldUp));

        if (IsKeyDown(KEY_W))
            moveVector = Vector3Add(moveVector, Vector3Scale(cameraFront, moveSpeed.z));
        if (IsKeyDown(KEY_S))
            moveVector = Vector3Subtract(moveVector, Vector3Scale(cameraFront, moveSpeed.z));
        if (IsKeyDown(KEY_A))
            moveVector = Vector3Subtract(moveVector, Vector3Scale(cameraRight, moveSpeed.x));
        if (IsKeyDown(KEY_D))
            moveVector = Vector3Add(moveVector, Vector3Scale(cameraRight, moveSpeed.x));
        if (IsKeyDown(KEY_SPACE))
            moveVector.y += moveSpeed.y;
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C))
            moveVector.y -= moveSpeed.y;

        camera.position = Vector3Add(camera.position, moveVector);
        // Update target again after position change to keep it relative
        camera.target = Vector3Add(camera.position, cameraFront);
    }

    void ApplyBounce(Vector3 bounceMovement)
    {
        camera.position = Vector3Add(camera.position, bounceMovement);
        camera.target = Vector3Add(camera.position, cameraFront);
    }

    void SetPosition(Vector3 pos)
    {
        camera.position = pos;
        camera.target = Vector3Add(camera.position, cameraFront);
    }

    Camera3D GetCamera() const
    {
        return camera;
    }

    Ray GetForwardRay() const
    {
        Ray forwardRay;
        forwardRay.position = camera.position;
        forwardRay.direction = cameraFront;
        return forwardRay;
    }

private:
    void UpdateCameraVectors()
    {
        cameraFront.x = sinf(yaw) * cosf(pitch);
        cameraFront.y = sinf(pitch);
        cameraFront.z = cosf(yaw) * cosf(pitch);
        cameraFront = Vector3Normalize(cameraFront);
    }
};

#endif // CUSTOM_CAMERA_H
