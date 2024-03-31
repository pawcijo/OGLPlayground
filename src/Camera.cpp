#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

void Camera::updateCameraVectors()
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector

    // Normalize the vectors, because their length gets closer to 0 the more
    // you look up or down which results in slower movement.
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

Camera::Camera() : Position(CAMERA_DEFAULT_POSTITION),
                   WorldUp(CAMERA_DEFAULT_WORLD_UP),
                   Yaw(CAMERA_DEFAULT_YAW),
                   Front(CAMERA_DAFAULT_FRONT),
                   MovementSpeed(CAMERA_DEFAULT_SPEED),
                   Pitch(CAMERA_DEFAULT_PITCH),
                   MouseSensitivity(CAMERA_DEFAULT_MOUSE_SENSIVITY),
                   Zoom(CAMERA_DEFAULT_ZOOM)
{
    updateCameraVectors();
}

// Constructor with vectors
Camera::Camera(glm::vec3 position,
               glm::vec3 worldUp,
               float yaw,
               float pitch) : Front(CAMERA_DAFAULT_FRONT),
                              MovementSpeed(CAMERA_DEFAULT_SPEED),
                              MouseSensitivity(CAMERA_DEFAULT_MOUSE_SENSIVITY),
                              Zoom(CAMERA_DEFAULT_ZOOM),
                              Position(position),
                              WorldUp(worldUp),
                              Yaw(yaw),
                              Pitch(pitch)
{
    updateCameraVectors();
}
// Constructor with scalar values
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(CAMERA_DAFAULT_FRONT),
                                                                                                              MovementSpeed(CAMERA_DEFAULT_SPEED),
                                                                                                              MouseSensitivity(CAMERA_DEFAULT_MOUSE_SENSIVITY),
                                                                                                              Zoom(CAMERA_DEFAULT_ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    printf("Pitch %f  yaw %f Position : x: %f y: %f z: %f  \n", Pitch, Yaw, Position.x, Position.y, Position.z);
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{

    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }
    // Update Front, Right and Up Vectors using the updated Eular angles
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    if (Zoom >= 1.0f && Zoom <= 45.0f)
        Zoom -= yoffset * 0.1f;
    if (Zoom <= 1.0f)
        Zoom = 1.0f;
    if (Zoom >= 45.0f)
        Zoom = 45.0f;
}