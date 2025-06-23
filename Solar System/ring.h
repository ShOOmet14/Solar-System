#pragma once
#include <glm/glm.hpp>

class Ring {
public:
    glm::vec3 center;           // Środek pierścienia (taka sama jak pozycja planety)
    float innerRadius;          // Promień wewnętrzny pierścienia
    float outerRadius;          // Promień zewnętrzny pierścienia
    glm::vec3 color;            // Kolor pierścienia

    Ring(glm::vec3 center, float innerRadius, float outerRadius, glm::vec3 color);
    void draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection) const;
};