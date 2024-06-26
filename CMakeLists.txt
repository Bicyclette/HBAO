CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

project(hbao LANGUAGES C CXX)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
src/main.cpp
src/application.hpp
src/application.cpp
src/mesh.cpp
src/mesh.hpp
src/shader.hpp
src/shader.cpp
src/camera.hpp
src/camera.cpp
src/light.hpp
src/GLCommon.h
src/utils.h
src/material.hpp
src/material.cpp
src/framebuffer.hpp
src/framebuffer.cpp
src/stb_image.h
src/imgui/imgui.cpp
src/imgui/imgui_demo.cpp
src/imgui/imgui_draw.cpp
src/imgui/imgui_impl_glfw.cpp
src/imgui/imgui_impl_opengl3.cpp
src/imgui/imgui_tables.cpp
src/imgui/imgui_widgets.cpp
)

add_definitions(-DPROJECT_DIRECTORY="${PROJECT_SOURCE_DIR}")

target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/dep/glad/src/glad.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/dep/glad/include/)

add_subdirectory(dep/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

add_subdirectory(dep/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR})
