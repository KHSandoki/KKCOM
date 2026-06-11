#pragma once

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cstddef>

class ResourceManager {
public:
    // Load embedded image data and create OpenGL texture
    static unsigned int loadTextureFromMemory(const unsigned char* data, size_t size, int* width = nullptr, int* height = nullptr);
    
    // Set window icon from embedded data
    static void setWindowIcon(GLFWwindow* window, const unsigned char* data, size_t size);
    
    // Show splash screen with launch image
    static void showSplashScreen(const unsigned char* launchData, size_t launchSize, 
                               const unsigned char* iconData, size_t iconSize);
    
private:
    // Create and show splash screen window
    static GLFWwindow* createSplashWindow(int width, int height);
    
    // Render texture to screen
    static void renderTexture(unsigned int texture, int windowWidth, int windowHeight);
    
    // Initialize OpenGL for splash screen
    static void initSplashGL();
};