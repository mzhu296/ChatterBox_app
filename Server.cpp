#include "socketserver.h"
#include "socket.h"
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <algorithm>
#include <memory>
#include <map>

using namespace Sync;

class ChatRoom {
public:
    std::vector<std::shared_ptr<Socket>> clients;
    std::mutex clients_mutex;
};

std::map<std::string, ChatRoom> chatrooms;
std::mutex chatrooms_mutex;

void handle_client(std::shared_ptr<Socket> client_socket, const std::string& chatroom_name) {
    while (true) {
        try {
            ByteArray data;
            client_socket->Read(data);
            std::string message = data.ToString();

            if (message == "/quit") {
                std::cout << "Client has disconnected." << std::endl;
                break;
            } else {
                // Forward the message to all clients in the same chatroom
                std::lock_guard<std::mutex> lock(chatrooms_mutex);
                auto& chatroom = chatrooms[chatroom_name];
                std::lock_guard<std::mutex> client_lock(chatroom.clients_mutex);
                for (auto& client : chatroom.clients) {
                    if (client != client_socket) {
                        client->Write(data);
                    }
                }
            }
        } catch (std::exception& e) {
            std::cerr << "Exception in client handler: " << e.what() << std::endl;
            break;
        }
    }

    client_socket->Close();
    std::lock_guard<std::mutex> lock(chatrooms_mutex);
    auto& chatroom = chatrooms[chatroom_name];
    std::lock_guard<std::mutex> client_lock(chatroom.clients_mutex);
    chatroom.clients.erase(std::remove(chatroom.clients.begin(), chatroom.clients.end(), client_socket), chatroom.clients.end());
}

int main(int argc, char* argv[]) {
    SocketServer server(3000);

    try {
        while (true) {
            std::shared_ptr<Socket> client_socket(new Socket(server.Accept()));
            std::cout << "Client connected!" << std::endl;
            
            // Receive chatroom name from client
            ByteArray room_data;
            client_socket->Read(room_data);
            std::string chatroom_name = room_data.ToString();

            // Join the chatroom or create if it doesn't exist
            std::lock_guard<std::mutex> lock(chatrooms_mutex);
            chatrooms[chatroom_name].clients.push_back(client_socket);

            std::thread client_thread(handle_client, client_socket, chatroom_name);
            client_thread.detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
    }

    return 0;
}
