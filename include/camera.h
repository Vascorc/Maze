#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @class Camera
 * @brief Class to handle a camera using Euler Angles.
 * 
 * Defines a camera with position, orientation (using Euler angles), and
 * capabilities to process keyboard and mouse input for movement (FPS style).
 */
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;     ///< Camera position in world space
    glm::vec3 Front;        ///< Vector pointing to the front of the camera
    glm::vec3 Up;           ///< Vector pointing up relative to the camera
    glm::vec3 Right;        ///< Vector pointing to the right relative to the camera
    glm::vec3 WorldUp;      ///< Global up vector (usually 0,1,0)

    // Euler Angles
    float Yaw;              ///< Yaw angle (horizontal rotation)
    float Pitch;            ///< Pitch angle (vertical rotation)

    // Camera options
    float MovementSpeed;    ///< Speed of camera movement
    float MouseSensitivity; ///< Sensitivity of mouse movement
    float Zoom;             ///< Field of view (zoom level)

    /**
     * @brief Constructor with vectors.
     * 
     * @param position Initial position of the camera.
     * @param up Global up vector.
     * @param yaw Initial yaw angle.
     * @param pitch Initial pitch angle.
     */
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    /**
     * @brief Returns the view matrix calculated using Euler Angles and the LookAt Matrix.
     * @return glm::mat4 The view matrix.
     */
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    /**
     * @brief Processes input received from any keyboard-like input system.
     * 
     * @param direction Direction of movement (0=FORWARD, 1=BACKWARD, 2=LEFT, 3=RIGHT).
     * @param deltaTime Time between current frame and last frame.
     */
    void ProcessKeyboard(int direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 0) // FORWARD
            Position += Front * velocity;
        if (direction == 1) // BACKWARD
            Position -= Front * velocity;
        if (direction == 2) // LEFT
            Position -= Right * velocity;
        if (direction == 3) // RIGHT
            Position += Right * velocity;
        
        // Lock Y movement for FPS feel (optional, but good for walking)
        // Position.y = 0.0f; 
    }

    /**
     * @brief Processes input received from a mouse input system.
     * 
     * @param xoffset Offset value in x direction.
     * @param yoffset Offset value in y direction.
     * @param constrainPitch Whether to constrain the pitch value to prevent screen flipping.
     */
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    /**
     * @brief Processes input received from a mouse scroll-wheel event.
     * 
     * @param yoffset Offset value of the scroll.
     */
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    /**
     * @brief Calculates the front vector from the Camera's (updated) Euler Angles.
     */
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
