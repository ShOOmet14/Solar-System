#pragma once
#include <glm/glm.hpp>
#include <vector>

class Planet {
public:
    glm::vec3 position;
    glm::vec3 color;
    float radius;

    // Parametry orbity
    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
    float orbitAngle = 0.0f;

    Planet(glm::vec3 position, float radius, glm::vec3 color);

    void update(float deltaTime);
    void draw(unsigned int shaderProgram) const;

    std::vector<Planet> moons; // Tablica wektorowa księżyców

    void updateMoons(float deltaTime);
    void drawMoons(unsigned int shaderProgram) const;
};