cmake_minimum_required(VERSION 3.16)
project(KKCOM_CPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find packages, fallback to manual setup if not found
find_package(OpenGL REQUIRED)

# Try to find GLFW (prefer static linking)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found via find_package, trying PkgConfig")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)
endif()

# Force static linking for Windows
if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(BUILD_SHARED_LIBS OFF)
    # Use static runtime library for standalone executable
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Try to find nlohmann_json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, you may need to install it manually")
    # For now, we'll include a simple fallback
endif()

# Add ImGui (we'll need to download it manually or use a simpler approach)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui")
if(EXISTS "${IMGUI_DIR}")
    file(GLOB IMGUI_SOURCES 
        "${IMGUI_DIR}/*.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    message(STATUS "Found ImGui sources")
else()
    message(WARNING "ImGui not found at ${IMGUI_DIR}. Please download Dear ImGui to libs/imgui/")
    set(IMGUI_SOURCES "")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(EXISTS "${IMGUI_DIR}")
    include_directories(
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
endif()

# Source files (exclude test files)
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/test_serial.cpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
    # Additional Windows libraries needed for static GLFW
    if(WIN32)
        target_link_libraries(${PROJECT_NAME} user32 gdi32 shell32)
    endif()
elseif(GLFW3_FOUND)
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${GLFW3_CFLAGS_OTHER})
endif()

if(nlohmann_json_FOUND)
    target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
else()
    # Simple fallback - assume header is available in system
    target_compile_definitions(${PROJECT_NAME} PRIVATE SIMPLE_JSON_FALLBACK)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm ws2_32 setupapi)
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

# Copy assets if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()