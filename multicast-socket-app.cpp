#include "MulticastSocket.h"
#include <iostream>

int main()
{
    const char* MULTICAST_GROUP = "224.0.0.212";
    const int MULTICAST_PORT = 8201;
    const unsigned short LOOP_BACK = 1;
    const unsigned short TTL = 8;

    const unsigned short NUMBER_OF_MESSAGES = 10;
    const int DELAY_MILLISECONDS = 1000;

    MulticastSocket socket{};

    // Join Group.
    bool result = socket.Intialize(
        MULTICAST_GROUP,
        MULTICAST_PORT,
        LOOP_BACK,
        TTL,
        [](char* message, int messageSize) {
            std::cout << "Bytes Received: " << messageSize << std::endl;
            std::cout << "Message: " << message << std::endl << std::endl;
        });

    if (result == false)
    {
        std::cout << "Failed to initialize multicast socket." << std::endl;
        return 1;
    }

    char message[BUFFER_SIZE]{};
    int bytesSent{};
    int count{ 0 };

    // Sending
    while (count < NUMBER_OF_MESSAGES)
    {
        // Format Test Message.
        sprintf_s(message, "Hello World. Count: %d", ++count);

        // Send Message on Multicast Group.
        result = socket.Send(message, std::strlen(message), bytesSent);

        if (result == false)
            std::cout << "Failed to send message." << std::endl;

        // Sleep for 1 Second.
        Sleep(DELAY_MILLISECONDS);
    }

    return 0;
}