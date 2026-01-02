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
        shader.setVec2("scale", glm::vec2(1.0f, 1.0f)); 
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void renderImageOverlay(Shader& shader, unsigned int texture, float scrWidth, float scrHeight, float imgAspect = 1.777f) {
        shader.use();
        shader.setVec3("overlayColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setFloat("alpha", 0.95f);  
        shader.setFloat("time", 0.0f);
        shader.setInt("useTexture", 1);  
        
        float scrAspect = scrWidth / scrHeight;
        glm::vec2 scale(1.0f);
        
        // Lógica de ajuste: Manter aspect ratio da imagem dentro do ecrã
        if (scrAspect > imgAspect) {
            // Ecrã mais largo que a imagem (barras laterais)
            scale.x = imgAspect / scrAspect;
        } else {
            // Ecrã mais alto que a imagem (barras em cima/baixo)
            scale.y = scrAspect / imgAspect;
        }
        shader.setVec2("scale", scale);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.setInt("imageTexture", 0);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        shader.setInt("useTexture", 0);  
    }

    ~OverlayRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

#endif
