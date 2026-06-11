#include "SerialApp.h"
#include "DebugConfig.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

#if DEBUG_MODE && defined(_WIN32)
// Debug mode: use standard main with console
int main() {
#elif defined(_WIN32)
// Release mode: use WinMain without console
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
// Non-Windows: always use main
int main() {
#endif

#ifdef _WIN32
    // Initialize debug console if needed
    DebugConsole::allocateConsole();
#endif

    try {
        DEBUG_PRINT("KKCOM starting in debug mode");
        
        // Show splash screen first
        SerialApp::showSplashScreen();
        
        // Then create and run the main application
        SerialApp app;
        app.run();
        
        DEBUG_PRINT("KKCOM shutting down normally");
    } catch (const std::exception& e) {
        DEBUG_ERROR("Application error: " << e.what());
        
        // Error handling based on debug mode
#if DEBUG_MODE
        std::cerr << "Application error: " << e.what() << std::endl;
#elif defined(_WIN32)
        MessageBoxA(NULL, e.what(), "KKCOM Error", MB_OK | MB_ICONERROR);
#else
        std::cerr << "Application error: " << e.what() << std::endl;
#endif

#ifdef _WIN32
        DebugConsole::freeConsole();
#endif
        return -1;
    }
    
#ifdef _WIN32
    DebugConsole::freeConsole();
#endif
    return 0;
}