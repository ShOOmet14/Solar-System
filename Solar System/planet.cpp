#include "Planet.h"
#include <cmath>

#include "stb_image.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace {

float wrapDegrees(float angle) {
    angle = std::fmod(angle, 360.0f);
    return angle < 0.0f ? angle + 360.0f : angle;
}

} // namespace

Planet::Planet(glm::vec3 initialPosition, float visualRadius, glm::vec3 fallbackColor)
    : position(initialPosition), color(fallbackColor), radius(visualRadius) {}

void Planet::draw(unsigned int shaderProgram, unsigned int sphereVao, int sphereIndexCount) const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, glm::radians(selfRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(radius));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));

    const bool hasTexture = textureId != 0;
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), hasTexture ? GL_TRUE : GL_FALSE);
    if (hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }

    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b);

    glBindVertexArray(sphereVao);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Planet::update(float deltaTime, const glm::vec3& orbitCenter) {
    orbitAngle = wrapDegrees(orbitAngle + orbitSpeed * deltaTime);
    selfRotationAngle = wrapDegrees(selfRotationAngle + selfRotationSpeed * deltaTime);

    if (orbitRadius > 0.0f) {
        const float angleRadians = glm::radians(orbitAngle);
        position = orbitCenter + glm::vec3(std::cos(angleRadians) * orbitRadius, 0.0f,
                                           std::sin(angleRadians) * orbitRadius);
    }

    for (Planet& moon : moons) {
        moon.update(deltaTime, position);
    }
}

void Planet::drawMoons(unsigned int shaderProgram, unsigned int sphereVao,
                       int sphereIndexCount) const {
    for (const Planet& moon : moons) {
        moon.draw(shaderProgram, sphereVao, sphereIndexCount);
        moon.drawMoons(shaderProgram, sphereVao, sphereIndexCount);
    }
}

unsigned int loadTexture(const std::filesystem::path& path) {
    int width = 0;
    int height = 0;
    int componentCount = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &componentCount, 0);

    if (data == nullptr) {
        std::cerr << "Failed to load texture '" << path.string() << "': "
                  << (stbi_failure_reason() != nullptr ? stbi_failure_reason()
                                                       : "unknown image error")
                  << '\n';
        return 0;
    }

    GLenum format = 0;
    switch (componentCount) {
    case 1:
        format = GL_RED;
        break;
    case 2:
        format = GL_RG;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        std::cerr << "Unsupported component count in texture '" << path.string()
                  << "': " << componentCount << '\n';
        stbi_image_free(data);
        return 0;
    }

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(format), width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return textureId;
}
