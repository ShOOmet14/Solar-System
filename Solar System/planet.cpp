#include "Planet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <iostream>
extern unsigned int VAO, indexCount; // zdefiniowane w main.cpp

Planet::Planet(glm::vec3 position, float radius, glm::vec3 color)
    : position(position), radius(radius), color(color) {
}

void Planet::draw(unsigned int shaderProgram) const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(radius));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void Planet::update(float deltaTime) {
    orbitAngle += orbitSpeed * deltaTime;
    float angleRad = glm::radians(orbitAngle);
    position = glm::vec3(cos(angleRad) * orbitRadius, 0.0f, sin(angleRad) * orbitRadius);
}

void Planet::updateMoons(float deltaTime) {
    for (auto& moon : moons) {
        moon.orbitAngle += moon.orbitSpeed * deltaTime;
        float angleRad = glm::radians(moon.orbitAngle);
        float x = cos(angleRad) * moon.orbitRadius;
        float z = sin(angleRad) * moon.orbitRadius;
        moon.position = position + glm::vec3(x, 0.0f, z); // orbituje wokół planety
    }
}

void Planet::drawMoons(unsigned int shaderProgram) const {
    for (const auto& moon : moons) {
        moon.draw(shaderProgram);
    }
}