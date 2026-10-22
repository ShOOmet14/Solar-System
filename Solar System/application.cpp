#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Planet.h"
#include "planets_setup.h"

namespace {

constexpr int initialWindowWidth = 1280;
constexpr int initialWindowHeight = 720;
constexpr int sphereSectorCount = 36;
constexpr int sphereStackCount = 18;
constexpr int orbitSegmentCount = 160;
constexpr float pi = 3.14159265358979323846f;
constexpr float normalCameraSpeed = 5.0f;
constexpr float fastCameraSpeed = 15.0f;
constexpr float mouseSensitivity = 0.1f;
constexpr float minimumFieldOfView = 15.0f;
constexpr float maximumFieldOfView = 80.0f;
constexpr float maximumFrameStep = 0.1f;

struct IndexedMesh {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    int indexCount = 0;
};

struct LineMesh {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    int vertexCount = 0;
};

glm::vec3 cameraPosition(0.0f, 8.0f, 20.0f);
glm::vec3 cameraFront = glm::normalize(-cameraPosition);
constexpr glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float cameraYaw = -90.0f;
float cameraPitch = -21.8f;
double lastMouseX = initialWindowWidth / 2.0;
double lastMouseY = initialWindowHeight / 2.0;
bool isFirstMouseEvent = true;
float fieldOfView = 45.0f;

constexpr const char* vertexShaderSource = R"glsl(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoordinate;

out vec2 fragmentTextureCoordinate;
out vec3 fragmentPosition;
out vec3 fragmentNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fragmentPosition = vec3(model * vec4(position, 1.0));
    fragmentNormal = mat3(transpose(inverse(model))) * normal;
    fragmentTextureCoordinate = textureCoordinate;
    gl_Position = projection * view * vec4(fragmentPosition, 1.0);
}
)glsl";

constexpr const char* fragmentShaderSource = R"glsl(
#version 330 core

in vec2 fragmentTextureCoordinate;
in vec3 fragmentPosition;
in vec3 fragmentNormal;

out vec4 fragmentColor;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float emissiveStrength;
uniform sampler2D textureSampler;
uniform bool useTexture;

void main() {
    const float ambientStrength = 0.16;
    vec3 baseColor = useTexture
        ? texture(textureSampler, fragmentTextureCoordinate).rgb
        : objectColor;

    float normalLength = length(fragmentNormal);
    vec3 normalizedNormal = normalLength > 0.0
        ? fragmentNormal / normalLength
        : vec3(0.0);
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    float diffuseStrength = max(dot(normalizedNormal, lightDirection), 0.0);

    vec3 ambient = ambientStrength * lightColor * baseColor;
    vec3 diffuse = diffuseStrength * lightColor * baseColor;
    vec3 emissive = emissiveStrength * baseColor;
    fragmentColor = vec4(ambient + diffuse + emissive, 1.0);
}
)glsl";

void glfwErrorCallback(int errorCode, const char* description) {
    std::cerr << "GLFW error " << errorCode << ": " << description << '\n';
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow*, double xPosition, double yPosition) {
    if (isFirstMouseEvent) {
        lastMouseX = xPosition;
        lastMouseY = yPosition;
        isFirstMouseEvent = false;
    }

    const float xOffset = static_cast<float>(xPosition - lastMouseX) * mouseSensitivity;
    const float yOffset = static_cast<float>(lastMouseY - yPosition) * mouseSensitivity;
    lastMouseX = xPosition;
    lastMouseY = yPosition;

    cameraYaw += xOffset;
    cameraPitch = std::clamp(cameraPitch + yOffset, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(cameraYaw)) * std::cos(glm::radians(cameraPitch));
    direction.y = std::sin(glm::radians(cameraPitch));
    direction.z = std::sin(glm::radians(cameraYaw)) * std::cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(direction);
}

void scrollCallback(GLFWwindow*, double, double yOffset) {
    fieldOfView = std::clamp(fieldOfView - static_cast<float>(yOffset), minimumFieldOfView,
                             maximumFieldOfView);
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const float baseSpeed =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? fastCameraSpeed : normalCameraSpeed;
    const float movement = baseSpeed * deltaTime;
    const glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPosition += movement * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPosition -= movement * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPosition -= movement * right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosition += movement * right;
    }
}

unsigned int compileShader(unsigned int type, const char* source, const char* name) {
    const unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int compilationSucceeded = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilationSucceeded);
    if (compilationSucceeded == GL_TRUE) {
        return shader;
    }

    int logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(std::max(logLength, 1)), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    std::cerr << "Failed to compile " << name << " shader:\n" << log << '\n';
    glDeleteShader(shader);
    return 0;
}

unsigned int createShaderProgram() {
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource, "vertex");
    if (vertexShader == 0) {
        return 0;
    }

    const unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource, "fragment");
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int linkingSucceeded = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkingSucceeded);
    if (linkingSucceeded == GL_TRUE) {
        return program;
    }

    int logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<std::size_t>(std::max(logLength, 1)), '\0');
    glGetProgramInfoLog(program, logLength, nullptr, log.data());
    std::cerr << "Failed to link shader program:\n" << log << '\n';
    glDeleteProgram(program);
    return 0;
}

IndexedMesh createSphereMesh() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int stack = 0; stack <= sphereStackCount; ++stack) {
        const float stackAngle = pi / 2.0f - static_cast<float>(stack) * pi / sphereStackCount;
        const float xy = std::cos(stackAngle);
        const float z = std::sin(stackAngle);

        for (int sector = 0; sector <= sphereSectorCount; ++sector) {
            const float sectorAngle = static_cast<float>(sector) * 2.0f * pi / sphereSectorCount;
            const float x = xy * std::cos(sectorAngle);
            const float y = xy * std::sin(sectorAngle);

            vertices.insert(vertices.end(), {
                                                x,
                                                y,
                                                z,
                                                x,
                                                y,
                                                z,
                                                static_cast<float>(sector) / sphereSectorCount,
                                                1.0f - static_cast<float>(stack) / sphereStackCount,
                                            });
        }
    }

    for (int stack = 0; stack < sphereStackCount; ++stack) {
        int first = stack * (sphereSectorCount + 1);
        int second = first + sphereSectorCount + 1;

        for (int sector = 0; sector < sphereSectorCount; ++sector) {
            indices.push_back(static_cast<unsigned int>(first));
            indices.push_back(static_cast<unsigned int>(second));
            indices.push_back(static_cast<unsigned int>(first + 1));
            indices.push_back(static_cast<unsigned int>(first + 1));
            indices.push_back(static_cast<unsigned int>(second));
            indices.push_back(static_cast<unsigned int>(second + 1));
            ++first;
            ++second;
        }
    }

    IndexedMesh mesh;
    mesh.indexCount = static_cast<int>(indices.size());
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(),
                 GL_STATIC_DRAW);

    constexpr int stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return mesh;
}

LineMesh createOrbitMesh() {
    std::vector<float> vertices;
    vertices.reserve(static_cast<std::size_t>(orbitSegmentCount) * 3);

    for (int segment = 0; segment < orbitSegmentCount; ++segment) {
        const float angle = 2.0f * pi * static_cast<float>(segment) / orbitSegmentCount;
        vertices.push_back(std::cos(angle));
        vertices.push_back(0.0f);
        vertices.push_back(std::sin(angle));
    }

    LineMesh mesh;
    mesh.vertexCount = orbitSegmentCount;
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return mesh;
}

void drawOrbit(unsigned int shaderProgram, const LineMesh& orbitMesh, float radius) {
    const glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glBindVertexArray(orbitMesh.vao);
    glDrawArrays(GL_LINE_LOOP, 0, orbitMesh.vertexCount);
    glBindVertexArray(0);
}

void collectTextureIds(const Planet& planet, std::unordered_set<unsigned int>& textureIds) {
    if (planet.textureId != 0) {
        textureIds.insert(planet.textureId);
    }
    for (const Planet& moon : planet.moons) {
        collectTextureIds(moon, textureIds);
    }
}

void deleteTextures(const std::vector<Planet>& planets) {
    std::unordered_set<unsigned int> uniqueTextureIds;
    for (const Planet& planet : planets) {
        collectTextureIds(planet, uniqueTextureIds);
    }

    const std::vector<unsigned int> textureIds(uniqueTextureIds.begin(), uniqueTextureIds.end());
    if (!textureIds.empty()) {
        glDeleteTextures(static_cast<int>(textureIds.size()), textureIds.data());
    }
}

void deleteMesh(IndexedMesh& mesh) {
    glDeleteBuffers(1, &mesh.ebo);
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteVertexArrays(1, &mesh.vao);
    mesh = {};
}

void deleteMesh(LineMesh& mesh) {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteVertexArrays(1, &mesh.vao);
    mesh = {};
}

} // namespace

int main() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(initialWindowWidth, initialWindowHeight,
                                          "Solar System Visualization", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create the GLFW window.\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    const unsigned int shaderProgram = createShaderProgram();
    if (shaderProgram == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    IndexedMesh sphereMesh = createSphereMesh();
    LineMesh orbitMesh = createOrbitMesh();

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);

    std::vector<Planet> planets;
    initializePlanets(planets);

    float lastFrameTime = static_cast<float>(glfwGetTime());
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        const float currentFrameTime = static_cast<float>(glfwGetTime());
        const float deltaTime =
            std::clamp(currentFrameTime - lastFrameTime, 0.0f, maximumFrameStep);
        lastFrameTime = currentFrameTime;

        processInput(window, deltaTime);
        for (Planet& planet : planets) {
            planet.update(deltaTime);
        }

        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0) {
            glfwWaitEvents();
            continue;
        }

        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glClearColor(0.005f, 0.008f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        const glm::mat4 projection = glm::perspective(glm::radians(fieldOfView),
                                                      static_cast<float>(framebufferWidth) /
                                                          static_cast<float>(framebufferHeight),
                                                      0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPosition"),
                    planets.front().position.x, planets.front().position.y,
                    planets.front().position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), GL_FALSE);
        glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), 0.65f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.24f, 0.28f, 0.36f);
        for (std::size_t index = 1; index < planets.size(); ++index) {
            drawOrbit(shaderProgram, orbitMesh, planets[index].orbitRadius);
        }

        for (std::size_t index = 0; index < planets.size(); ++index) {
            glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"),
                        index == 0 ? 0.85f : 0.0f);
            planets[index].draw(shaderProgram, sphereMesh.vao, sphereMesh.indexCount);
            glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), 0.0f);
            planets[index].drawMoons(shaderProgram, sphereMesh.vao, sphereMesh.indexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    deleteTextures(planets);
    deleteMesh(orbitMesh);
    deleteMesh(sphereMesh);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
