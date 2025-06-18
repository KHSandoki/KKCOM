#include "SerialApp.h"
#include <iostream>

int main() {
    try {
        SerialApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}