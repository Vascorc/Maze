#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <ctime>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>

class Maze {
public:
    unsigned int VAO, VBO;
    unsigned int exitVAO, exitVBO; // For exit marker
    std::vector<float> vertices; // Stores position (3), normal (3)
    
    struct Triangle {
        glm::vec3 v0, v1, v2;
        glm::vec3 normal;
        glm::vec3 centroid;
    };

    std::vector<Triangle> floorTriangles;
    std::vector<Triangle> wallTriangles;

    glm::vec3 startPosition;
    glm::vec3 exitPosition;
    float modelSize;
    
    // Bounding box
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    Maze(const std::string& filepath) {
        loadModel(filepath);
        calculateBounds();
        addFloor(); // Add procedural floor
        setupMesh();
        initExitMarker(); // Setup exit cube
        setRandomStartAndExit();
    }

    void loadModel(const std::string& filepath) {
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "models/"; 

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(filepath, reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader: " << reader.Error() << std::endl;
            }
            return;
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning() << std::endl;
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // We assume triangles (fv=3)
                if (fv != 3) {
                    index_offset += fv;
                    continue;
                }

                Triangle tri;
                glm::vec3 faceNormal;
                
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    
                    float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    float vz = attrib.vertices[3 * idx.vertex_index + 2];

                    if (v == 0) tri.v0 = glm::vec3(vx, vy, vz);
                    if (v == 1) tri.v1 = glm::vec3(vx, vy, vz);
                    if (v == 2) tri.v2 = glm::vec3(vx, vy, vz);
                }
                
                // Calculate face normal FIRST (before adding to vertices)
                glm::vec3 edge1 = tri.v1 - tri.v0;
                glm::vec3 edge2 = tri.v2 - tri.v0;
                faceNormal = glm::normalize(glm::cross(edge1, edge2));
                
                // Now add vertices with the calculated face normal
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    
                    float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    float vz = attrib.vertices[3 * idx.vertex_index + 2];

                    // Store position
                    vertices.push_back(vx); 
                    vertices.push_back(vy); 
                    vertices.push_back(vz);
                    
                    // Use calculated face normal (more reliable than OBJ normals)
                    vertices.push_back(faceNormal.x);
                    vertices.push_back(faceNormal.y);
                    vertices.push_back(faceNormal.z);
                }
                
                tri.normal = faceNormal;
                tri.centroid = (tri.v0 + tri.v1 + tri.v2) / 3.0f;

                // Classify
                if (tri.normal.y > 0.7f) {
                    floorTriangles.push_back(tri);
                } else {
                    wallTriangles.push_back(tri);
                }

                index_offset += fv;
            }
        }
    }

    void addFloor() {
        // Add a large quad at Y = minBounds.y
        float y = minBounds.y;
        float expand = 100.0f; 
        float minX = minBounds.x - expand;
        float maxX = maxBounds.x + expand;
        float minZ = minBounds.z - expand;
        float maxZ = maxBounds.z + expand;

        // Helper to add triangle to floorTriangles and vertices
        auto addTri = [&](glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
            Triangle tri; tri.v0 = v0; tri.v1 = v1; tri.v2 = v2;
            tri.normal = glm::vec3(0, 1, 0);
            tri.centroid = (v0 + v1 + v2) / 3.0f;
            floorTriangles.push_back(tri);

            vertices.push_back(v0.x); vertices.push_back(v0.y); vertices.push_back(v0.z);
            vertices.push_back(0); vertices.push_back(1); vertices.push_back(0);
            vertices.push_back(v1.x); vertices.push_back(v1.y); vertices.push_back(v1.z);
            vertices.push_back(0); vertices.push_back(1); vertices.push_back(0);
            vertices.push_back(v2.x); vertices.push_back(v2.y); vertices.push_back(v2.z);
            vertices.push_back(0); vertices.push_back(1); vertices.push_back(0);
        };

        addTri(glm::vec3(minX, y, maxZ), glm::vec3(maxX, y, maxZ), glm::vec3(maxX, y, minZ));
        addTri(glm::vec3(minX, y, maxZ), glm::vec3(maxX, y, minZ), glm::vec3(minX, y, minZ));
    }

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void initExitMarker() {
        float cubeVertices[] = {
            // positions          // normals
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
        };

        glGenVertexArrays(1, &exitVAO);
        glGenBuffers(1, &exitVBO);

        glBindVertexArray(exitVAO);
        glBindBuffer(GL_ARRAY_BUFFER, exitVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void calculateBounds() {
        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();
        float maxZ = std::numeric_limits<float>::lowest();

        for (size_t i = 0; i < vertices.size(); i += 6) {
            float x = vertices[i];
            float y = vertices[i+1];
            float z = vertices[i+2];

            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (z < minZ) minZ = z;
            if (x > maxX) maxX = x;
            if (y > maxY) maxY = y;
            if (z > maxZ) maxZ = z;
        }

        minBounds = glm::vec3(minX, minY, minZ);
        maxBounds = glm::vec3(maxX, maxY, maxZ);
        modelSize = glm::distance(minBounds, maxBounds);
    }

    void draw(Shader& shader) {
        shader.setVec3("objectColor", 0.7f, 0.7f, 0.7f); 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
    }

    void drawExit(Shader& shader) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, exitPosition);
        model = glm::scale(model, glm::vec3(5.0f)); // Make it visible (5x5x5 cube)
        shader.setMat4("model", model);
        shader.setVec3("objectColor", 0.0f, 1.0f, 0.0f); // Green exit
        
        glBindVertexArray(exitVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void setRandomStartAndExit() {
        if (floorTriangles.empty()) return;
        
        // Start: Find floor closest to (0,0,0)
        float minStartDist = std::numeric_limits<float>::max();
        glm::vec3 center(0.0f, 0.0f, 0.0f);
        
        for(const auto& tri : floorTriangles) {
            // Ignore safety floor
            if (tri.centroid.y <= minBounds.y + 1.0f) continue;

            float d = glm::distance(glm::vec2(tri.centroid.x, tri.centroid.z), glm::vec2(center.x, center.z));
            if (d < minStartDist) {
                minStartDist = d;
                startPosition = tri.centroid;
            }
        }
        startPosition.y += 50.0f; // Eye height (standing person)

        // Exit: Find floor closest to (0, 0, maxBounds.z) - The Gate?
        // Or maybe just the furthest point from center?
        // Let's try Max Z edge.
        float minExitDist = std::numeric_limits<float>::max();
        glm::vec3 gateTarget(0.0f, 0.0f, maxBounds.z);
        
        for(const auto& tri : floorTriangles) {
             // Ignore safety floor
            if (tri.centroid.y <= minBounds.y + 1.0f) continue;

            float d = glm::distance(glm::vec2(tri.centroid.x, tri.centroid.z), glm::vec2(gateTarget.x, gateTarget.z));
            if (d < minExitDist) {
                minExitDist = d;
                exitPosition = tri.centroid;
            }
        }
        exitPosition.y += 1.0f;

        std::cout << "Start (Center): " << startPosition.x << " " << startPosition.y << " " << startPosition.z << std::endl;
        std::cout << "Exit (Gate?): " << exitPosition.x << " " << exitPosition.y << " " << exitPosition.z << std::endl;
        
    }

    float getFloorHeight(glm::vec3 pos, bool checkSlope = true) {
        // Find floor triangle directly below/above pos
        // We want the HIGHEST WALKABLE floor at this X,Z position
        
        float bestY = -std::numeric_limits<float>::max(); // Start very low
        bool found = false;
        const float MAX_WALKABLE_SLOPE = 0.5f; // Normal.y must be > 0.5 (angle < ~60°)

        for (const auto& tri : floorTriangles) {
            // Optimization removed: large triangles caused issues
            // if (glm::distance(glm::vec2(pos.x, pos.z), glm::vec2(tri.centroid.x, tri.centroid.z)) > 500.0f) continue;

            // Skip surfaces that are too steep (walls) if slope checking is enabled
            if (checkSlope && tri.normal.y < MAX_WALKABLE_SLOPE) continue;

            // Barycentric coordinates for 2D (X, Z)
            float u, v, w;
            barycentric(glm::vec2(pos.x, pos.z), 
                        glm::vec2(tri.v0.x, tri.v0.z), 
                        glm::vec2(tri.v1.x, tri.v1.z), 
                        glm::vec2(tri.v2.x, tri.v2.z), u, v, w);
            
            if (u >= 0 && v >= 0 && w >= 0) {
                // Inside triangle
                float height = u * tri.v0.y + v * tri.v1.y + w * tri.v2.y;
                
                if (height > bestY) {
                    bestY = height;
                    found = true;
                }
            }
        }
        
        if (!found) return minBounds.y; // Fallback to safety floor level
        return bestY;
    }

    bool checkWallCollision(glm::vec3 position, float radius = 1.0f) {
        for (const auto& tri : wallTriangles) {
            // Relaxed optimization: Check larger radius or remove check for walls
            // Walls can be large, so centroid might be far even if we are close to edge.
            // Let's increase to 500.0f or just remove it for safety.
            // if (glm::distance(position, tri.centroid) > 500.0f) continue; 
            if (checkTriangleCollision(position, radius, tri.v0, tri.v1, tri.v2)) return true;
        }
        return false;
    }

private:
    void barycentric(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c, float &u, float &v, float &w) {
        glm::vec2 v0 = b - a, v1 = c - a, v2 = p - a;
        float d00 = glm::dot(v0, v0);
        float d01 = glm::dot(v0, v1);
        float d11 = glm::dot(v1, v1);
        float d20 = glm::dot(v2, v0);
        float d21 = glm::dot(v2, v1);
        float denom = d00 * d11 - d01 * d01;
        if (std::abs(denom) < 1e-6) { u = -1; v = -1; w = -1; return; }
        v = (d11 * d20 - d01 * d21) / denom;
        w = (d00 * d21 - d01 * d20) / denom;
        u = 1.0f - v - w;
    }

    bool checkTriangleCollision(glm::vec3 sphereCenter, float sphereRadius, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
        glm::vec3 N = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        float dist = glm::dot(sphereCenter - v0, N);
        if (std::abs(dist) > sphereRadius) return false;
        glm::vec3 P = sphereCenter - dist * N;
        
        // Check if P inside triangle
        // 3D Barycentric
        glm::vec3 u = v1 - v0;
        glm::vec3 v = v2 - v0;
        glm::vec3 w = P - v0;
        float uu = glm::dot(u, u);
        float uv = glm::dot(u, v);
        float vv = glm::dot(v, v);
        float wu = glm::dot(w, u);
        float wv = glm::dot(w, v);
        float D = uv * uv - uu * vv;
        if (std::abs(D) < 1e-6) return false;
        float s = (uv * wv - vv * wu) / D;
        float t = (uv * wu - uu * wv) / D;
        return (s >= 0.0f && t >= 0.0f && (s + t) <= 1.0f);
    }
};

#endif
