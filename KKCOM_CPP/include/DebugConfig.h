#pragma once

// Debug configuration
// Set to 1 to enable debug mode with console window
// Set to 0 for release mode without console window
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

// Debug output macros
#if DEBUG_MODE
#include <iostream>
#define DEBUG_PRINT(x) std::cout << x << std::endl
#define DEBUG_ERROR(x) std::cerr << x << std::endl
#define DEBUG_ENABLED true
#else
#define DEBUG_PRINT(x)
#define DEBUG_ERROR(x)
#define DEBUG_ENABLED false
#endif

// Console window management for Windows
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>

class DebugConsole {
public:
    static void allocateConsole() {
#if DEBUG_MODE
        if (AllocConsole()) {
            // Redirect stdout, stdin, stderr to console
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
            freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
            freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
            
            // Make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
            // point to console as well
            std::ios::sync_with_stdio(true);
            
            // Set console title
            SetConsoleTitleA("KKCOM Debug Console");
        }
#endif
    }
    
    static void freeConsole() {
#if DEBUG_MODE
        FreeConsole();
#endif
    }
};
#endif