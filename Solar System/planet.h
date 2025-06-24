#pragma once
#include <glm/glm.hpp>
#include <vector>

unsigned int loadTexture(const char* path);

class Planet {
public:
    glm::vec3 position;
    glm::vec3 color;
    float radius;

    // Parametry orbity
    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
    float orbitAngle = 0.0f;
    float selfRotationAngle = 0.0f;  // Kąt obrotu wokół własnej osi
    float selfRotationSpeed = 20.0f; // Stopnie na sekundę

    unsigned int textureID = 0;

    Planet(glm::vec3 position, float radius, glm::vec3 color);

    void update(float deltaTime); // Aktualizuje pozycję planety na orbicie
    void draw(unsigned int shaderProgram) const;

    std::vector<Planet> moons; // Tablica wektorowa księżyców

    void updateMoons(float deltaTime);
    void drawMoons(unsigned int shaderProgram) const;
};