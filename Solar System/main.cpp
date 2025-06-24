#include <glad/glad.h> // Biblioteka która pomaga znaleźć funkcje OpenGL, których miejsce zależne jest od sterowników
#include <GLFW/glfw3.h> // Główna biblioteka OpenGL, tworzenie okna i obsługa klawiatury i myszy
#include <glm/glm.hpp> // Biblioteka matematyczna 3D OpenGL, zawiera macierze, transformacje i wektory
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include "planet.h"
#include "planets_setup.h"

#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif

// Funkcje callback i pomocnicze
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // Funkcja callback, która ustawia rozmiar okna
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // Funkcja callback, która obsługuje ruch myszy
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // Funkcja callback, która obsługuje przewijanie myszy
void processInput(GLFWwindow* window); // Funkcja do przetwarzania wejścia z klawiatury
void initializeShader();
void drawOrbit(float radius, const glm::mat4& view, const glm::mat4& projection);

// Globalne zmienne OpenGL
unsigned int VAO, shaderProgram;
unsigned int orbitVAO, orbitVBO;
unsigned int indexCount;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Globalne zmienne kamery
glm::vec3 cameraPos = glm::vec3(0.0f, 7.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f; // Kąt obrotu kamery w poziomie
float pitch = 0.0f; // Kąt obrotu kamery w pionie
float lastX = 0.0f;
float lastY = 0.0f;
bool firstMouse = true;
float fov = 45.0f;

// Vertex shader - definiuje wierzchołki i ich atrybuty
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

// Fragment shader - oblicza kolor fragmentu na podstawie światła i tekstury
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
    int width, height;

    // Inicjalizacja okna i OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor); // Użyj rozdzielczości monitora jako domyślnej

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwPollEvents();

	// Pobierz rozmiar okna i ustaw lastX/lastY pozycji myszy
    glfwGetFramebufferSize(window, &width, &height);
    lastX = width / 2.0f;
    lastY = height / 2.0f;

    // Inicjalizacja GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD" << std::endl;
        return -1;
    }

    initializeShader();
	glEnable(GL_DEPTH_TEST); // Włącz test głębokości, aby poprawnie rysować obiekty 3D

	// Planety i tekstury
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    std::vector<Planet> planets;
    initializePlanets(planets);

    // Pętla główna renderująca
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

		// Aktualizacja pozycji planet i ich księżyców
        for (size_t i = 1; i < planets.size(); ++i) {
            planets[i].orbitAngle += planets[i].orbitSpeed * deltaTime;
            float angleRad = glm::radians(planets[i].orbitAngle);
            float x = cos(angleRad) * planets[i].orbitRadius;
            float z = sin(angleRad) * planets[i].orbitRadius;
            planets[i].position = glm::vec3(x, 0.0f, z);
            planets[i].updateMoons(deltaTime);
        }

        lastFrame = currentFrame;
		glfwGetFramebufferSize(window, &width, &height); // Dynamczznie pobierz rozmiar okna

		processInput(window); // Przetwarzanie wejścia z klawiatury
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f);

        // Ustaw uniformy view/projection tylko raz
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view)); // macierz widoku
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); // macierz projekcji

        // Ustaw światło globalnie
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"),
            planets[0].position.x,
            planets[0].position.y,
            planets[0].position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // Rysuj orbity
        for (size_t i = 1; i < planets.size(); ++i) {
            drawOrbit(planets[i].orbitRadius, view, projection);
        }

        // Rysuj Słońce
        glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), 1.0f);
        if (planets[0].textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planets[0].textureID);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
        }
        else {
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
        }
        planets[0].draw(shaderProgram);

        // Rysuj pozostałe planety i ich księżyce
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

            // Księżyce
            for (const auto& moon : planets[i].moons) {
                if (moon.textureID != 0) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, moon.textureID);
                    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
                }
                else {
                    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
                }
                moon.draw(shaderProgram);
            }
        }

		glfwSwapBuffers(window); // Wymiana buforów, aby wyświetlić narysowane obiekty
        glfwPollEvents();
    }

    glfwTerminate();
}

// Funkcja callback, która ustawia rozmiar okna, przyjmuje następujące parametry: okno, szerokość i wysokość
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Funkcja do przetwarzania wejścia z klawiatury, przyjmuje następujące parametry: okno
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

// Inicjalizuje shadery i tworzy VAO/VBO dla sfery
void initializeShader() {
    // Vertex shader, który przekształca wierzchołki i oblicza normalne
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

	// Fragment shader, który oblicza kolor fragmentu na podstawie światła i tekstury
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

	// Tworzenie programu shaderów, do którego dołączamy vertex i fragment shader
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
	
    // Lokalne sprawdzanie błędów linkowania programu shaderów
    {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }

	// Usuwanie shaderów, które zostały dołączone do programu
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

	// Inicjalizacja VAO/VBO dla sfery
    std::vector<float> vertices;
	const int sectorCount = 36; // liczba sektorów (kółek) w sferze
	const int stackCount = 18; // liczba stosów (półkul) w sferze
	const float radius = 1.0f; // promień sfery

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

			// Dodaj wierzchołki do wektora
            vertices.push_back(x);  // pozycja x
            vertices.push_back(y);  // pozycja y
            vertices.push_back(z);  // pozycja z

			vertices.push_back(nx); // normalny x
            vertices.push_back(ny); // normalny y
            vertices.push_back(nz); // normalny z

            float u = (float)j / sectorCount;
            float v = 1.0f - (float)i / stackCount;
			vertices.push_back(u); // tekstura u
			vertices.push_back(v); // tekstura v
        }
    }

	// Tworzenie indeksów dla sfery, które będą używane do rysowania trójkątów
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

	// Tworzenie VAO, VBO i EBO dla sfery
	unsigned int VBO; // Vertex Buffer Object
	unsigned int EBO; // Element Buffer Object
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

	// Inicjalizacja orbit, tworzenie VAO i VBO dla linii okręgu
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

// Funkcja callback, która obsługuje ruch myszy, przyjmuje parametry: okno, pozycja x i y myszy
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

	// Oblicz przesunięcie myszy
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

	// Ustaw czułość myszy, aby kontrolować szybkość obrotu kamery
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

	// Aktualizuj kąty kamery na podstawie przesunięcia myszy
    yaw += xoffset;
    pitch += yoffset;

    // Ogranicz pitch, żeby kamera się nie przewróciła
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

// Funkcja callback, która obsługuje przewijanie myszy, przyjmuje parametry: okno, przesunięcie w poziomie i pionie
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

// Funkcja, która rysuje orbitę planety jako linię okręgu, przyjmuje parametry: promień orbity, macierz widoku i macierz projekcji
void drawOrbit(float radius, const glm::mat4& view, const glm::mat4& projection) {
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(radius)); // macierz modelu dla orbity
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(orbitVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 100);
    glBindVertexArray(0);
}