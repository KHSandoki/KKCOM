#include "SerialManager.h"
#include <iostream>
#include <thread>
#include <chrono>

void onDataReceived(const std::string& data) {
    std::cout << "Received: " << data;
}

int main() {
    std::cout << "KKCOM Serial Test - Testing Serial Communication" << std::endl;
    
    SerialManager serialManager;
    
    // Set data callback
    serialManager.setDataCallback(onDataReceived);
    
    // Get available ports
    auto ports = serialManager.getAvailablePorts();
    std::cout << "Available ports:" << std::endl;
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << i << ": " << ports[i] << std::endl;
    }
    
    if (ports.empty() || ports[0] == "No ports found") {
        std::cout << "No serial ports available for testing." << std::endl;
        return 0;
    }
    
    // Try to connect to first available port
    std::cout << "Attempting to connect to " << ports[0] << " at 115200 baud..." << std::endl;
    
    if (serialManager.connect(ports[0], 115200)) {
        std::cout << "Connected successfully!" << std::endl;
        
        // Send a test command
        std::cout << "Sending test command..." << std::endl;
        serialManager.sendData("AT");
        
        // Wait for response
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Send another command
        serialManager.sendData("AT+VERSION");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::cout << "Test completed. Disconnecting..." << std::endl;
        serialManager.disconnect();
    } else {
        std::cout << "Failed to connect to " << ports[0] << std::endl;
    }
    
    return 0;
}