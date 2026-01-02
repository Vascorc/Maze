#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <stb_image.h>
#include <shader_m.h>

class Skybox {
public:
    unsigned int VAO, VBO;
    unsigned int textureID;

    Skybox(const std::vector<std::string>& faces) {
        setupMesh();
        textureID = loadCubemap(faces);
    }

    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection, float lightIntensity) {
        glDepthFunc(GL_LEQUAL);
        shader.use();
        
        // Converter matriz de visualização para mat3 para remover translação (assim o skybox fica em torno do jogador)
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        
        shader.setMat4("view", skyboxView);
        shader.setMat4("projection", projection);
        shader.setFloat("lightIntensity", lightIntensity);

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

private:
    void setupMesh() {
        float skyboxVertices[] = {
            // posições          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    unsigned int loadCubemap(std::vector<std::string> faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        // IMPORTANTE: Desativar flip vertical para cubemaps
        // (o flip é usado para texturas normais, mas não para skybox)
        stbi_set_flip_vertically_on_load(false);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
                std::cout << "Skybox face " << i << " carregada: " << faces[i] << std::endl;
            }
            else
            {
                std::cout << "Falha ao carregar Cubemap em: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }

        // Configurar filtros de textura para melhor qualidade
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Ativar seamless cubemap para eliminar costuras entre faces (OpenGL 3.2+)
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // Reativar flip vertical para outras texturas que possam ser carregadas depois
        stbi_set_flip_vertically_on_load(true);

        return textureID;
    }
};

#endif
