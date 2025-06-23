#include "Planet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

    if (textureID != 0) {
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
    }

    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
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

unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
