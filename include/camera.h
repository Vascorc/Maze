#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @class Camera
 * @brief Classe para controlar a câmara usando Ângulos de Euler.
 * 
 * Define uma câmara com posição, orientação (usando ângulos de Euler), e
 * capacidades para processar entrada de teclado e rato para movimento (estilo FPS).
 */
class Camera
{
public:
    // Atributos da Câmara
    glm::vec3 Position;     ///< Posição da câmara no espaço do mundo
    glm::vec3 Front;        ///< Vetor a apontar para a frente da câmara
    glm::vec3 Up;           ///< Vetor a apontar para cima em relação à câmara
    glm::vec3 Right;        ///< Vetor a apontar para a direita em relação à câmara
    glm::vec3 WorldUp;      ///< Vetor cima global (geralmente 0,1,0)

    // Ângulos de Euler
    float Yaw;              ///< Ângulo Yaw (rotação horizontal)
    float Pitch;            ///< Ângulo Pitch (rotação vertical)

    // Opções da câmara
    float MovementSpeed;    ///< Velocidade de movimento da câmara
    float MouseSensitivity; ///< Sensibilidade do movimento do rato
    float Zoom;             ///< Campo de visão (nível de zoom)

    /**
     * @brief Construtor com vetores.
     * 
     * @param position Posição inicial da câmara.
     * @param up Vetor cima global.
     * @param yaw Ângulo yaw inicial.
     * @param pitch Ângulo pitch inicial.
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
     * @brief Retorna a matriz de visualização calculada usando Ângulos de Euler e a Matriz LookAt.
     * @return glm::mat4 A matriz de visualização.
     */
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    /**
     * @brief Processa entrada recebida de qualquer sistema de entrada tipo teclado.
     * 
     * @param direction Direção do movimento (0=FRENTE, 1=TRÁS, 2=ESQUERDA, 3=DIREITA).
     * @param deltaTime Tempo entre o frame atual e o último frame.
     */
    void ProcessKeyboard(int direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 0) // FRENTE
            Position += Front * velocity;
        if (direction == 1) // TRÁS
            Position -= Front * velocity;
        if (direction == 2) // ESQUERDA
            Position -= Right * velocity;
        if (direction == 3) // DIREITA
            Position += Right * velocity;
        
        // Bloquear movimento Y para sensação de FPS (opcional, mas bom para caminhar)
        // Position.y = 0.0f; 
    }

    /**
     * @brief Processa entrada recebida de um sistema de entrada rato.
     * 
     * @param xoffset Valor de deslocamento na direção x.
     * @param yoffset Valor de deslocamento na direção y.
     * @param constrainPitch Se deve restringir o valor de pitch para evitar virar o ecrã.
     */
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // Garantir que quando o pitch sai dos limites, o ecrã não vira ao contrário
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
     * @brief Processa entrada recebida de um evento de roda de scroll do rato.
     * 
     * @param yoffset Valor de deslocamento do scroll.
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
     * @brief Calcula o vetor frontal a partir dos Ângulos de Euler (atualizados) da Câmara.
     */
    void updateCameraVectors()
    {
        // Calcular o novo vetor Front
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Também recalcular o vetor Right e Up
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalizar os vetores, porque seu comprimento fica mais próximo de 0 quanto mais você olha para cima ou para baixo, o que resulta em movimento mais lento.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
