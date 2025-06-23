#include "ring.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern unsigned int ringVAO, ringVBO;

Ring::Ring(glm::vec3 center, float innerRadius, float outerRadius, glm::vec3 color)
    : center(center), innerRadius(innerRadius), outerRadius(outerRadius), color(color) {
}

void Ring::draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection) const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0)); // ustaw płasko w XZ

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), 0.0f);

    glBindVertexArray(ringVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 200); // 100 punktów × 2 (wewnętrzny + zewnętrzny)
    glBindVertexArray(0);
}
