#pragma once
#include <filesystem>
#include <vector>

#include <glm/glm.hpp>

unsigned int loadTexture(const std::filesystem::path& path);

class Planet {
  public:
    Planet(glm::vec3 position, float radius, glm::vec3 color);

    void update(float deltaTime, const glm::vec3& orbitCenter = glm::vec3(0.0f));
    void draw(unsigned int shaderProgram, unsigned int sphereVao, int sphereIndexCount) const;
    void drawMoons(unsigned int shaderProgram, unsigned int sphereVao, int sphereIndexCount) const;

    glm::vec3 position;
    glm::vec3 color;
    float radius;
    float selfRotationAngle = 0.0f;
    float selfRotationSpeed = 20.0f;
    unsigned int textureId = 0;

    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
    float orbitAngle = 0.0f;

    std::vector<Planet> moons;
};
