#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

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

int main() {
    std::cout << "Client: Starting..." << std::endl;
    initializeSockets();

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Client: Failed to create socket" << std::endl;
        cleanupSockets();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
#ifdef _WIN32
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
#else
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

    std::cout << "Client: Connecting to server..." << std::endl;
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Client: Connection failed. Is the server running?" << std::endl;
        closesocket(clientSocket);
        cleanupSockets();
        return 1;
    }

    std::cout << "Client: Connected to server!" << std::endl;
    std::cout << "Type messages to send (type 'quit' to exit):" << std::endl;

    char buffer[BUFFER_SIZE];
    std::string message;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (message.empty()) continue;

        send(clientSocket, message.c_str(), static_cast<int>(message.length()), 0);

        if (message == "quit") {
            std::cout << "Client: Disconnecting..." << std::endl;
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived > 0) {
            std::cout << "Client: " << buffer << std::endl;
        } else {
            std::cout << "Client: Server disconnected" << std::endl;
            break;
        }
    }

    closesocket(clientSocket);
    cleanupSockets();

    return 0;
}