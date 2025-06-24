#pragma once
#include <glm/glm.hpp>
#include <vector>

unsigned int loadTexture(const char* path);

class Planet {
public:
	// Parametry planety
    glm::vec3 position;
    glm::vec3 color;
    float radius;
    float selfRotationAngle = 0.0f;  // Kąt obrotu wokół własnej osi
    float selfRotationSpeed = 20.0f; // Stopnie na sekundę
    unsigned int textureID = 0;

    // Parametry orbity
    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
    float orbitAngle = 0.0f;

	Planet(glm::vec3 position, float radius, glm::vec3 color);

	// Funkcje do rysowania i aktualizacji planety
    void update(float deltaTime);
    void draw(unsigned int shaderProgram) const;

	// Funkcje do rysowania i aktualizacji księżyców
    std::vector<Planet> moons;
    void updateMoons(float deltaTime);
    void drawMoons(unsigned int shaderProgram) const;
};