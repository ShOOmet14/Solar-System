#include <glad/glad.h> //biblioteka obsługująca odnajdywanie funkcji OpenGL, jest zależne od dostawcy sterowników
#include <GLFW/glfw3.h> //biblioteka obsługująca OpenGL

int main()
{
    //inicjalizowanie okna
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    return 0;
}