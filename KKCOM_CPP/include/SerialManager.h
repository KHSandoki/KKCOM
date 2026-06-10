#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class SerialManager {
public:
    struct PortInfo {
        std::string port;
        std::string deviceName;
    };

    using DataCallback = std::function<void(const std::string&)>;
    
    SerialManager();
    ~SerialManager();
    
    // Port management
    std::vector<PortInfo> getAvailablePorts();
    bool connect(const std::string& portName, int baudRate);
    void disconnect();
    bool isConnected() const { return connected_; }
    // True if the receive loop detected the device went away (e.g. USB unplug).
    // The UI polls this and calls disconnect() to tear the connection down.
    bool connectionLost() const { return connectionLost_.load(); }
    
    // Data operations
    bool sendData(const std::string& data);
    void setDataCallback(DataCallback callback);
    
    // Configuration
    void setTimeout(int milliseconds) { timeout_ = milliseconds; }
    
private:
    void receiveLoop();
    
#ifdef _WIN32
    HANDLE serialHandle_;
#else
    int serialFd_;
#endif
    
    std::atomic<bool> connected_;
    std::atomic<bool> receiving_;
    std::atomic<bool> connectionLost_{false};
    std::thread receiveThread_;
    DataCallback dataCallback_;
    int timeout_;
    std::mutex callbackMutex_;
    std::mutex writeMutex_;   // serializes sendData() across UI/sendEvery/toggle threads
};