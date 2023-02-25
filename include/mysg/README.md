```cmake
cmake_minimum_required(VERSION 3.10)

project(SmileBox)
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ext")
option(ENABLE_FREETYPE "Enable support of freetype." OFF)
option(COPY_ASSETS "Copy assets/ folder into build directory" ON)

set(EXECUTABLE ${PROJECT_NAME})
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
aux_source_directory(${SRC_DIR} SOURCE_FILES)
add_executable(${EXECUTABLE} ${SOURCE_FILES})
set_property(TARGET ${EXECUTABLE} PROPERTY CXX_STANDARD 17)
target_include_directories(${EXECUTABLE} PRIVATE "${INC_DIR}")

# External Libraries
set(FREETYPE_DIR "${EXTERNAL_DIR}/freetype-2.12.1")
set(GLAD_DIR "${EXTERNAL_DIR}/glad-0.1.36")
set(GLFW_DIR "${EXTERNAL_DIR}/glfw-3.3.8")
set(GLM_DIR "${EXTERNAL_DIR}/glm-0.9.9.8")
set(IMGUI_DIR "${EXTERNAL_DIR}/imgui-1.88")
set(STB_DIR "${EXTERNAL_DIR}/stb-2.27")

# OS Libraries
if(WIN32)
  message(FATAL_ERROR "Unsupported OS")
elseif(APPLE)
  find_library(OPENGL_LIB OpenGL REQUIRED)
  find_library(COCOA_LIB Cocoa REQUIRED)
  find_library(IOKIT_LIB IOKit REQUIRED)
  target_link_libraries(${EXECUTABLE} ${OPENGL_LIB} ${COCOA_LIB} ${IOKIT_LIB})
elseif(UNIX)
  message(FATAL_ERROR "Unsupported OS")
endif()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${GLFW_DIR} build.glfw)
target_link_libraries(${EXECUTABLE} glfw)
target_include_directories(${EXECUTABLE} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${EXECUTABLE} PRIVATE "GLFW_INCLUDE_NONE")

# GLAD
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${EXECUTABLE} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${EXECUTABLE} "glad" "${CMAKE_DL_LIBS}")

# STB
add_library("stb" "${STB_DIR}/src/stb.c")
target_include_directories("stb" PRIVATE "${STB_DIR}/include")
target_include_directories(${EXECUTABLE} PRIVATE "${STB_DIR}/include")
target_link_libraries(${EXECUTABLE} "stb")

# GLM
target_include_directories(${EXECUTABLE} PRIVATE "${GLM_DIR}")

# Freetype
if(ENABLE_FREETYPE)
  set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
  add_subdirectory(${FREETYPE_DIR} build.freetype)
  target_link_libraries(${EXECUTABLE} freetype)
  target_include_directories(${EXECUTABLE} PRIVATE "${FREETYPE_DIR}/include")
endif()

# ImGui
add_library("imgui" STATIC)
set_property(TARGET "imgui" PROPERTY CXX_STANDARD 17)
target_sources("imgui"
  PRIVATE
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories("imgui" PRIVATE "${GLFW_DIR}/include")
target_include_directories("imgui" PRIVATE "${GLAD_DIR}/include")
target_include_directories("imgui"
  PUBLIC ${IMGUI_DIR}
  PUBLIC ${IMGUI_DIR}/backends
)
target_link_libraries("imgui" PUBLIC ${OPENGL_LIB})
target_link_libraries(${EXECUTABLE} "imgui")

# Resources
add_custom_target(copy_assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
if(COPY_ASSETS)
  add_dependencies(${EXECUTABLE} copy_assets)
endif()
```

```cpp
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <string.h>

#include <array>
#include <vector>
#include <string>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "framework/GUI.hpp"
#include "framework/Shader.hpp"
#include "framework/Window.hpp"
#include "util/log.h"
#include "SmileBox.hpp"

int main() {
    GLint success = 0;
    Window window(&success, "A-4-LoopSubdivision", 800, 600);
    if (success != 1) {
        log_fatal("Failed to create GLFW window");
        return -1;
    }
    window.EnableStatisticGUI(Window::STAT_FPS | Window::STAT_MEMORY | Window::STAT_POSITION);

    Camera &camera = window.camera;
    {
        camera.SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        camera.SetMovementSpeed(15);
        camera.SetMaxRenderDistance(1e8f);
    }

    std::vector<SmileBox> smileBoxs(12);
    for (int i = 0; i < smileBoxs.size(); i++) {
        smileBoxs[i].SetShaderPath("assets/shaders/smilebox.vs", "assets/shaders/smilebox.fs");
        smileBoxs[i].SetTexturePath("assets/textures/container.jpg", "assets/textures/awesomeface.png");
        smileBoxs[i].Setup(i);
        std::vector<glm::vec3> cubePositions {
            glm::vec3(0.0f * 2, -1.0f, -40.0f),
            glm::vec3(1.0f * 2, -1.0f, -40.0f),
            glm::vec3(2.0f * 2, -1.0f, -40.0f),
            glm::vec3(3.0f * 2, -1.0f, -40.0f),
            glm::vec3(0.0f * 2,  0.0f, -40.0f),
            glm::vec3(1.0f * 2,  0.0f, -40.0f),
            glm::vec3(2.0f * 2,  0.0f, -40.0f),
            glm::vec3(3.0f * 2,  0.0f, -40.0f),
            glm::vec3(0.0f * 2, +1.0f, -40.0f),
            glm::vec3(1.0f * 2, +1.0f, -40.0f),
            glm::vec3(2.0f * 2, +1.0f, -40.0f),
            glm::vec3(3.0f * 2, +1.0f, -40.0f),
        };
        assert(smileBoxs.size() <= cubePositions.size());
        smileBoxs[i].MoveTo(cubePositions[i]);
        window.AddObject(&smileBoxs[i]);
    }

    double now;
    double lastUpdateTime = 0;
    double lastRenderTime = 0;
    double FPSlimits = 60; // std::numeric_limits<double>::infinity()
    while (window.ContinueLoop()) {
        now = glfwGetTime();
        ////// Update Logic
        window.Update(now, lastUpdateTime);
        lastUpdateTime = now;
        ////// Render Frame
        if (now - lastRenderTime >= 1 / FPSlimits) {
            window.Render(now, lastRenderTime);
            lastRenderTime = now;
        }
    }

    return 0;
}
```
