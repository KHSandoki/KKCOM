#include "ResourceManager.h"
#include "DebugConfig.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Define image data implementations
#define ICON_DATA_IMPLEMENTATION
#include "icon_data.h"
#define LAUNCH_DATA_IMPLEMENTATION  
#include "launch_data.h"

#include <iostream>
#include <chrono>
#include <thread>

// Define OpenGL constants if not available
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

unsigned int ResourceManager::loadTextureFromMemory(const unsigned char* data, size_t size, int* width, int* height) {
    DEBUG_PRINT("Loading texture from memory, size: " << size << " bytes");
    
    int w, h, channels;
    unsigned char* imageData = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &channels, 4);
    
    if (!imageData) {
        DEBUG_ERROR("Failed to load image from memory: " << stbi_failure_reason());
        return 0;
    }
    
    DEBUG_PRINT("Image loaded successfully: " << w << "x" << h << ", channels: " << channels);
    
    if (width) *width = w;
    if (height) *height = h;
    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    DEBUG_PRINT("Generated texture ID: " << texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        DEBUG_ERROR("OpenGL error after texture creation: " << error);
    }
    
    stbi_image_free(imageData);
    return texture;
}

void ResourceManager::setWindowIcon(GLFWwindow* window, const unsigned char* data, size_t size) {
    DEBUG_PRINT("Setting window icon, data size: " << size);
    
    int width, height, channels;
    unsigned char* imageData = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 4);
    
    if (!imageData) {
        DEBUG_ERROR("Failed to load icon from memory: " << stbi_failure_reason());
        return;
    }
    
    GLFWimage icon;
    icon.width = width;
    icon.height = height;
    icon.pixels = imageData;
    
    glfwSetWindowIcon(window, 1, &icon);
    
    stbi_image_free(imageData);
}

GLFWwindow* ResourceManager::createSplashWindow(int width, int height) {
    // Use compatibility profile for better compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // No window decorations
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Not resizable
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);    // Always on top
    
    GLFWwindow* window = glfwCreateWindow(width, height, "KKCOM Loading...", nullptr, nullptr);
    if (!window) {
        DEBUG_ERROR("Failed to create splash window");
        return nullptr;
    }
    
    DEBUG_PRINT("Created splash window: " << width << "x" << height);
    
    // Center the window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);
    
    return window;
}

void ResourceManager::initSplashGL() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    // Clear any OpenGL errors
    while (glGetError() != GL_NO_ERROR);
    
    // Set up orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void ResourceManager::renderTexture(unsigned int texture, int windowWidth, int windowHeight) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (texture == 0) {
        DEBUG_ERROR("Warning: Invalid texture ID");
        return;
    }
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Check if texture binding was successful
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        DEBUG_ERROR("OpenGL error after texture binding: " << error);
    }
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);  // Bottom-left (flip Y)
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 0.0f);  // Bottom-right (flip Y)
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);  // Top-right (flip Y)
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 1.0f);  // Top-left (flip Y)
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ResourceManager::showSplashScreen(const unsigned char* launchData, size_t launchSize, 
                                     const unsigned char* iconData, size_t iconSize) {
    // Create splash window
    GLFWwindow* splashWindow = createSplashWindow(600, 300);
    if (!splashWindow) return;
    
    glfwMakeContextCurrent(splashWindow);
    
    // Set window icon
    setWindowIcon(splashWindow, iconData, iconSize);
    
    // Initialize OpenGL for splash
    initSplashGL();
    
    // Load launch image texture
    int imageWidth, imageHeight;
    unsigned int launchTexture = loadTextureFromMemory(launchData, launchSize, &imageWidth, &imageHeight);
    
    if (launchTexture == 0) {
        glfwDestroyWindow(splashWindow);
        return;
    }
    
    // Show splash screen for 2.5 seconds
    auto startTime = std::chrono::high_resolution_clock::now();
    const auto splashDuration = std::chrono::milliseconds(2500);
    
    while (!glfwWindowShouldClose(splashWindow)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        if (currentTime - startTime >= splashDuration) {
            break;
        }
        
        glfwPollEvents();
        
        int width, height;
        glfwGetFramebufferSize(splashWindow, &width, &height);
        glViewport(0, 0, width, height);
        
        renderTexture(launchTexture, width, height);
        
        glfwSwapBuffers(splashWindow);
        
        // Small delay to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Cleanup
    glDeleteTextures(1, &launchTexture);
    glfwDestroyWindow(splashWindow);
}