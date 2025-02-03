#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#include <unistd.h>
#endif

#define SERVER_IP "127.0.0.1"
#define PORT 9100

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error creant el socket" << std::endl;
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        std::cerr << "Adreça invàlida" << std::endl;
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "No es pot connectar al servidor" << std::endl;
        return 1;
    }

    std::cout << "Introduïu la vostra comanda: ";
    std::string command;
    std::getline(std::cin, command);

    send(sock, command.c_str(), command.length(), 0);

    char buffer[1024] = {0};
    int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        std::cout << "Resposta del servidor: " << buffer << std::endl;
    }

    #ifdef _WIN32
        closesocket(sock);
    #else
        close(sock);
    #endif
    return 0;
}
