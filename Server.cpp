#include "thread.h"
#include "socketserver.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace Sync;

class ClientHandler : public Thread
{
private:
    Socket* clientSocket;
    std::vector<ClientHandler*>& clients;
    std::mutex& clientsMutex;

public:
    ClientHandler(Socket* socket, std::vector<ClientHandler*>& clients, std::mutex& mutex)
        : clientSocket(socket), clients(clients), clientsMutex(mutex)
    {}

    ~ClientHandler()
    {
        // Cleanup
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(std::remove(clients.begin(), clients.end(), this), clients.end());
        }
        std::cout << "Client disconnected." << std::endl;
        delete clientSocket;
    }

    virtual long ThreadMain()
    {
        while (true) {
            try {
                // Read data from the client
                ByteArray data;
                clientSocket->Read(data);
                std::string message = data.ToString();

                // Check for termination message
                if (message == "done") {
                    std::cout << "Client requested to terminate connection." << std::endl;
                    break;
                }

                // Forward the message to all clients
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (ClientHandler* client : clients) {
                        if (client != this) {
                            client->clientSocket->Write(data);
                        }
                    }
                }
            } catch (const std::exception& ex) {
                std::cerr << "Error in ClientHandler::ThreadMain(): " << ex.what() << std::endl;
                break;
            }
        }

        // Close the connection
        clientSocket->Close();

        return 0;
    }
};

int main(void)
{
    std::cout << "I am a server. (type end to terminate)" << std::endl;

    // Create our server
    SocketServer server(3000);
    std::vector<ClientHandler*> clients;
    std::mutex clientsMutex;

    while (true) {
        // Wait for a client socket connection
        Socket* clientSocket = new Socket(server.Accept());
        std::cout << "Client connected." << std::endl;

        // Create a new thread to handle the client
        ClientHandler* clientHandler = new ClientHandler(clientSocket, clients, clientsMutex);

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientHandler);
        }
    }

    // Clean up
    for (ClientHandler* client : clients) {
        delete client;
    }

    return 0;
}
