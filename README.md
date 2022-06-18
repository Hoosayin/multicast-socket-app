# multicast-socket-app

UDP Multicast Socket Using Windows Sockets v2.2 with Asynchronous Reception

DESCRIPTION: A console-based app that demonstrates to use Windows Sockets v2.2 to create a multicast group on a network with predefined TTL and Loop-Back value. Network Event “FD_READ” is registered to get “Read Notification” whenever some data is ready to be read from the socket. This is done by using “WSAEventSelect()” method that is called asynchronously, and is independent of any Window or Dialog Handles. Works pretty well with Console apps too. Network events are handled in the Reception Thread. Data is transmitted in the Main Thread. Since two threads simultaneously use the Socket Handle, Mutex Lock is used to avoid parallel use of the resource to achieve synchronization when transmitting or receiving bytes over the network.

IDE: Visual Studio 2022
PROGRAMMING LANGUAGE: c++
TOOLSET: v143 (Also works well on v142)
