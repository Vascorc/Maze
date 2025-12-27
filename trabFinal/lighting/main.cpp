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
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
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
        
        // Bind textures
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

        // Draw Maze (Type 0)
        lightingShader.setInt("objectType", 0);
        maze.draw(lightingShader);

        // Draw Exit (Type 1)
        lightingShader.setInt("objectType", 1);
        maze.drawExit(lightingShader);

        // Ecrã de vitória
        if (victoryAchieved) {
            victoryTime += deltaTime;
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            
            float fadeAlpha = glm::min(victoryTime / 0.5f, 1.0f);
            overlayRenderer.renderOverlay(overlayShader, glm::vec3(1.0f, 0.84f, 0.0f), fadeAlpha * 0.6f, victoryTime);
            
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        // Mostrar controlos (TAB)
        if (showControls) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            
            overlayRenderer.renderImageOverlay(overlayShader, controlsTexture);
            
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
            std::cout << "Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
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

    // Physics Sub-stepping
    int steps = 4;
    float subDeltaTime = deltaTime / steps;
    
    for (int i = 0; i < steps; i++) {
        glm::vec3 stepOldPosition = camera.Position;
        
        // Movimento WASD
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(0, subDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(1, subDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(2, subDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(3, subDeltaTime);
            
        // Colisão com paredes
        if (!noclip && maze.checkWallCollision(camera.Position, 5.0f)) {
            camera.Position = stepOldPosition;
        }
        
        // Gravidade e seguimento do terreno
        if (!noclip) {
            float oldFloorHeight = maze.getFloorHeight(stepOldPosition);
            float newFloorHeight = maze.getFloorHeight(camera.Position);
            
            // Revert if invalid floor (void)
            if (newFloorHeight < -90000.0f) {
                 camera.Position = stepOldPosition;
                 newFloorHeight = oldFloorHeight;
            } else {
                const float MAX_STEP_HEIGHT = 15.0f;
                float heightDiff = newFloorHeight - oldFloorHeight;
                
                // Revert if Step is too high
                if (heightDiff > MAX_STEP_HEIGHT) {
                    camera.Position.x = stepOldPosition.x;
                    camera.Position.z = stepOldPosition.z;
                    newFloorHeight = oldFloorHeight;
                }
            }
            
            // Apply height
            if (newFloorHeight > -90000.0f) {
                 camera.Position.y = newFloorHeight + 50.0f;
            }
        }
    }

    camera.MovementSpeed = originalSpeed;
    
    // Failsafe: If player fell through map, reset to start
    if (camera.Position.y < -300.0f) {
        std::cout << "Failsafe triggered! Resetting player." << std::endl;
        camera.Position = maze.startPosition;
        camera.Position.y += 50.0f;
    }

    // Verificar chegada ao portão
    if (!victoryAchieved) {
        glm::vec2 cameraPos2D(camera.Position.x, camera.Position.z);
        glm::vec2 exitPos2D(maze.exitPosition.x, maze.exitPosition.z);
        if (glm::distance(cameraPos2D, exitPos2D) < 10.0f) {
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