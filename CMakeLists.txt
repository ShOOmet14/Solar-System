cmake_minimum_required(VERSION 3.24)

project(SolarSystemVisualization LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw glm)

find_package(OpenGL REQUIRED)

add_library(glad STATIC
    third_party/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    third_party/glad/include
)

add_library(stb_image STATIC
    "Solar System/stb_image.cpp"
)
target_include_directories(stb_image PUBLIC
    "Solar System"
)

add_executable(solar_system
    "Solar System/application.cpp"
    "Solar System/planet.cpp"
    "Solar System/Planet.h"
    "Solar System/planets_setup.cpp"
    "Solar System/planets_setup.h"
)

target_link_libraries(solar_system PRIVATE
    glad
    glfw
    glm::glm
    OpenGL::GL
    stb_image
)

if(MSVC)
    target_compile_options(solar_system PRIVATE /W4 /permissive- /utf-8)
    target_compile_options(glad PRIVATE /W0)
    target_compile_options(stb_image PRIVATE /W0)
endif()

set_target_properties(solar_system PROPERTIES
    OUTPUT_NAME "SolarSystem"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:solar_system>"
)

add_custom_command(TARGET solar_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Solar System/resources"
        "$<TARGET_FILE_DIR:solar_system>/resources"
    COMMENT "Copying runtime textures"
)
