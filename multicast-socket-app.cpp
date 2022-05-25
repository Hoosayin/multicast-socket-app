#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#endif _WIN32

#include <iostream>
#include <thread>
#include <mutex>

constexpr auto BUFFER_SIZE = 1024;

int main()
{
    const char* MULTICAST_GROUP = "224.0.0.212";
    const int MULTICAST_PORT = atoi("8201");

    const int DELAY_SECONDS = 1;
    const char* MESSAGE = "Hello, World!";

#ifdef _WIN32
    WSADATA wsaData{};

    if (WSAStartup(0x0101, &wsaData)) 
    {
        std::cout << "WSAStartup() failed with error " << GetLastError() << std::endl;
        return 1;
    }
#endif _WIN32

    SOCKET socketHandle = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketHandle < 0) 
    {
        std::cout << "socket() failed with error " << GetLastError() << std::endl;
        return 1;
    }

    unsigned int allowReuse{ 1 };

    int result = setsockopt(
        socketHandle,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char*)&allowReuse,
        sizeof(allowReuse));

    if (result == SOCKET_ERROR) 
    {
        std::cout << "setsockopt() failed for socket reuse with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    struct sockaddr_in address{};

    ZeroMemory(&address, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(MULTICAST_PORT);

    result = bind(socketHandle, (struct sockaddr*)&address, sizeof(address));

    if (result == SOCKET_ERROR) {
        std::cout << "bind() failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    unsigned long ipV4address{};
    inet_pton(AF_INET, MULTICAST_GROUP, &ipV4address);

    struct ip_mreq multicastGroupConfiguration{};
    multicastGroupConfiguration.imr_multiaddr.s_addr = ipV4address;
    multicastGroupConfiguration.imr_interface.s_addr = htonl(INADDR_ANY);

    result = setsockopt(
        socketHandle, 
        IPPROTO_IP, 
        IP_ADD_MEMBERSHIP, 
        (char*)&multicastGroupConfiguration, 
        sizeof(multicastGroupConfiguration));

    if (result == SOCKET_ERROR) {
        std::cout << "setcsockopt() failed for joining group with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    int loopValue = 1;

    result = setsockopt(
        socketHandle,
        IPPROTO_IP,
        IP_MULTICAST_LOOP,
        (char*)&loopValue,
        sizeof(loopValue));

    if (result == SOCKET_ERROR) {
        std::cout << "setcsockopt() failed for loop-back with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    int ttl = 11;

    result = setsockopt(
        socketHandle,
        IPPROTO_IP,
        IP_MULTICAST_TTL,
        (char*)&ttl,
        sizeof(ttl));

    if (result == SOCKET_ERROR) {
        std::cout << "setcsockopt() failed for TTL with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    std::mutex mutex{};

    std::thread receptionThread([&socketHandle, &address, &mutex](int x) {
        WSAEVENT networkEvent = WSACreateEvent();
        int result = WSAEventSelect(socketHandle, networkEvent, FD_READ);

        if (result == SOCKET_ERROR)
        {
            std::cout << "WSAEventSelect() failed with error " << WSAGetLastError() << std::endl;
            return;
        }

        while (true)
        {
            WSANETWORKEVENTS networkEvents{};
            WSAEnumNetworkEvents(socketHandle, networkEvent, &networkEvents);

            if (networkEvents.lNetworkEvents & FD_READ)
            {
                if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
                {
                    std::cout << "FD_READ failed with error " << networkEvents.iErrorCode[FD_READ_BIT] << std::endl;
                    break;
                }
                else
                {
                    mutex.lock();

                    char messageBuffer[BUFFER_SIZE];
                    int addressLength = sizeof(address);

                    int bytesRead = recvfrom(
                        socketHandle,
                        messageBuffer,
                        BUFFER_SIZE,
                        0,
                        (struct sockaddr*)&address,
                        &addressLength);

                    if (bytesRead < 0) {
                        std::cout << "recvfrom() failed with error " << GetLastError() << std::endl;
                        mutex.unlock();
                        return;
                    }

                    messageBuffer[bytesRead] = '\0';

                    std::cout << "Read " << bytesRead << " bytes." << std::endl << "Message: " << messageBuffer << std::endl;

                    mutex.unlock();
                }
            }
        }
        }, 1);


    struct sockaddr_in sendingAddress {};

    ZeroMemory(&sendingAddress, sizeof(sendingAddress));

    unsigned long multicastGroupIPv4Address{};
    inet_pton(AF_INET, MULTICAST_GROUP, &multicastGroupIPv4Address);

    sendingAddress.sin_family = AF_INET;
    sendingAddress.sin_addr.s_addr = multicastGroupIPv4Address;
    sendingAddress.sin_port = htons(MULTICAST_PORT);

    ///////////////////////////////////////////////
    std::cout << "Socket setup successfully." << std::endl;

    int count{ 0 };

    // Sending
    while (true)
    {
        mutex.lock();

        char buffer[BUFFER_SIZE]{};
        
        sprintf_s(
            buffer,
            BUFFER_SIZE,
            "%s %d",
            MESSAGE, count++
        );

        int bytesSent = sendto(
            socketHandle,
            buffer,
            strlen(buffer),
            0,
            (struct sockaddr*)&sendingAddress,
            (int)sizeof(sendingAddress));

        if (bytesSent < 0) {
            std::cout << "sendto() failed with error " << WSAGetLastError() << std::endl;
            mutex.unlock();
            return -1;
        }

        std::cout << "Sent " << bytesSent << " bytes." << std::endl << "Message: " << buffer << std::endl;

        mutex.unlock();

#ifdef _WIN32
        Sleep(DELAY_SECONDS * 1000);
#else
        sleep(delay_secs); // Unix sleep is seconds
#endif
    }


    /*while (true) 
    {
        char messageBuffer[BUFFER_SIZE];
        int addressLength = sizeof(address);

        int bytesRead = recvfrom(
            socketHandle,
            messageBuffer,
            BUFFER_SIZE,
            0,
            (struct sockaddr*)&address,
            &addressLength);

        if (bytesRead < 0) {
            std::cout << "recvfrom() failed with error " << GetLastError() << std::endl;
            return 1;
        }

        std::cout << "Read " << bytesRead << " bytes." << std::endl;
    }*/

    receptionThread.join();

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}