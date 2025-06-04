#include <glad/glad.h> // library that supports finding OpenGL functions, that are dependent on the driver manufacturer
#include <GLFW/glfw3.h> // openGL supporting library
#include <iostream>
#include <vector>

#ifndef M_PI
#   define M_PI 3.1415926535897932384626433832
#endif

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void initializeShader();

unsigned int VAO, shaderProgram;
float screenHeight = 600.0f;
float screenWidth = 800.0f;
int resolution = 100;

// GLSL soure code for basic vertex shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    void main()
    {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
 )";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    void main(){
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";

int main()
{
    // initialization of my window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // initialiaztion of GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD" << std::endl;
        return - 1;
    }

    initializeShader();

    while (!glfwWindowShouldClose(window)) {
        // input
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT);

        // render
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, resolution + 1);

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
}

void initializeShader() {
    // vertex shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shaders
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::vector<float> vertices;

    float centerX = screenWidth/2.0f;
    float centerY = screenHeight/2.0f;
    float centerZ = 0;
    float radius = 50.0f;
    float angle, x, y, z;

    for (int i = 0; i <= resolution; i++) {
        angle = 2.0f * M_PI * (static_cast<float>(i) / resolution);
        x = (centerX + cos(angle) * radius) / (screenWidth / 2.0f) - 1.0f;
        y = (centerY + sin(angle) * radius) / (screenHeight / 2.0f) - 1.0f;
        z = 0.0f;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    unsigned int VBO; // vertex buffer objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}