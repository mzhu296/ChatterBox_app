#include "socket.h"
#include <iostream>
#include <thread>

using namespace Sync;

int main(void)
{
    std::cout << "I am a client. (type end to terminate)" << std::endl;

    // Create our socket
    Socket socket("127.0.0.1", 3000);
    socket.Open();

    // Start a thread to receive messages from the server
    std::thread receiveThread([&socket](){
        while (true) {
            ByteArray receivedData;
            socket.Read(receivedData);
            std::cout << "Received: " << receivedData.ToString() << std::endl;
        }
    });

    // Prepare data to send
    std::string response;
    while (true) {
        // std::cout << "Enter a message: ";
        std::getline(std::cin, response);

        if (response == "end") {
            std::cout << "Terminating connection." << std::endl;
            break;
        }

        ByteArray packet(response);

        // Send data
        socket.Write(packet);
    }

    // Join the receive thread
    receiveThread.join();

    // Close the connection
    socket.Close();

    return 0;
}
