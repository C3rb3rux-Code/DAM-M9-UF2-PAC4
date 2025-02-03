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

#define PORT 9100

std::mutex file_mutex;
int order_counter = 1;

std::string generateOrderID() {
    std::ostringstream oss;
    oss << "ORD-" << std::setw(4) << std::setfill('0') << order_counter++;
    return oss.str();
}

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    #ifdef _WIN32
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    #else
        int bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
    #endif
    
    if (bytes_received <= 0) {
        #ifdef _WIN32
            closesocket(client_socket);
        #else
            #ifdef _WIN32
                closesocket(client_socket);
            #else
                #ifdef _WIN32
                    closesocket(client_socket);
                #else
                    close(client_socket);
                #endif
            #endif
        #endif
        return;
    }

    std::string command(buffer);
    std::string order_id;

    // Bloqueig per evitar corrupció del fitxer
    {
        std::lock_guard<std::mutex> lock(file_mutex);
        order_id = generateOrderID();
        
        std::ofstream file("comandes.txt", std::ios::app);
        if (file) {
            file << order_id << ": \"" << command << "\"\n";
            file.close();
        }
    }

    std::string response = "Identificador: " + order_id;
    send(client_socket, response.c_str(), response.length(), 0);
    closesocket(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error en el binding" << std::endl;
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Error en el listen" << std::endl;
        return 1;
    }

    std::cout << "Servidor escoltant al port " << PORT << "..." << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addr_len);
        if (new_socket >= 0) {
            std::cout << "Connexió acceptada" << std::endl;
            std::thread(handleClient, new_socket).detach();
        }
    }

    return 0;
}
