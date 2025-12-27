#ifndef OVERLAY_RENDERER_H
#define OVERLAY_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader_m.h>

class OverlayRenderer {
public:
    unsigned int VAO, VBO;
    
    OverlayRenderer() {
        float quadVertices[] = {
            // posições   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void renderOverlay(Shader& shader, glm::vec3 color, float alpha, float time) {
        shader.use();
        shader.setVec3("overlayColor", color);
        shader.setFloat("alpha", alpha);
        shader.setFloat("time", time);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void renderImageOverlay(Shader& shader, unsigned int texture) {
        shader.use();
        shader.setVec3("overlayColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setFloat("alpha", 0.95f);  // Quase opaco
        shader.setFloat("time", 0.0f);
        shader.setInt("useTexture", 1);  // Ativar modo textura
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.setInt("imageTexture", 0);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        shader.setInt("useTexture", 0);  // Desativar modo textura
    }

    ~OverlayRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

#endif
