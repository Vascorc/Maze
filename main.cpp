/**
 * \mainpage Documentação do Projeto Labirinto 3D
 *
 * \section intro_sec Introdução
 *
 * Este projeto é um jogo de labirinto 3D desenvolvido em C++ usando OpenGL e GLFW.
 * O objetivo do jogo é encontrar a saída do labirinto enquanto navega por corredores texturizados com iluminação dinâmica.
 *
 * \section features_sec Funcionalidades Principais
 *
 * - **Renderização 3D**: Utiliza OpenGL para renderizar o labirinto, chão, skybox e sobreposições.
 * - **Câmara FPS**: Movimentação livre em primeira pessoa com suporte a rato e teclado.
 * - **Sistema de Colisões**: Implementação robusta de colisões com paredes e deteção de chão/terreno.
 * - **Iluminação**: Suporte para iluminação ambiente, difusa e especular, incluindo uma lanterna (spotlight).
 * - **Texturas**: Suporte para texturas em paredes, chão, objetos e skybox.
 * - **Interface**: Sobreposições gráficas para condições de vitória e controlos.
 *
 * \section install_sec Instalação e Execução
 *
 * O projeto requer bibliotecas OpenGL, GLFW, GLAD, GLM e STB Image.
 * Compile usando o Makefile fornecido ou CMake.
 *
 * \author Alexandre Santos
 * \author Vasco Colaço
 * \date 2025
 */

/**
 * @file main.cpp
 * @brief Ponto de entrada principal para o Jogo Labirinto 3D.
 * 
 * Este ficheiro contém o loop principal do jogo, código de inicialização para OpenGL/GLFW,
 * e lógica de gestão de entrada. Configura a cena (labirinto, câmara, iluminação) e
 * gere o ciclo de renderização.
 * 
 * @author Alexandre Santos
 * @author Vasco Colaço
 * @date 2025
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>
#include <Maze.h>
#include <Skybox.h>
#include <OverlayRenderer.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vector>
#include <iostream>

// Protótipos de funções
/**
 * @brief Função de callback quando o tamanho da janela muda.
 * @param window A janela que mudou de tamanho.
 * @param width Nova largura.
 * @param height Nova altura.
 */
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

/**
 * @brief Função de callback para movimento do rato.
 * @param window A janela que recebe o evento.
 * @param xpos Posição X do rato.
 * @param ypos Posição Y do rato.
 */
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

/**
 * @brief Função de callback para scroll do rato.
 * @param window A janela que recebe o evento.
 * @param xoffset Offset de scroll X.
 * @param yoffset Offset de scroll Y.
 */
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

/**
 * @brief Processa toda a entrada (teclas) para o frame atual.
 * @param window A janela.
 * @param camera O objeto câmara para controlar.
 * @param deltaTime Tempo decorrido desde o último frame.
 * @param maze O objeto labirinto (para deteção de colisões).
 */
void processInput(GLFWwindow *window, Camera &camera, float deltaTime, Maze &maze);

// Configurações da janela
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Variáveis globais
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool noclip = false;
bool flashLightOn = true;

float lightIntensity = 1.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 topLightPos(0.0f, 50.0f, 0.0f);

bool isFullscreen = false;
int savedXPos, savedYPos, savedWidth, savedHeight;

bool victoryAchieved = false;
float victoryTime = 0.0f;

bool showControls = false;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Obter monitor primário e sua resolução
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    // Criar janela em tela cheia
    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Labirinto 3D", monitor, NULL);
    if (!window) {
        std::cout << "Erro ao criar janela GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Marcar como fullscreen desde o início
    isFullscreen = true;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Erro ao inicializar GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Carregar shaders
    Shader lightingShader("shaders/2.1.basic_lighting.vs", "shaders/2.1.basic_lighting.fs");
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
    Shader overlayShader("shaders/overlay.vs", "shaders/overlay.fs");

    // Configurar labirinto
    Maze maze("models/3d-model.obj");
    camera.Position = maze.startPosition;
    camera.MovementSpeed = maze.modelSize / 20.0f;
    camera.MouseSensitivity = 0.005f;

    // Carregar textura da parede (bricks)
    stbi_set_flip_vertically_on_load(true);
    unsigned int wallTexture;
    glGenTextures(1, &wallTexture);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("imagens/wall_texture.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Textura parede carregada!" << std::endl;
    } else {
        std::cout << "Falha ao carregar textura parede" << std::endl;
    }
    stbi_image_free(data);

    // Carregar textura do chão
    unsigned int floorTexture;
    glGenTextures(1, &floorTexture);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("imagens/floor_texture.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Textura chao carregada!" << std::endl;
    } else {
        std::cout << "Falha ao carregar textura chao" << std::endl;
    }
    stbi_image_free(data);

    // Carregar textura do portão (gate)
    unsigned int gateTexture;
    glGenTextures(1, &gateTexture);
    glBindTexture(GL_TEXTURE_2D, gateTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("imagens/gate_texture.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Textura portao carregada!" << std::endl;
    } else {
        std::cout << "Falha ao carregar textura portao" << std::endl;
    }
    stbi_image_free(data);

    // Carregar textura dos controlos
    unsigned int controlsTexture;
    glGenTextures(1, &controlsTexture);
    glBindTexture(GL_TEXTURE_2D, controlsTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("imagens/controlos.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        std::cout << "Textura controlos carregada!" << std::endl;
    } else {
        std::cout << "Falha ao carregar textura controlos" << std::endl;
    }
    stbi_image_free(data);

    // Carregar textura de vitoria
    unsigned int victoryTexture;
    glGenTextures(1, &victoryTexture);
    glBindTexture(GL_TEXTURE_2D, victoryTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("imagens/victory.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        std::cout << "Textura vitoria carregada!" << std::endl;
    } else {
        std::cout << "Falha ao carregar textura vitoria" << std::endl;
    }
    stbi_image_free(data);

    // Configurar skybox
    std::vector<std::string> faces {
        "imagens/right.png",
        "imagens/left.png",
        "imagens/top.png",
        "imagens/bottom.png",
        "imagens/front.png",
        "imagens/back.png"
    };
    Skybox skybox(faces);
    OverlayRenderer overlayRenderer;

    // Mostrar controlos
    std::cout << "\n========== CONTROLOS ==========" << std::endl;
    std::cout << "W/A/S/D       - Mover (frente/esquerda/tras/direita)" << std::endl;
    std::cout << "SHIFT         - Correr (2x velocidade)" << std::endl;
    std::cout << "F             - Ligar/Desligar lanterna" << std::endl;
    std::cout << "V             - Noclip (atravessar paredes)" << std::endl;
    std::cout << "TAB           - Mostrar/Esconder controlos" << std::endl;
    std::cout << "ESC           - Sair do jogo" << std::endl;
    std::cout << "Mouse         - Olhar em volta" << std::endl;
    std::cout << "==============================\n" << std::endl;

    // Loop de renderização
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Ciclo dia/noite
        lightIntensity -= 0.02f * (deltaTime / 2);
        if (lightIntensity < 0.1f) lightIntensity = 0.1f;

        processInput(window, camera, deltaTime, maze);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Desenhar skybox
        int scrWidth, scrHeight;
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)scrWidth / (float)scrHeight, 0.1f, 5000.0f);
        
        skybox.draw(skyboxShader, camera.GetViewMatrix(), projection, lightIntensity);

        // Desenhar labirinto
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("lightIntensity", lightIntensity);
        lightingShader.setVec3("topLightPos", topLightPos);
        
        // Ativar texturas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        lightingShader.setInt("wallTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        lightingShader.setInt("floorTexture", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gateTexture);
        lightingShader.setInt("gateTexture", 2);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", model);

        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setVec3("flashLightDir", camera.Front);
        lightingShader.setFloat("flashLightCutoff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("flashLightOuterCutoff", glm::cos(glm::radians(17.5f)));
        lightingShader.setBool("flashLightOn", flashLightOn);

        // Desenhar Labirinto (Tipo 0)
        lightingShader.setInt("objectType", 0);
        maze.draw(lightingShader);

        // Desenhar Saida (Tipo 1)
        lightingShader.setInt("objectType", 1);
        // maze.drawExit(lightingShader);

        // Ecrã de vitória
        if (victoryAchieved) {
            victoryTime += deltaTime;
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            
            // Usar a imagem de vitoria em vez de apenas cor
            overlayRenderer.renderImageOverlay(overlayShader, victoryTexture, (float)scrWidth, (float)scrHeight);
             
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        // Mostrar controlos (TAB)
        if (showControls) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            
            overlayRenderer.renderImageOverlay(overlayShader, controlsTexture, (float)scrWidth, (float)scrHeight);
            
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, Camera &camera, float deltaTime, Maze &maze)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Noclip (V)
    static bool vPressed = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!vPressed) {
            noclip = !noclip;
            std::cout << "Noclip: " << (noclip ? "LIGADO" : "DESLIGADO") << std::endl;
            vPressed = true;
        }
    } else {
        vPressed = false;
    }

    // Lanterna (F)
    static bool fPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fPressed) {
            flashLightOn = !flashLightOn;
            fPressed = true;
        }
    } else {
        fPressed = false;
    }

    // Mostrar controlos (TAB)
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabPressed) {
            showControls = !showControls;
            tabPressed = true;
        }
    } else {
        tabPressed = false;
    }

    // Fullscreen (F11)
    static bool f11Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        if (!f11Pressed) {
            if (isFullscreen) {
                glfwSetWindowMonitor(window, NULL, savedXPos, savedYPos, savedWidth, savedHeight, 0);
                isFullscreen = false;
            } else {
                glfwGetWindowPos(window, &savedXPos, &savedYPos);
                glfwGetWindowSize(window, &savedWidth, &savedHeight);
                
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                isFullscreen = true;
            }
            f11Pressed = true;
        }
    } else {
        f11Pressed = false;
    }



    // Sprint (Shift)
    float originalSpeed = camera.MovementSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.MovementSpeed *= 1.5f;
    }

    // Sub-steps de física
    int steps = 4; // Reduzido de 8 para 4 para melhor performance
    float subDeltaTime = deltaTime / steps;
    
    // Cache da altura inicial para evitar chamadas redundantes (Otimizacao)
    float currentFloorHeight = -99999.0f;
    if (!noclip) {
        currentFloorHeight = maze.getFloorHeight(camera.Position);
        if (currentFloorHeight < -90000.0f) currentFloorHeight = camera.Position.y - 50.0f;
    }

    for (int i = 0; i < steps; i++) {
        glm::vec3 stepOldPosition = camera.Position;
        float oldFloorHeight = currentFloorHeight;
        
        // Movimento WASD
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(0, subDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(1, subDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(2, subDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(3, subDeltaTime);
            
        // Logica de colisao e chao
        if (!noclip) {
            // 1. Parede
            if (maze.checkWallCollision(camera.Position, 5.0f)) {
                camera.Position = stepOldPosition;
            } else {
                // 2. Chao (So calcula se nao bateu na parede)
                float newFloorHeight = maze.getFloorHeight(camera.Position);
                
                // Se chao invalido (buraco/void)
                if (newFloorHeight < -90000.0f) {
                     camera.Position = stepOldPosition;
                } else {
                    const float MAX_STEP_HEIGHT = 15.0f;
                    float heightDiff = newFloorHeight - oldFloorHeight;
                    
                    if (heightDiff > MAX_STEP_HEIGHT) {
                        // Degrau muito alto
                        camera.Position.x = stepOldPosition.x;
                        camera.Position.z = stepOldPosition.z;
                    } else {
                        // Valido
                        camera.Position.y = newFloorHeight + 50.0f;
                        currentFloorHeight = newFloorHeight;
                    }
                }
            }
        }
    }

    camera.MovementSpeed = originalSpeed;
    
    // À prova de falhas: Se o jogador cair do mapa, reiniciar no início
    if (camera.Position.y < -300.0f) {
        std::cout << "Failsafe ativado! A reiniciar jogador." << std::endl;
        camera.Position = maze.startPosition;
        camera.Position.y += 50.0f;
    }

    // Verificar chegada ao portão
    if (!victoryAchieved) {
        glm::vec2 cameraPos2D(camera.Position.x, camera.Position.z);
        glm::vec2 exitPos2D(maze.exitPosition.x, maze.exitPosition.z);
        if (glm::distance(cameraPos2D, exitPos2D) < 50.0f) {
            std::cout << "========================================" << std::endl;
            std::cout << "   PARABENS! CHEGASTE AO PORTAO!" << std::endl;
            std::cout << "   Pressiona ESC para sair" << std::endl;
            std::cout << "========================================" << std::endl;
            victoryAchieved = true;
            victoryTime = 0.0f;
        }
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}