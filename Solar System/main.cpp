#include <glad/glad.h> // library that supports finding OpenGL functions, that are dependent on the driver manufacturer
#include <GLFW/glfw3.h> // openGL supporting library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "planet.h"

#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void initializeShader();
void drawOrbit(float radius, const glm::mat4& view, const glm::mat4& projection);

unsigned int VAO, shaderProgram;
unsigned int orbitVAO, orbitVBO;
unsigned int indexCount;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 7.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f; // zaczynamy od patrzenia w -Z
float pitch = 0.0f;
int width, height;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;
float fov = 45.0f;

// GLSL soure code for basic vertex shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;
    
    out vec2 TexCoords;
    out vec3 FragPos;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;  

    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
        TexCoords = aTexCoord;
    }
)";


const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    in vec2 TexCoords;

    out vec4 FragColor;

    uniform vec3 lightPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    uniform float emissiveStrength;
    uniform sampler2D texture1;
    uniform bool useTexture;

    void main() {
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDirN = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDirN), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 baseColor = useTexture ? texture(texture1, TexCoords).rgb : objectColor;

        vec3 emissive = emissiveStrength * baseColor;
        vec3 result = ambient + diffuse + emissive;

        FragColor = vec4(result * baseColor, 1.0);
    }
)";


int main()
{
    // initialization of my window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // set the viewport
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwPollEvents(); // wykonaj raz przed pętlą, żeby ustawić pozycję kursora
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // initialiaztion of GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD" << std::endl;
        return -1;
    }

    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    initializeShader();
    glEnable(GL_DEPTH_TEST);

    std::vector<Planet> planets;

    // Słońce (pozycja w centrum, bez prędkości)
    Planet sun(glm::vec3(0.0f), 0.7f, glm::vec3(1.0f, 1.0f, 0.0f));
    planets.push_back(sun);

    planets.emplace_back(glm::vec3(0.0f), 0.10f, glm::vec3(0.5f));                  // Merkury
    planets.emplace_back(glm::vec3(0.0f), 0.18f, glm::vec3(0.9f, 0.7f, 0.2f));      // Wenus

    planets.emplace_back(glm::vec3(0.0f), 0.22f, glm::vec3(0.2f, 0.6f, 1.0f));      // Ziemia
    Planet moon(glm::vec3(0.0f), 0.05f, glm::vec3(0.8f, 0.8f, 0.8f));
    moon.orbitRadius = 0.5f;
    moon.orbitSpeed = 100.0f;
    planets[3].moons.push_back(moon); // Ziemia jest na pozycji 3

    planets.emplace_back(glm::vec3(0.0f), 0.15f, glm::vec3(0.8f, 0.3f, 0.2f));      // Mars
    Planet phobos(glm::vec3(0.0f), 0.03f, glm::vec3(0.6f));
    phobos.orbitRadius = 0.3f;
    phobos.orbitSpeed = 120.0f;

    Planet deimos(glm::vec3(0.0f), 0.02f, glm::vec3(0.7f));
    deimos.orbitRadius = 0.5f;
    deimos.orbitSpeed = 90.0f;

    planets[4].moons.push_back(phobos);
    planets[4].moons.push_back(deimos);

    planets.emplace_back(glm::vec3(0.0f), 0.45f, glm::vec3(0.9f, 0.8f, 0.6f));      // Jowisz 4 największe księżyce
    float jMoonSize = 0.05f;
    planets[5].moons.push_back(Planet(glm::vec3(0.0f), jMoonSize, glm::vec3(0.9f, 0.6f, 0.3f))); // Io
    planets[5].moons.back().orbitRadius = 0.7f;
    planets[5].moons.back().orbitSpeed = 110.0f;

    planets[5].moons.push_back(Planet(glm::vec3(0.0f), jMoonSize, glm::vec3(0.6f, 0.8f, 1.0f))); // Europa
    planets[5].moons.back().orbitRadius = 0.9f;
    planets[5].moons.back().orbitSpeed = 100.0f;

    planets[5].moons.push_back(Planet(glm::vec3(0.0f), jMoonSize, glm::vec3(0.4f, 0.7f, 0.9f))); // Ganimedes
    planets[5].moons.back().orbitRadius = 1.2f;
    planets[5].moons.back().orbitSpeed = 90.0f;

    planets[5].moons.push_back(Planet(glm::vec3(0.0f), jMoonSize, glm::vec3(0.6f, 0.5f, 0.4f))); // Kallisto
    planets[5].moons.back().orbitRadius = 1.5f;
    planets[5].moons.back().orbitSpeed = 80.0f;

    planets.emplace_back(glm::vec3(0.0f), 0.40f, glm::vec3(0.9f, 0.85f, 0.5f));     // Saturn
    Planet tytan(glm::vec3(0.0f), 0.06f, glm::vec3(0.8f, 0.7f, 0.4f));
    tytan.orbitRadius = 1.0f;
    tytan.orbitSpeed = 85.0f;
    planets[6].moons.push_back(tytan);

    planets.emplace_back(glm::vec3(0.0f), 0.30f, glm::vec3(0.6f, 0.9f, 0.9f));      // Uran
    Planet miranda(glm::vec3(0.0f), 0.03f, glm::vec3(0.6f, 0.6f, 0.8f));
    miranda.orbitRadius = 0.8f;
    miranda.orbitSpeed = 90.0f;
    planets[7].moons.push_back(miranda);

    planets.emplace_back(glm::vec3(0.0f), 0.28f, glm::vec3(0.4f, 0.5f, 0.9f));      // Neptun
    Planet tryton(glm::vec3(0.0f), 0.04f, glm::vec3(0.5f, 0.7f, 0.9f));
    tryton.orbitRadius = 0.7f;
    tryton.orbitSpeed = 95.0f;
    planets[8].moons.push_back(tryton);

    // Ustaw parametry orbity dla planet (nie słońca)
    planets[1].orbitRadius = 2.5f;  planets[1].orbitSpeed = 60.0f;  // Merkury
    planets[2].orbitRadius = 3.5f;  planets[2].orbitSpeed = 45.0f;  // Wenus
    planets[3].orbitRadius = 5.0f;  planets[3].orbitSpeed = 35.0f;  // Ziemia
    planets[4].orbitRadius = 6.5f;  planets[4].orbitSpeed = 28.0f;  // Mars
    planets[5].orbitRadius = 8.5f;  planets[5].orbitSpeed = 18.0f;  // Jowisz
    planets[6].orbitRadius = 10.5f; planets[6].orbitSpeed = 14.0f;  // Saturn
    planets[7].orbitRadius = 12.0f; planets[7].orbitSpeed = 10.0f;  // Uran
    planets[8].orbitRadius = 13.5f; planets[8].orbitSpeed = 8.0f;   // Neptun

    planets[0].textureID = loadTexture("resources/sun.jpg");
    planets[1].textureID = loadTexture("resources/mercury.jpg");
    planets[2].textureID = loadTexture("resources/venus.jpg");
    planets[3].textureID = loadTexture("resources/earth.jpg");
    planets[4].textureID = loadTexture("resources/mars.jpg");
    planets[5].textureID = loadTexture("resources/mercury.jpg");
    planets[6].textureID = loadTexture("resources/venus.jpg");
    planets[7].textureID = loadTexture("resources/earth.jpg");
    planets[8].textureID = loadTexture("resources/mars.jpg");

    while (!glfwWindowShouldClose(window)) {
        // input
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        for (size_t i = 1; i < planets.size(); ++i) {
            planets[i].orbitAngle += planets[i].orbitSpeed * deltaTime;
            float angleRad = glm::radians(planets[i].orbitAngle);
            float x = cos(angleRad) * planets[i].orbitRadius;
            float z = sin(angleRad) * planets[i].orbitRadius;
            planets[i].position = glm::vec3(x, 0.0f, z);

            // Aktualizacja księżyców
            planets[i].updateMoons(deltaTime);
        }

        lastFrame = currentFrame;
        glfwGetFramebufferSize(window, &width, &height);

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);

        // teraz można wywołać drawOrbit
        for (size_t i = 1; i < planets.size(); ++i) {
            drawOrbit(planets[i].orbitRadius, view, projection);
        }

        glm::mat4 model = glm::mat4(1.0f);


        // pobierz lokalizacje
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        // ustaw uniformy view/projection tylko raz:
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // ustaw światło globalnie
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"),
            planets[0].position.x,
            planets[0].position.y,
            planets[0].position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // Rysuj Słońce jako świecące
        glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), 1.0f);
        planets[0].draw(shaderProgram);

        // Rysuj pozostałe planety
        glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), 0.0f);
        for (size_t i = 1; i < planets.size(); ++i) {
            if (planets[i].textureID != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, planets[i].textureID);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
            }
            else {
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
            }
            planets[i].draw(shaderProgram);
            planets[i].drawMoons(shaderProgram); // ← rysuj księżyce
        }

        // check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float baseSpeed = 5.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        baseSpeed = 15.0f;

    float cameraSpeed = baseSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

}

void initializeShader() {
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    {
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    {
        int success;
        char infoLog[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::vector<float> vertices;
    const int sectorCount = 36; // longitude
    const int stackCount = 18;  // latitude
    const float radius = 1.0f;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = M_PI / 2 - i * M_PI / stackCount;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * M_PI / sectorCount;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            // vertex position + normal + tex coords
            vertices.push_back(x);  // pos x
            vertices.push_back(y);  // pos y
            vertices.push_back(z);  // pos z

            vertices.push_back(nx); // normal x
            vertices.push_back(ny); // normal y
            vertices.push_back(nz); // normal z

            float u = (float)j / sectorCount;
            float v = (float)i / stackCount;
            vertices.push_back(u);  // tex u
            vertices.push_back(v);  // tex v
        }
    }

    std::vector<unsigned int> indices;
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }

    indexCount = indices.size();

    unsigned int VBO; // vertex buffer objects
    unsigned int EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Ustaw stride = 8 floatów (3 pozycja, 3 normalne, 2 tex coords)
    int stride = 8;

    // Pozycja: aPos - location = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normalny: aNormal - location = 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Tekstura: aTexCoord - location = 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glBindVertexArray(0);

    // Inicjalizacja orbit
    std::vector<float> orbitVertices;
    const int segments = 100;

    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = cos(angle);
        float z = sin(angle);
        orbitVertices.push_back(x);
        orbitVertices.push_back(0.0f);
        orbitVertices.push_back(z);
    }

    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(float), orbitVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // odwrócone: góra = większy pitch
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // ogranicz pitch, żeby kamera się nie przewróciła
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

void drawOrbit(float radius, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(orbitVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 100);
    glBindVertexArray(0);
}