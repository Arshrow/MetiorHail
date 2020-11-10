cmake_minimum_required(VERSION 3.16)
project(MetiorHail VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

file(GLOB imguiFiles
    "imgui/*.h"
    "imgui/*.cpp"
    "imgui/*.hpp"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})


find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(unofficial-brotli CONFIG REQUIRED)


add_compile_options(-lglfw3 -lGL -lx11 -lpthread -lXrandr -lXi -ldl -lGLU -lglut -lglad)


include_directories(src)
include_directories(imgui)
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${NANOGUI_EXTRA_INCS})


add_executable(${PROJECT_NAME}
		${imguiFiles}
		src/GUISlot.hpp
		src/GUISlot.cpp
        src/rand.cpp
        src/rand.hpp
        src/main.cpp)


target_link_libraries(MetiorHail PRIVATE glad::glad)
target_link_libraries(MetiorHail PRIVATE glfw)
target_link_libraries(MetiorHail PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(MetiorHail PRIVATE ZLIB::ZLIB)
target_include_directories(MetiorHail PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(MetiorHail PRIVATE freetype)
target_link_libraries(MetiorHail PRIVATE unofficial::brotli::brotlidec-static unofficial::brotli::brotlienc-static unofficial::brotli::brotlicommon-static)

