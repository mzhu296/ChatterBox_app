#include "socketserver.h"
#include "socket.h"
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <algorithm>
#include <memory>

using namespace Sync;

std::vector<std::shared_ptr<Socket>> clients;
std::mutex clients_mutex;

void handle_client(std::shared_ptr<Socket> client_socket) {
    while (true) {
        try {
            ByteArray data;
            client_socket->Read(data);
            std::string message = data.ToString();

            if (message == "/quit") {
                std::cout << "Client has disconnected." << std::endl;
                break;
            }

            std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto& client : clients) {
                if (client != client_socket) {
                    client->Write(data);
                }
            }
        } catch (std::exception& e) {
            std::cerr << "Exception in client handler: " << e.what() << std::endl;
            break;
        }
    }

    client_socket->Close();
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
}

int main(int argc, char* argv[]) {
    SocketServer server(3000);

    try {
        while (true) {
            std::shared_ptr<Socket> client_socket(new Socket(server.Accept()));
            std::cout << "Client connected!" << std::endl;
            
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);

            std::thread client_thread(handle_client, client_socket);
            client_thread.detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
    }

    return 0;
}
