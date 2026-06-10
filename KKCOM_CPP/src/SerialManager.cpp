#include "SerialManager.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
#include <setupapi.h>
#include <devguid.h>
#pragma comment(lib, "setupapi.lib")
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

SerialManager::SerialManager() 
    : connected_(false), receiving_(false), timeout_(40)
#ifdef _WIN32
    , serialHandle_(INVALID_HANDLE_VALUE)
#else
    , serialFd_(-1)
#endif
{
}

SerialManager::~SerialManager() {
    disconnect();
}

std::vector<SerialManager::PortInfo> SerialManager::getAvailablePorts() {
    std::vector<PortInfo> ports;
    
#ifdef _WIN32
    // Windows implementation
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return ports;
    }
    
    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        HKEY key = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if (key != INVALID_HANDLE_VALUE) {
            char portName[256];
            DWORD size = sizeof(portName);
            if (RegQueryValueExA(key, "PortName", nullptr, nullptr, (LPBYTE)portName, &size) == ERROR_SUCCESS) {
                if (strncmp(portName, "COM", 3) == 0) {
                    char friendlyName[256];
                    if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)friendlyName, sizeof(friendlyName), nullptr)) {
                        std::string displayName = std::string(portName) + " - " + std::string(friendlyName);
                        ports.push_back({std::string(portName), displayName});
                    } else {
                        ports.push_back({std::string(portName), std::string(portName)});
                    }
                }
            }
            RegCloseKey(key);
        }
    }
    
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
#else
    // Linux implementation
    DIR* dir = opendir("/dev");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.find("ttyUSB") == 0 || name.find("ttyACM") == 0 || name.find("ttyS") == 0) {
                ports.push_back({"/dev/" + name, "/dev/" + name});
            }
        }
        closedir(dir);
    }
#endif
    
    if (ports.empty()) {
        ports.push_back({"No ports found", "No ports found"});
    }
    
    return ports;
}

bool SerialManager::connect(const std::string& portName, int baudRate) {
    if (connected_) {
        disconnect();
    }
    
#ifdef _WIN32
    std::string fullPortName = "\\\\.\\" + portName;
    serialHandle_ = CreateFileA(
        fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (serialHandle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DCB dcb = {};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(serialHandle_, &dcb)) {
        CloseHandle(serialHandle_);
        serialHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }
    
    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    
    if (!SetCommState(serialHandle_, &dcb)) {
        CloseHandle(serialHandle_);
        serialHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }
    
    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = timeout_;
    timeouts.ReadTotalTimeoutConstant = timeout_;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    
    if (!SetCommTimeouts(serialHandle_, &timeouts)) {
        CloseHandle(serialHandle_);
        serialHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }
    
#else
    serialFd_ = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serialFd_ < 0) {
        return false;
    }
    
    struct termios tty;
    if (tcgetattr(serialFd_, &tty) != 0) {
        close(serialFd_);
        serialFd_ = -1;
        return false;
    }
    
    // Configure port settings
    cfsetospeed(&tty, B115200); // Will need to map baudRate properly
    cfsetispeed(&tty, B115200);
    
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;
    
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = timeout_ / 100; // Convert ms to deciseconds
    
    if (tcsetattr(serialFd_, TCSANOW, &tty) != 0) {
        close(serialFd_);
        serialFd_ = -1;
        return false;
    }
#endif
    
    connected_ = true;
    receiving_ = true;
    connectionLost_ = false;
    receiveThread_ = std::thread(&SerialManager::receiveLoop, this);

    return true;
}

void SerialManager::disconnect() {
    if (!connected_) {
        return;
    }
    
    receiving_ = false;
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    
#ifdef _WIN32
    if (serialHandle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(serialHandle_);
        serialHandle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (serialFd_ >= 0) {
        close(serialFd_);
        serialFd_ = -1;
    }
#endif
    
    connected_ = false;
    connectionLost_ = false;
}

bool SerialManager::sendData(const std::string& data) {
    if (!connected_) {
        return false;
    }

    // The caller (SerialApp::sendCommand) is responsible for any line ending —
    // send the bytes exactly as given.

    // Multiple threads (UI, sendEvery, toggleSend) may call sendData()
    // concurrently — serialize writes so commands don't interleave on the wire.
    std::lock_guard<std::mutex> lock(writeMutex_);

#ifdef _WIN32
    DWORD bytesWritten;
    return WriteFile(serialHandle_, data.c_str(), static_cast<DWORD>(data.length()), &bytesWritten, nullptr) &&
           bytesWritten == data.length();
#else
    ssize_t bytesWritten = write(serialFd_, data.c_str(), data.length());
    return bytesWritten == static_cast<ssize_t>(data.length());
#endif
}

void SerialManager::setDataCallback(DataCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    dataCallback_ = callback;
}

void SerialManager::receiveLoop() {
    // Larger buffer reduces syscalls at high baud rates. Reads are paced by the
    // read timeout (idle) or by data availability (busy) — there is no fixed
    // per-iteration sleep, which previously capped throughput well below the
    // 921600 baud line rate and caused data to back up over time.
    char buffer[8192];

    while (receiving_) {
        bool gotData = false;

#ifdef _WIN32
        DWORD bytesRead = 0;
        if (!ReadFile(serialHandle_, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
            // Read failed — the device was most likely removed/unplugged.
            connectionLost_ = true;
            receiving_ = false;
            break;
        }
        if (bytesRead > 0) {
            gotData = true;
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (dataCallback_) {
                dataCallback_(std::string(buffer, bytesRead));
            }
        } else {
            // No data this cycle. Some drivers keep returning empty reads instead
            // of failing when the device is unplugged, so probe the line state —
            // GetCommModemStatus fails once the underlying device is gone.
            DWORD modemStatus = 0;
            if (!GetCommModemStatus(serialHandle_, &modemStatus)) {
                connectionLost_ = true;
                receiving_ = false;
                break;
            }
        }
#else
        ssize_t bytesRead = read(serialFd_, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            gotData = true;
            std::lock_guard<std::mutex> lock(callbackMutex_);
            if (dataCallback_) {
                dataCallback_(std::string(buffer, bytesRead));
            }
        } else if (bytesRead == 0) {
            // EOF on the fd — device gone.
            connectionLost_ = true;
            receiving_ = false;
            break;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            // A real error (EIO/ENXIO/EBADF...) — device removed.
            connectionLost_ = true;
            receiving_ = false;
            break;
        }
#endif

        // Only yield when idle so streaming throughput isn't throttled.
        if (!gotData) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}