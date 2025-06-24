#include "Planet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern unsigned int VAO, indexCount; // Zdefiniowane w main.cpp

// Konstruktor klasy Planet, przyjmuje parametry: pozycja, promień i kolor
Planet::Planet(glm::vec3 position, float radius, glm::vec3 color)
    : position(position), radius(radius), color(color) {
}

// Funkcja do rysowania planety, przyjmuje parametr: program shaderów
void Planet::draw(unsigned int shaderProgram) const {
	// Macierz modelu do transformacji planety
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, glm::radians(selfRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // ← rotacja własna
    model = glm::scale(model, glm::vec3(radius));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// Ustawienie tekstury, jeśli jest dostępna
    if (textureID != 0) {
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
    }

	// Ustawienie koloru obiektu
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b);

	// Rysowanie planety
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Funkcja do aktualizacji pozycji planety na orbicie, przyjmuje parametr: deltaTime
void Planet::update(float deltaTime) {
    orbitAngle += orbitSpeed * deltaTime;
    selfRotationAngle += selfRotationSpeed * deltaTime;

    float angleRad = glm::radians(orbitAngle);
    position = glm::vec3(cos(angleRad) * orbitRadius, 0.0f, sin(angleRad) * orbitRadius);
}

// Funkcja do aktualizacji pozycji księżyców planety, przyjmuje parametr: deltaTime
void Planet::updateMoons(float deltaTime) {
    for (auto& moon : moons) {
        moon.orbitAngle += moon.orbitSpeed * deltaTime;
        float angleRad = glm::radians(moon.orbitAngle);
        float x = cos(angleRad) * moon.orbitRadius;
        float z = sin(angleRad) * moon.orbitRadius;
        moon.position = position + glm::vec3(x, 0.0f, z); // orbituje wokół planety
    }
}

// Funkcja do rysowania księżyców planety, przyjmuje parametr: program shaderów
void Planet::drawMoons(unsigned int shaderProgram) const {
    for (const auto& moon : moons) {
        moon.draw(shaderProgram);
    }
}

// Funkcja do ładowania tekstury, przyjmuje parametr: ścieżka do pliku tekstury
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

	// Ładowanie tekstury z pliku
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	// Sprawdzenie, czy tekstura została poprawnie załadowana
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID); // wiązanie tekstury
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // ustawienie danych tekstury
		glGenerateMipmap(GL_TEXTURE_2D); // generowanie mipmap

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