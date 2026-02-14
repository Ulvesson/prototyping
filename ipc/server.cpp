#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <sstream>
#include <csignal>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

std::mutex coutMutex;
std::atomic<int> clientCount{ 0 };
std::atomic<bool> serverRunning{true};

void initializeSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        exit(1);
    }
#endif
}

void cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

template<typename... Args>
void logThreadSafe(Args&&... args) {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::ostringstream oss;
    (oss << ... << args);
    std::cout << oss.str() << std::endl;
}

void handleClient(SOCKET clientSocket, int clientId) {
    logThreadSafe("Server: Client #", clientId, " connected! (Total clients: ", clientCount, ")");

    // Set socket timeout to detect dead connections
#ifdef _WIN32
    DWORD timeout = 30000; // 30 seconds
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif

    char buffer[BUFFER_SIZE];
    bool connectionAlive = true;

    while (connectionAlive) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived == 0) {
            // Client closed connection gracefully
            logThreadSafe("Server: Client #", clientId, " closed connection gracefully");
            break;
        } 
        else if (bytesReceived < 0) {
            // Error occurred
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error == WSAETIMEDOUT) {
                logThreadSafe("Server: Client #", clientId, " connection timed out");
            } else if (error == WSAECONNRESET) {
                logThreadSafe("Server: Client #", clientId, " connection reset by peer");
            } else {
                logThreadSafe("Server: Client #", clientId, " receive error: ", error);
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                logThreadSafe("Server: Client #", clientId, " connection timed out");
            } else if (errno == ECONNRESET) {
                logThreadSafe("Server: Client #", clientId, " connection reset by peer");
            } else {
                logThreadSafe("Server: Client #", clientId, " receive error: ", strerror(errno));
            }
#endif
            break;
        }

        std::string message(buffer, bytesReceived);
        
        // Validate message isn't empty or contains only whitespace
        if (message.empty() || message.find_first_not_of(" \t\n\r") == std::string::npos) {
            logThreadSafe("Server: Client #", clientId, " sent empty or invalid message");
            continue;
        }

        logThreadSafe("Server: Client #", clientId, " sent: ", message);

        if (message == "quit") {
            logThreadSafe("Server: Client #", clientId, " requested disconnect");
            break;
        }

        // Send response with error checking
        std::string response = "Echo from server to client #" + std::to_string(clientId) + ": " + message;
        int bytesSent = send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
        
        if (bytesSent == SOCKET_ERROR) {
#ifdef _WIN32
            int error = WSAGetLastError();
            logThreadSafe("Server: Client #", clientId, " send error: ", error);
#else
            logThreadSafe("Server: Client #", clientId, " send error: ", strerror(errno));
#endif
            connectionAlive = false;
        } else if (bytesSent < static_cast<int>(response.length())) {
            logThreadSafe("Server: Client #", clientId, " partial send (", bytesSent, "/", response.length(), " bytes)");
        }
    }

    closesocket(clientSocket);
    --clientCount;

    logThreadSafe("Server: Client #", clientId, " handler terminated (Remaining clients: ", clientCount, ")");
}

void handleClientSafe(SOCKET clientSocket, int clientId) {
    try {
        handleClient(clientSocket, clientId);
    } catch (const std::exception& ex) {
        logThreadSafe("Server: Client #", clientId, " exception: ", ex.what());
        closesocket(clientSocket);
        --clientCount;
    } catch (...) {
        logThreadSafe("Server: Client #", clientId, " unknown exception occurred");
        closesocket(clientSocket);
        --clientCount;
    }
}

void signalHandler(int signal) {
    logThreadSafe("Server: Received signal ", signal, ", shutting down...");
    serverRunning = false;
}

int main() {
    std::cout << "Server: Starting multi-client server..." << std::endl;
    initializeSockets();

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
#ifdef _WIN32
        std::cerr << "Server: Failed to create socket. Error: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "Server: Failed to create socket. Error: " << strerror(errno) << std::endl;
#endif
        cleanupSockets();
        return 1;
    }

    // Allow socket reuse
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Server: Failed to set SO_REUSEADDR. Error: " << WSAGetLastError() << std::endl;
    }
#else
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Server: Failed to set SO_REUSEADDR. Error: " << strerror(errno) << std::endl;
    }
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
#ifdef _WIN32
        std::cerr << "Server: Bind failed. Error: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "Server: Bind failed. Error: " << strerror(errno) << std::endl;
#endif
        closesocket(serverSocket);
        cleanupSockets();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
#ifdef _WIN32
        std::cerr << "Server: Listen failed. Error: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "Server: Listen failed. Error: " << strerror(errno) << std::endl;
#endif
        closesocket(serverSocket);
        cleanupSockets();
        return 1;
    }

    std::cout << "Server: Listening on port " << PORT << " for multiple clients..." << std::endl;
    std::cout << "Server: Press Ctrl+C to stop" << std::endl;

    std::vector<std::thread> clientThreads;
    int nextClientId = 1;
    constexpr int MAX_CLIENTS = 100; // Limit concurrent connections

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    while (serverRunning) {
        // Check if we've reached max clients
        if (clientCount >= MAX_CLIENTS) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET) {
#ifdef _WIN32
            int error = WSAGetLastError();
            std::cerr << "Server: Accept failed. Error: " << error << std::endl;
            if (error == WSAEINTR) {
                // Interrupted, likely shutting down
                break;
            }
#else
            std::cerr << "Server: Accept failed. Error: " << strerror(errno) << std::endl;
            if (errno == EINTR) {
                // Interrupted, likely shutting down
                break;
            }
#endif
            continue;
        }

        ++clientCount;
        int clientId = nextClientId++;

        try {
            // Create a new thread to handle this client with exception wrapper
            clientThreads.emplace_back(handleClientSafe, clientSocket, clientId);
            clientThreads.back().detach();
        } catch (const std::system_error& e) {
            logThreadSafe("Server: Failed to create thread for client #", clientId, ": ", e.what());
            closesocket(clientSocket);
            --clientCount;
        }
    }

    // Cleanup
    closesocket(serverSocket);
    cleanupSockets();

    return 0;
}
