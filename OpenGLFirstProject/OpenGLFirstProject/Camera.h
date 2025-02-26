#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 15.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    glm::vec3 WorldForward = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 WorldRight = glm::vec3(1.0f, 0.0f, 0.0f);

    // Euler angles
    float Yaw;
    float Pitch;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Constructor with scalars
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return customLookAt(Position, Position + Front, Up);
    }

    glm::mat4 GetViewMatrix_Behind()
    {
        return customLookAt(Position, Position - Front, Up);
    }

    // Process input received from any keyboard-like input system
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        // "Camera" chapter exercise 1 is to move along a plane as in a traditional FPS.
        // Answer: Instead of moving along the camera's forward, we move along the cross of the camera's right and the world up,
        // because that gives us the world forward in the direction we're facing
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += glm::normalize(glm::cross(-Right, WorldUp)) * velocity;
        if (direction == BACKWARD)
            Position -= glm::normalize(glm::cross(-Right, WorldUp)) * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // Process input received from a mouse input system. Excepts the offset value in both the x and y directions
    void ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch = true)
    {
        xOffset *= MouseSensitivity;
        yOffset *= MouseSensitivity;

        Yaw += xOffset;
        Pitch += yOffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // Process input received from a mouse scroll-wheel event.
    void ProcessMouseScroll(float yOffset)
    {
        Zoom -= yOffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    glm::mat4 customLookAt(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
    {
        // Exercise 2 is to creatre our own lookat matrix
        glm::vec3 zAxis = glm::normalize(position - target);
        glm::vec3 xAxis = glm::normalize(glm::cross(glm::normalize(worldUp), zAxis));
        glm::vec3 yAxis = glm::cross(zAxis, xAxis);

        // Important note!! (because I screwed this up)
        // Matrix definitions are along COLUMNS, not rows.
        glm::mat4 rotMatrix = glm::mat4(xAxis.x, yAxis.x, zAxis.x, 0,
                                        xAxis.y, yAxis.y, zAxis.y, 0,
                                        xAxis.z, yAxis.z, zAxis.z, 0,
                                        0,       0,       0,       1);
        
        glm::mat4 posMatrix = glm::mat4(1, 0, 0, 0,
                                        0, 1, 0, 0,
                                        0, 0, 1, 0,
                                        -position.x, -position.y, -position.z, 1);

        glm::mat4 lookAt = rotMatrix * posMatrix;
        return lookAt;
    }
};
#endif